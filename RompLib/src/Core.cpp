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
  int diffIndex;
  auto histBeforeCur = happensBefore(histLabel, curLabel, diffIndex);
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
bool happensBefore(Label* histLabel, Label* curLabel, int& diffIndex) {
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
        return analyzeSiblingImpTask(histLabel, curLabel, diffIndex);
      }
    } else { 
      return analyzeSameImpTask(histLabel, curLabel, diffIndex); 
    }
  }
}

/*
 * This function analyzes happens-before relation when first pair of different 
 * segments are implicit segment, where offset are different. This means the 
 * T(histLabel, diffIndex) and T(curLabel, diffIndex) are two sibling implicit 
 * tasks in the same parallel region. T(histLabel) and T(curLabel) are 
 * descendent tasks of T(histLabel, diffIndex), T(curLabel, diffIndex) 
 * respectively.
 *
 * Return true if T(histLabel) -> T(curLabel)
 * Return false if T(histLabel) || T(curLabel)
 * Issue fatal warning if T(curLabel) -> T(histLabel)
 *
 */
bool analyzeSiblingImpTask(Label* histLabel, Label* curLabel, int diffIndex) { 
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
    uint64_t histSegLoopCount, curSegLoopCount, histPhase, curPhase;
    auto histSeg = histLabel->getKthSegment(diffIndex);
    auto curSeg = curLabel->getKthSegment(diffIndex);
    histSeg->getLoopCount(histSegLoopCount);
    curSeg->getLoopCount(curSegLoopCount);
    if (histSegLoopCount == curSegLoopCount) {
      return analyzeOrderedSection(histLabel, curLabel, diffIndex + 1);
    } 
    return false;
  }
  return false;
}

/*
 * This function analyzes if Task(histLabel) is ordered with ordered section 
 * with Task(curLabel) 
 */
bool analyzeOrderedSection(Label* histLabel, Label* curLabel, int startIndex) {
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
  if (histWorkShareId >= curWorkShareId) {
    RAW_LOG(FATAL, "not expecting hist iter id >= cur iter id");
  }
  uint64_t histPhase, curPhase;
  histBaseSeg->getPhase(histPhase);
  curBaseSeg->getPhase(curPhase);
  auto histExitRank = computeExitRank(histPhase);
  auto curEnterRank = computeEnterRank(curPhase);
  if (histExitRank < curEnterRank && 
      inFinishScope(histLabel, startIndex) &&
      inFinishScope(curLabel, startIndex)) {
    return true; 
  }
  return false;
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
bool analyzeSameImpTask(Label* histLabel, Label* curLabel, int diffIndex) {
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
 * T(histLabel) must happens before T(curLabel) because the sub parallel
 * region associated with implicit task T(histLabel, diffIndex + 1) syncs
 * all descendent tasks inside the parallel region.
 */
bool analyzeNextImpExp(Label* histLabel, Label* curLabel, int diffIndex) {
  auto histSeg = histLabel->getKthSegment(diffIndex);
  auto curSeg = curLabel->getKthSegment(diffIndex);

  return true;
}


bool analzyeNextImpWork(Label* histLabel, Label* curLabel, int diffIndex) {
  return true;
}

bool analyzeNextExpImp(Label* histLabel, Label* curLabel, int diffIndex) {
  return true;
}

bool analyzeNextExpExp(Label* histLabel, Label* curLabel, int diffIndex) {
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



uint64_t computeExitRank(uint64_t phase) {
  return phase - (phase % 2); 
}

uint64_t computeEnterRank(uint64_t phase) {
  return phase + (phase % 2);
}

bool inFinishScope(Label* label, int startIndex) {
  //TODO: implement the check of finish scope
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
