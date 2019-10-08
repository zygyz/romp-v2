#include <glog/logging.h>
#include <glog/raw_logging.h>

#include "DataSharing.h"
#include "Initialize.h"
#include "Label.h"
#include "LockSet.h"
#include "TaskData.h"
#include "ThreadData.h"
#include "QueryFuncs.h"

namespace romp {

using LabelPtr = std::shared_ptr<Label>;
using LockSetPtr = std::shared_ptr<LockSet>;

extern "C" {

/** 
 * implement ompt_start_tool which is defined in OpenMP spec 5.0
 */
ompt_start_tool_result_t* ompt_start_tool(
        unsigned int ompVersion,
        const char* runtimeVersion) {
  ompt_data_t data;
  static ompt_start_tool_result_t startToolResult = { 
      &omptInitialize, &omptFinalize, data}; 
  LOG(INFO) << "ompt_start_tool";
  return &startToolResult;
}

void checkAccess(void* address,
            uint32_t bytesAccessed,
            uint64_t instnAddr,
            bool hwLock,
            bool isWrite) {
  RAW_LOG(INFO, "address:%lx bytesAccessed:%u instnAddr: %lx hwLock: %u,"
                "isWrite: %u", address, bytesAccessed, instnAddr, 
                 hwLock, isWrite);
  if (!gOmptInitialized) {
    RAW_LOG(INFO, "%s", "ompt not initialized yet");
    return;
  }
  AllTaskInfo allTaskInfo;
  int threadNum, taskType;
  if (!queryAllTaskInfo(0, taskType, threadNum, allTaskInfo)) {
    // task info is not available
    return;
  }
  if (!allTaskInfo.taskData.ptr) { // pointer to task data is not set
    return;
  }
  int teamSize;
  auto parRegionInfo = queryParallelInfo(0, teamSize);
  if (parRegionInfo == nullptr) {
    return;
  }
  auto curThreadData = queryThreadInfo();
  if (!curThreadData) {
    RAW_LOG(INFO, "%s\n", "thread data not set yet"); 
    return;
  }
  // query data  
  auto curTaskData = static_cast<TaskData*>(allTaskInfo.taskData.ptr);
  auto currentLabel = curTaskData->label;
  auto currentLockSet = curTaskData->lockSet; 

  if (!analyzeDataS
}

}

}

