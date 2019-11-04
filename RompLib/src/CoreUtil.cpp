#include "CoreUtil.h"

#include <glog/logging.h>
#include <glog/raw_logging.h>

namespace romp {

/*
 * Called by `checkAccess`. This function prepares all information 
 * for data race detection algorithm. This function does best effort to 
 * retrieve all necessary info. But consumer is still responsible for checking 
 * if the data is actually set. Return false if core information such as 
 * task data is not available.
 */
bool prepareAllInfo(int& taskType, 
                    int& teamSize, 
                    int& threadNum, 
                    void*& curParRegionData,
                    void*& curThreadData,
                    AllTaskInfo& allTaskInfo) {
  if (!queryParallelInfo(0, teamSize, curParRegionData)) {
    RAW_DLOG(INFO, "parallel region is not setup yet");
    return false;
  }
  if (!queryAllTaskInfo(0, taskType, threadNum, allTaskInfo)) {
    RAW_DLOG(INFO, "task data info is not available");
    // it is necessary to have parallel region set up 
    return false;
  }
  queryOmpThreadInfo(curThreadData);
  return true;
}

void reportDataRace(void* instnAddrPrev, void* instnAddrCur, void* address) {
  //TODO: add source line information
  RAW_LOG(INFO, "data race found: 0x%lx 0x%lx @ 0x%lx", instnAddrPrev, 
          instnAddrCur, address);
}

}
