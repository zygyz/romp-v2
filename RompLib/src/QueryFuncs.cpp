#include "QueryFuncs.h"

#include <glog/logging.h>
#include <glog/raw_logging.h>


namespace romp {

/* 
 * Helper function to determine if the query function get available result.
 */
bool infoIsAvailable(const int retVal) { 
  if (retVal == 0) {
    // task does not exist
    return false; 
  } else if (retVal == 1) {
    // task exists at the specified ancestor level but the information 
    // is not available 
    RAW_LOG(WARNING, "%s\n",  "task exists but info is not available");
    return false;
  } else if (retVal == 2) {
    // task exists at the specified ancestor level and the information
    // is available
    return true;
  } else {
    RAW_LOG(FATAL, "%s\n", "unknown return value");
    return false;
  }
}

/*
 * Query all openmp task information given the task level in one time.
 * This function could be called when multiple aspects of information about 
 * openmp task is needed. 
 */
bool queryAllTaskInfo(const int ancestorLevel, 
                      int& taskType,
                      int& threadNum,
                      AllTaskInfo& allTaskInfo) {
  auto taskDataPtr = &(allTaskInfo.taskData);
  auto taskDataPtrPtr = &taskDataPtr;
  auto taskFramePtr = &(allTaskInfo.taskFrame);
  auto taskFramePtrPtr = &taskFramePtr;
  auto parDataPtr = &(allTaskInfo.parallelData);
  auto parDataPtrPtr = &parDataPtr; 
  auto retVal = omptGetTaskInfo(ancestorLevel, &taskType, taskDataPtrPtr,
                                taskFramePtrPtr, parDataPtrPtr, &threadNum);
  return infoIsAvailable(retVal);
}

/*
 * Query openmp task information given the task level and specified query type.
 * On success, return the pointer to the information. Otherwise, return nullptr.
 */
void* queryTaskInfo(const int ancestorLevel,
                    const OmptTaskQueryType& queryType, 
                    int& taskType,
                    int& threadNum) {
  int retVal = -1;
  ompt_data_t omptTaskData;
  auto taskDataPtr = &omptTaskData;
  if (queryType == eTaskData) {
    auto taskDataPtrPtr = &taskDataPtr;
    retVal = omptGetTaskInfo(ancestorLevel, &taskType, taskDataPtrPtr, 
                                  NULL, NULL, &threadNum);
  } else if (queryType == eTaskFrame) {
    //TODO: implement the query task frame procedure
     
  } else if (queryType == eParallelData) {
    //TODO: implement the query parallel data procedure
  } else {
    RAW_LOG(FATAL, "%s\n", "unknown query type");  
  }
  if (infoIsAvailable(retVal)) {
    return taskDataPtr->ptr; 
  } else {
    return nullptr; 
  }
}

/*
 * Query openmp runtime information about the parallel region. 
 * On success, return pointer to parallel region data. Otherwise, 
 * return nullptr.
 */
void* queryParallelInfo(
        const int ancestorLevel,
        int& teamSize) {
  ompt_data_t omptParData;
  auto parDataPtr = &omptParData;
  auto parDataPtrPtr = &parDataPtr;
  auto retVal = omptGetParallelInfo(ancestorLevel, parDataPtrPtr, &teamSize);
  if (infoIsAvailable(retVal)) {
    return parDataPtr->ptr;
  } else {
    return nullptr;
  }
}

/*
 * Query openmp runtime information about the thread. 
 * On success, return pointer to thread data. Otherwise, return nullptr;
 */
void* queryThreadInfo() {
  auto curThreadData = omptGetThreadData();
  if (!curThreadData) {
    return nullptr;
  }
  return curThreadData->ptr;
}

}
