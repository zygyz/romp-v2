#include "CoreUtil.h"

#include <glog/logging.h>
#include <glog/raw_logging.h>

namespace romp {

/*
 * Called by `checkAccess`. This function prepares all information necessary 
 * for data race detection algortihm. Return true if all information is available
 * . Return false if some information is not 
 * available. Note that it is possible that information is available but 
 * the actual pointer to data structure is nullptr. 
 */
bool prepareAllInfo(int& taskType, 
                    int& teamSize, 
                    int& threadNum, 
                    void*& curParRegionData,
                    void*& curThreadData,
                    AllTaskInfo& allTaskInfo) {
  if (!queryAllTaskInfo(0, taskType, threadNum, allTaskInfo)) {
    // task info is not available
    return false;
  }
  if (!queryParallelInfo(0, teamSize, curParRegionData)) {
    return false; // parallel region is not available
  }
  if (!queryThreadInfo(curThreadData)) {
    // thread data is not available
    return false;
  }
  return true;
}

}
