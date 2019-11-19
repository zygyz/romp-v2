#include "Core.h"

#include <glog/logging.h>
#include <glog/raw_logging.h>

namespace romp {
/*
 * Note: comments for functions contain some notation about task and its 
 * label. We describe the notation in this comment here to clarify and to ease
 * the description in each comment.
 * T(L, index): the task represented by the task label, L', which contains 
 * prefix of label segments in label `L` up to index: L' = L[0:index]
 *
 * T(L): the task represented by the task label `L`
 *
 * Descendant tasks: all tasks spawned in the current task's execution context
 *
 */

/*
 * This function is a driver function that analyzes race condition between two
 * memory accesses.  Return true if there is race condition between the two 
 * accesses. Return false if no race condition is found.
 */
bool analyzeRaceCondition(const Record& histRecord, const Record& curRecord) {
  auto histLabel = histRecord.getLabel(); 
  auto curLabel = curRecord.getLabel(); 
  auto histTaskPtr = histRecord.getTaskPtr();
  auto curTaskPtr = curRecord.getTaskPtr();
  int diffIndex;
  auto histBeforeCur = happensBefore(histLabel, curLabel, diffIndex, 
          histTaskPtr, curTaskPtr);
  return histBeforeCur;
}

/*
 * This function analyzes the happens-before relationship between two memory
 * accesses based on their associated task labels. The idea is that task label
 * encodes nodes relationship in openmp task graph. If there exists a directed
 * path from node A to node B, node A `happens-before` node B. Otherwise, node
 * A is logically concurrent with node B. Note that instead of doing explicit 
 * graph traversal, we traverse and compare label segments. 
 * `hsitLabel`: task label denoting a task recorded in the access history
 * `curLabel`: task label denoting the current task
 * `diffIndex`: the index of first pair of different segment, which is the 
 * result of `compareLabel` function.
 * Return true if hist task happens before current task 
 * Return false if hist task is logically concurrent with current task
 * Issue fatal warning if current task happens before hist task.
 */
bool happensBefore(Label* histLabel, Label* curLabel, int& diffIndex, 
        void* histTaskPtr, void* curTaskPtr) {
  diffIndex = compareLabels(histLabel, curLabel);
  if (diffIndex < 0) {
    switch(diffIndex) {
      case static_cast<int>(eSameLabel):
        return false;
      case static_cast<int>(eLeftIsPrefix):
        return true;
      case static_cast<int>(eRightIsPrefix):
        RAW_LOG(FATAL, "right label: %s -> left label: %s", 
                histLabel->toString().c_str(), curLabel->toString().c_str());
        return false;
      default:
        RAW_LOG(FATAL, "unknown label compare result");
        return false;
    }
  }
  // `diffIndex` points to the first pair of different label segment
  auto histSegment = histLabel->getKthSegment(diffIndex); 
  auto curSegment = curLabel->getKthSegment(diffIndex);
  uint64_t histOffset, curOffset, histSpan, curSpan; 
  histSegment->getOffsetSpan(histOffset, histSpan);
  curSegment->getOffsetSpan(curOffset, curSpan);
  if (histSpan != curSpan) {
    RAW_LOG(FATAL, "left span: %lu != right span: %lu", histSpan, curSpan);
  }
  if (histSpan == 1) { // explicit task or work share task or initial task

  } else { // left span == right span and span > 1, implicit task
    if (histOffset != curOffset) { 
      auto span = histSpan;
      if (histOffset % span == curOffset % span) {
        RAW_CHECK(histOffset < curOffset, "not expecting history access joined \
                before current access");
        /* 
         * Any possible descendent tasks of T(histLabel, diffIndex) 
         * should have joined with T(histLabel, diffIndex). And they 
         * should happen before any descendent tasks of T(curLabel, diffIndex)
         */
        return true; 
      } else {
        return analyzeSiblingImpTask(histLabel, curLabel, diffIndex, 
                histTaskPtr, curTaskPtr);
      }
    } else { 
      return analyzeSameImpTask(histLabel, curLabel, diffIndex, histTaskPtr,
              curTaskPtr); 
    }
  }
}

/*
 * This function analyzes happens-before relation when first pair of different 
 * segments are implicit segment, where offset are different and offset%span 
 * are different. This means the T(histLabel, diffIndex) and 
 * T(curLabel, diffIndex) are two sibling implicit tasks in the same parallel 
 * region. T(histLabel) and T(curLabel) are descendent tasks of 
 * T(histLabel, diffIndex), T(curLabel, diffIndex) respectively.
 *
 * Return true if T(histLabel) -> T(curLabel)
 * Return false if T(histLabel) || T(curLabel)
 * Issue fatal warning if T(curLabel) -> T(histLabel)
 *
 */
bool analyzeSiblingImpTask(Label* histLabel, Label* curLabel, int diffIndex,
        void* histTaskPtr, void* curTaskPtr) { 
  auto lenHistLabel = histLabel->getLabelLength();
  auto lenCurLabel = curLabel->getLabelLength();
  if (diffIndex == (lenHistLabel - 1) || diffIndex == (lenCurLabel - 1)) {
    // if any one if T(histLabel) and T(curLabel) is leaf implicit task, 
    // we are sure there is happens-beofre relationship
    return false;
  }
  // now diffIndex + 1 must not be out of boundary
  auto histNextSeg = histLabel->getKthSegment(diffIndex + 1);  
  auto curNextSeg = curLabel->getKthSegment(diffIndex + 1);
  auto histNextSegType = histNextSeg->getType();
  auto curNextSegType = curNextSeg->getType();
  /* Task dependency applies to sibling tasks, which are child tasks in a 
   * task region. We already know that T(histLabel) and T(curLabel) can
   * not be sibling tasks.
   */
  if (histNextSegType == eWorkShare && curNextSegType == eWorkShare) {
    // in this case, it is possible to be ordered with ordered section
    if (static_cast<WorkShareSegment*>(histNextSeg)->isSection() || 
            static_cast<WorkShareSegment*>(curNextSeg)->isSection()) {
      // section construct does not have ordered section 
      return false;
    } 
    auto histSeg = histLabel->getKthSegment(diffIndex);
    auto curSeg = curLabel->getKthSegment(diffIndex);
    auto histSegLoopCount = histSeg->getLoopCount();
    auto curSegLoopCount = curSeg->getLoopCount();
    if (histSegLoopCount == curSegLoopCount) {
      return analyzeOrderedSection(histLabel, curLabel, diffIndex + 1, 
              histTaskPtr, curTaskPtr);
    } 
    return false;
  }
  return false;
}

/*
 * This function analyzes if Task(histLabel) is ordered by ordered section 
 * with Task(curLabel). T(histLabel, startIndex) and T(curLabel, startIndex) 
 * are workshare task.
 */
bool analyzeOrderedSection(Label* histLabel, Label* curLabel, int startIndex, 
        void* histTaskPtr, void* curTaskPtr) {
  auto histBaseSeg  = histLabel->getKthSegment(startIndex);
  auto curBaseSeg = curLabel->getKthSegment(startIndex);
  auto histSegment = static_cast<WorkShareSegment*>(histBaseSeg);
  auto curSegment = static_cast<WorkShareSegment*>(curBaseSeg);
  if (histSegment->isPlaceHolder() || curSegment->isPlaceHolder()) {
    // have not entered the workshare construct yet.
    return false;
  } 
  auto histWorkShareId = histSegment->getWorkShareId();
  auto curWorkShareId = curSegment->getWorkShareId(); 
  RAW_CHECK(histWorkShareId < curWorkShareId, "not expecting hist iter id >= \
          cur iter id");
  auto histPhase = histBaseSeg->getPhase();
  auto curPhase = curBaseSeg->getPhase();
  auto histExitRank = computeExitRank(histPhase);
  auto curEnterRank = computeEnterRank(curPhase);
  if (histExitRank < curEnterRank) {
    auto histLen = histLabel->getLabelLength();
    auto curLen = curLabel->getLabelLength();
    if (startIndex == histLen - 1) {
      /* T(histLabel, startIndex) is leaf task, while T(curLabel) is descendent
       * task of T(curLabel. startIndex), ordered section has imposed happens
       * before relation in this case
       */
      return true;
    } else {
      /*
       * T(histLabel) is the descendent task of T(histLabel, startIndex)
       */
      return analyzeOrderedDescendents(histLabel, startIndex, histTaskPtr, 
              curTaskPtr);
    } 
  }
  return false;
}

/*
 * This function analyzes if T(histLabel) sync with T(histLabel, startIndex) 
 * and thus is ordered by ordered section. 
 *
 * Return true if T(histLabel) sync with T(histLabel, startIndex) within the 
 * effect of ordered section
 * Return false if T(histLabel) does not sync with ordered section
 */
bool analyzeOrderedDescendents(Label* histLabel, int startIndex, 
        void* histTaskPtr, void* curTaskPtr) {
  auto nextSeg = histLabel->getKthSegment(startIndex + 1);
  auto nextSegType = nextSeg->getType();
  if (nextSegType == eImplicit) {
    // we know that implicit task syncs with its parent task
    return true;
  } else if (nextSegType == eWorkShare) {
    RAW_LOG(FATAL, "does not expect next segment of workshare seg to be \
           workshare segment"); 
  } else if (nextSegType == eExplicit) {
    /*
     * Explicit task does not interact with ordered section. i.e., if an
     * explicit task is created inside an ordered section, the finish 
     * of ordered section does not sync the explicit task. Here the idea 
     * is that since we have already moved to next iteration of ordered 
     * section, T(histLabel, startIndex+1) is an explicit task, and 
     * T(histLabel) is its descendent task. If T(histLabel) does sync by
     * the ordered section, there must be some explicit tasking sync 
     * applied (e.g., taskwait, taskgroup).
     */ 
    auto curSeg = histLabel->getKthSegment(startIndex);
    /*
     * first determine if T(histLabel, startIndex + 1) is outside of the scope
     * of ordered section or not. This is done by checking the phase value of 
     * label segment histLabel[startIndex]. If the phase value is even, it is 
     * out of the ordered section scope.
     */
    auto phase = curSeg->getPhase();  
    if (phase % 2 == 0) {
      /* 
       * T(histLabel, startIndex + 1) is out of ordered section scope
       * In this case, taskgroup construct's scope could be wrapping the 
       * ordered section's lexical scope. Thus taskgroup construct does
       * not necessarily guarantees T(histLabel) is in sync.
       */
      if (!nextSeg->isTaskwaited()) {
   
      } else {
        
      }
    } else {
      /* T(histLabel, startIndex + 1) is in ordered section scope
       * In this case, taskgroup construct would be confined in the ordered 
       * section's lexical scope. So if T(histLabel, startIndex) has the 
       * taskgroup construct, T(histLabel) should be in sync.
       */
      auto taskGroupLevel = curSeg->getTaskGroupLevel();  
      if (taskGroupLevel > 0) {
        return true;
      } else {
       // further check taskwait sync
        if (!nextSeg->isTaskwaited()) {
          return false; 
        } else {
          // explicit task T(histLabel, startIndex+1) is sync with taskwait clause, 
          // check further down the task creation chain
          return analyzeSyncChain(histLabel, startIndex + 1);     
        }
      }
    }
  }
}

/* 
 * This function analyzes if T(label) is in sync with T(label, startIndex).
 * i.e., T(label, startIndex)'s completion guarantees the completion of 
 * T(label)
 *
 * Return ture if T(label) is in sync with T(label, startIndex)
 * Return false otherwise.
 */
bool analyzeSyncChain(Label* label, int startIndex) {
  auto lenLabel = label->getLabelLength(); 
  if (startIndex == lenLabel - 1) {
    // already the leaf task
    return true;
  }
  for (auto i = startIndex; i < lenLabel; ++i) {
    auto seg = label->getKthSegment(i);
    auto segType = seg->getType();
    if (segType == eImplicit) {
      return true;
    } else if (segType == eExplicit) {
      auto taskGroupLevel = seg->getTaskGroupLevel();    
      if (taskGroupLevel > 0) {
        // taskgroup guarantees completion of descendents
        return true;
      } else {
        if (!seg->isTaskwaited()) { 
          // if current explicit task T(label, i) is not waited 
          // by parent task, no sync.
          return false;
        }
      }
    }
  }
  return true;
}


/*
 * This function analyzes happens-before relation when first pair of different 
 * segments are implicit segment, where offset are the same. This means that 
 * T(histLabel, diffIndex) and T(curLabel, diffIndex) are the same implicit 
 * task, denote it as T'. T(histLabel), T(curLabel) are descendent tasks of 
 * T'.
 *
 * Return true if T(histLabel) -> T(curLabel)
 * Return false if T(histLabel) || T(curLabel)
 * Issue fatal warning if T(curLabel) -> T(histLabel)
 */
bool analyzeSameImpTask(Label* histLabel, Label* curLabel, int diffIndex, 
        void* histTaskPtr, void* curTaskPtr) {
  auto lenHistLabel = histLabel->getLabelLength(); 
  auto lenCurLabel = curLabel->getLabelLength();
  if (lenHistLabel == lenCurLabel) {
    if (diffIndex == (lenHistLabel - 1)) {
      // T(histLabel) and T(curLabel) are the same implicit task (they are
      // leaf task)  but at different stages. In this case, it is only
      // possible to have T(histLabel) happens before T(curLabel)
      return true;
    } else { 
     /*
      * T(histLabel) and T(curLabel) are descendent tasks of
      * T(histLabel, diffIndex) and T(curLabel, diffIndex) respectively
      * We assert that T(histLabel, diffIndex + 1) and 
      * T(curLabel, diffIndex + 1) are not sibling implicit task. Because
      * otherwise histLabel[diffIndex] and curLabel[diffIndex] should be the 
      * same.
      */
      auto histNextSeg = histLabel->getKthSegment(diffIndex + 1);
      auto curNextSeg = curLabel->getKthSegment(diffIndex + 1);
      auto histNextType = histNextSeg->getType();
      auto curNextType = curNextSeg->getType();
      RAW_CHECK(!(histNextType == eImplicit && curNextType == eImplicit),
              "not expecting next level tasks are sibling implicit tasks");
      // invoke different checking depending on next segment's type 
      auto checkCase = buildCheckCase(histNextType, curNextType);
      return dispatchAnalysis(checkCase, histLabel, curLabel, diffIndex);
    }
  } else { 
     
  }
}

/* 
 * This function analyzes case when T(histLabel, diffIndex) and 
 * T(curLabel, diffIndex) are the same implicit task, T'. T(histLabel) and 
 * T(curLabel) are descendent tasks of T'. T(histLabel, diffIndex+1) is 
 * implicit task, T(curLabel, diffIndex + 1) is explicit task. 
 * In this case, we assume that task create count at histLabel[diffIndex] 
 * is smaller than task create count at curLabel[diffIndex]. And 
 * T(histLabel) must be in parallel with T(curLabel). Because if implicit
 * task T(histLabel, diffIndex + 1) is created first, the explicit task
 * T(curLabel, diffIndex + 1) can only be created after the end of parallel
 * region with T(histLabel, diffIndex+1). Then the offset field in
 * histLabel[diffIndex] and curLabel[diffIndex] should have been different.
 */
bool analyzeNextImpExp(Label* histLabel, Label* curLabel, int diffIndex) {
#ifdef DEBUG_CORE
  // checking like this is to make sure label segments meet our expectation
  // it could be eliminated in production 
  auto histSeg = histLabel->getKthSegment(diffIndex);
  auto curSeg = curLabel->getKthSegment(diffIndex);
  auto histTaskcreate = histSeg->getTaskcreate();
  auto curTaskcreate = curSeg->getTaskcreate();
  RAW_CHECK(histTaskcreate > curTaskcreate, "unexpecting hist task create \
         count <= cur task create count");
#endif
  return false;
}

/*
 * This function analyzes case when T(histLabel, diffIndex) and 
 * T(curLabel, diffIndex) are the same implicit task, T'. T(histLabel) and
 * T(curLabel) are descendent tasks of T'. T(histLabel, diffIndex+1) is
 * implicit task, T(curLabel, diffIndex+1) is workshare task. The workshare
 * task must be created first (If the implicit task is created first, it has
 * to be joined before creating the workshare task, then the offset field in 
 * histLabel[diffIndex] and curLabel[diffIndex] would be different) and does 
 * not encounter the implicit barrier because of the nowait clause (If there 
 * is no nowait clause, the implicit barrier would have made the offset field 
 * in histLabel[diffIndex], curLabel[diffIndex] different). 
 */
bool analzyeNextImpWork(Label* histLabel, Label* curLabel, int diffIndex) {
#ifdef DEBUG_CORE
  auto histSeg = histLabel->getKthSegment(diffIndex);
  auto curSeg = curLabel->getKthSegment(diffIndex);
  uint64_t histLoopCnt, curLoopCnt;
  auto histLoopCnt = histSeg->getLoopCount();
  auto curLoopCnt = curSeg->getLoopCount();
  RAW_CHECK(histLoopCnt > curLoopCnt, "unexpecting hist loop count <= cur loop\
          count");
#endif
  return false;
}

/*
 * This function analyes case when T(histLabel, diffIndex) and 
 * T(curLabel, diffIndex) are the same implicit task, T'. T(histLabel) and 
 * T(curLabel) are descendent tasks of T'. T(histLabel, diffIndex+1) is
 * explicit task, T(curLabel, diffIndex+1) is implicit task. For the same
 * reason described in comment of `analyeNextImpWork`, implicit task 
 * T(curLabel, diffIndex+1) must not be created before explicit task
 * T(histLabel, diffIndex+1). 
 */
bool analyzeNextExpImp(Label* histLabel, Label* curLabel, int diffIndex) {
#ifdef DBEUG_CORE
  auto histSeg = histLabel->getKthSegment(diffIndex);
  auto curSeg = curLabel->getKthSegment(diffIndex);
  auto histTaskcreate = histSeg->getTaskcreate();
  auto curTaskcreate = curSeg->getTaskcreate();
  RAW_CHECK(curTaskcreate > histTaskcreate, "unexpecting cur task create \
          count <= hist task create count");
#endif
  return false;
}

/*
 * This function analyzes case when T(histLabel, diffIndex) and 
 * T(curLabel, diffIndex) are the same implicit task, T'. T(histLabel) and
 * T(curLabel) are descendent tasks of T'. T(histLabel, diffIndex+1) is 
 * explicit task, T(curLabel, diffIndex+1) is also explicit task. 
 * Check syncrhonization that could affect the happens-before relation. 
 * e.g., taskwait, taskgroup
 */
bool analyzeNextExpExp(Label* histLabel, Label* curLabel, int diffIndex) {
  auto histSeg = histLabel->getKthSegment(diffIndex);
  auto curSeg = curLabel->getKthSegment(diffIndex);
  auto histTaskwait = histSeg->getTaskwait();
  auto curTaskwait = curSeg->getTaskwait();
  //TODO: 
  return true;
}

bool analyzeNextExpWork(Label* histLabel, Label* curLabel, int diffIndex) {
  return true;
}

bool analyzeNextWorkImp(Label* histLabel, Label* curLabel, int diffIndex) {
  return true;
}

bool analyzeNextWorkExp(Label* histLabel, Label* curLabel, int diffIndex) {
  return true;
}

bool analyzeNextWorkWork(Label* histLabel, Label* curLabel, int diffIndex) {
  return true;
}



uint32_t computeExitRank(uint32_t phase) {
  return phase - (phase % 2); 
}

uint32_t computeEnterRank(uint32_t phase) {
  return phase + (phase % 2);
}

/*
 * This function determines whether the finish of task T(label, startIndex) 
 * finalizes all its descendent tasks. i.e., not possible for any one of 
 * its descendent tasks to continue exists after T(label, startIndex) finishes.
 */
bool inFinishScope(Label* label, int startIndex) {
  auto lenLabel = label->getLabelLength();
  if (startIndex == lenLabel - 1) {
    // T(label, startIndex) is already the leaf task
    return true;
  } 
  auto seg = label->getKthSegment(startIndex);
  auto segType = seg->getType();
  if (segType == eImplicit) {
    // descendent tasks of this implicit task will finish as the implicit
    // task finishes 
    return true;
  }
  // label[startIndex] is either a workshare segment or an explicit segment
  auto taskGroupLevel = seg->getTaskGroupLevel();
  if (taskGroupLevel > 0) {
    // T(label) is wrapped in a taskgroup construct, should finish before 
    // T(label, startIndex) finish
    return true;
  }
   
  for (int i = startIndex; i < lenLabel; ++i) {
    auto seg = label->getKthSegment(i);   
    auto segType = seg->getType();
    if (segType == eImplicit) {
      return true;
    } 
    // if 
    auto taskGroupLevel = seg->getTaskGroupLevel();
    if (taskGroupLevel > 0) {
       
    }
  } 
  return true;
}

/*
 * Build the check case. Avoid conditional instructions. Concatenate 
 * bit level representation of two segment type to form a case code and 
 * directly  
 */
inline CheckCase buildCheckCase(SegmentType histType, SegmentType curType) {
  return static_cast<CheckCase>(histType | (curType << CASE_SHIFT));
}

/*
 * Helper function to dispatch different checking procedures based 
 * on the type of label segments of hist[diffIndex+1], cur[diffIndex+1]
 */
bool dispatchAnalysis(CheckCase checkCase, Label* hist, Label* cur, 
        int diffIndex) {
  switch(checkCase) {
    case eImpImp:
      RAW_LOG(FATAL, "not expected case: imp-imp");
    case eImpExp:
      return analyzeNextImpExp(hist, cur, diffIndex);
    case eImpWork:
      return analzyeNextImpWork(hist, cur, diffIndex);
    case eExpImp:
      return analyzeNextExpImp(hist, cur, diffIndex);
    case eExpExp:
      return analyzeNextExpExp(hist, cur, diffIndex);
    case eExpWork:
      return analyzeNextExpWork(hist, cur, diffIndex);
    case eWorkImp:
      return analyzeNextWorkImp(hist, cur, diffIndex);
    case eWorkExp:
      return analyzeNextWorkExp(hist, cur, diffIndex);
    case eWorkWork:
      return analyzeNextWorkWork(hist, cur, diffIndex);
  }
  return false;
}


}
