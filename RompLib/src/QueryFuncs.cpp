#include "QueryFuncs.h"

#include <glog/logging.h>
#include <glog/raw_logging.h>


namespace romp {

/* 
 * Helper function to determine if the query function get available result.
 */
bool infoIsAvailable(const int& retVal) { 
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
 * Query openmp task information given the task level and specified query type.
 * On success, return the pointer to the information. Otherwise, return nullptr.
 */
void* queryTaskInfo(const int& ancestorLevel,
                    const OmptTaskQueryType& queryType, 
                    int& taskType,
                    int& threadNum) {
  int retVal = -1;
  if (queryType == eTaskData) {
    ompt_data_t omptTaskData;
    auto taskDataPtr = &omptTaskData;
    auto taskDataPtrPtr = &taskDataPtr;
    auto retVal = omptGetTaskInfo(0, &taskType, taskDataPtrPtr, NULL, NULL, 
                                    &threadNum);
  } else if (queryType == eTaskFrame) {
    //TODO: implement the query task frame procedure
  } else if (queryType == eParallelData) {
    //TODO: implement the query parallel data procedure
  } else {
    RAW_LOG(FATAL, "%s\n", "unknown query type");  
  }
  if (isAvailable(retVal)) {
    return taskDataPtr->ptr; 
  } else {
    return nullptr; 
  }
}

}
