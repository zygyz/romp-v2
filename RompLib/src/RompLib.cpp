#include <glog/logging.h>
#include <glog/raw_logging.h>
#include <memory>

#include "Initialize.h"
#include "Label.h"
#include "QueryFuncs.h"

namespace romp {

using LabelPtr = std::shared_ptr<Label>;

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
  int threadNum, taskType;
  auto taskInfo = queryTaskInfo(0, eTaskData, taskType, threadNum);
  if (taskInfo == nullptr) {
    return; 
  }
  int teamSize;
  auto parRegionInfo = queryParallelInfo(0, teamSize);
  if (parRegionInfo == nullptr) {
    return;
  }
  LabelPtr currentLabel = nullptr;
   
}

}

}

