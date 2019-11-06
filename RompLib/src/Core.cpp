#include "Core.h"

#include <glog/logging.h>
#include <glog/raw_logging.h>

namespace romp {

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
 * This function analyze the happens-before relationship between two memory
 * accesses based on their associated task labels. The idea is that task label
 * encodes nodes relationship in openmp task graph. If there exists a directed
 * path from node A to node B, node A `happens-before` node B. Otherwise, node
 * A is logically concurrent with node B. Note that instead of doing explicit 
 * graph traversal, we traverse label segments. 
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
      // two different implicit tasks in the same parallel region
      return analyzeDiffSegImpImp(histLabel, curLabel, diffIndex);
    }  
  }
}

/*
 * This function analyzes happens-before relation when first pair of different 
 * segments are implicit segment, where offset are different. This means the 
 * two tasks associated with the two labels are descendant tasks of two sibling
 * implicit tasks.
 * Return true if T(histLabel) -> T(curLabel)
 * Return false otherwise
 */
bool analyzeDiffSegImpImp(Label* histLabel, Label* curLabel, int diffIndex) { 
  auto histLabelLength = histLabel->getLabelLength();
  auto curLabelLength = curLabel->getLabelLength();
  if (histLabelLength == curLabelLength && 
          diffIndex == (histLabelLength - 1)) {
    // T(histLabel) and T(curLabel) are leaf tasks.
    return false;
  }
  return true;
}
  

}
