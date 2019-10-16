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
 * If the information is available, set dataPtr to the pointer to actual data, 
 * then return true. If the information is not available, set dataPtr to 
 * nullptr and return false. 
 */
bool queryTaskInfo(const int ancestorLevel,
                   const OmptTaskQueryType& queryType, 
                   int& taskType,
                   int& threadNum,
                   void*& dataPtr) {
  int retVal = -1;
  ompt_data_t omptTaskData;
  dataPtr = nullptr;
  auto taskDataPtr = &omptTaskData;
  if (queryType == eTaskData) {
    auto taskDataPtrPtr = &taskDataPtr;
    retVal = omptGetTaskInfo(ancestorLevel, &taskType, taskDataPtrPtr, 
                                  NULL, NULL, &threadNum);
    dataPtr = taskDataPtr->ptr;
  } else if (queryType == eTaskFrame) {
    //TODO: implement the query task frame procedure
     
  } else if (queryType == eParallelData) {
    //TODO: implement the query parallel data procedure
  } else {
    RAW_LOG(FATAL, "%s\n", "unknown query type");  
  }
  return infoIsAvailable(retVal);
}

/*
 * Query openmp runtime information about the parallel region. 
 * On success, set dataPtr to pointer to parallel region data, and return true. 
 * Otherwise, set dataPtr to nullptr and return false.
 */
bool queryParallelInfo(
        const int ancestorLevel,
        int& teamSize,
        void*& dataPtr) {
  dataPtr = nullptr; 
  ompt_data_t omptParData;
  auto parDataPtr = &omptParData;
  auto parDataPtrPtr = &parDataPtr;
  auto retVal = omptGetParallelInfo(ancestorLevel, parDataPtrPtr, &teamSize);
  if (!infoIsAvailable(retVal) || !(parDataPtr->ptr)) {
    return false;
  }
  dataPtr = parDataPtr->ptr;
  return true;
}

/*
 * Query openmp runtime information about the thread. 
 * If thread data pointer is not nullptr, return true and pass the pointer
 * to dataPtr. Otherwise, return false.
 */
bool queryThreadInfo(void*& dataPtr) {
  dataPtr = nullptr;
  auto curThreadData = omptGetThreadData();
  if (!curThreadData || !(curThreadData->ptr)) {
    return false;
  }
  dataPtr = curThreadData->ptr;
  return true;
}

}
