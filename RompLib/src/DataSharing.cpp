#include "DataSharing.h"

#include <glog/logging.h>
#include <glog/raw_logging.h>

#include "ThreadData.h"

namespace romp {
  
/*
 * Analayze data sharing property of current memory access. 
 * Return the type of data sharing if the analysis is successful, 
 * otherwise return eUndefined. We assert threadDataPtr is not nullptr;
 * TODO: implement logic for checking if the memory access is for a 
 * task private data stored in explicit task's runtime data structure.
 */
DataSharingType analyzeDataSharing(const void* threadDataPtr, 
                        const void* address,
                        const ompt_frame_t& taskFrame) {
  if (taskFrame.exit_frame == ompt_data_none || !(taskFrame.exit_frame.ptr)) {
    RAW_LOG(INFO, "%s\n", "exit frame is not set");      
    return eUndefined;
  }
  const auto curExitFrameAddr = taskFrame.exit_frame.ptr;
  const auto threadData = static_cast<ThreadData*>(threadDataPtr);
  const auto stackTopAddr = threadData->stackTopAddr;
  const auto stackBaseAddr = threadData->stackBaseAddr;
  if (!stackTopAddr || !stackBaseAddr) {
    RAW_LOG(INFO, "%s\n", "thread stack bound is not completely set");
    return eUndefined;
  }
  const auto addressValue = static_cast<uint64_t>(address);
  if (addressValue < static_cast<uint64_t>(stackBaseAddr) || 
      addressValue > static_cast<uint64_t>(stackTopAddr)) {
    // Current memory access falls out of the thread stack's 
    // top and bottom boundary. Then the memory access is a 
    // non thread private access.
    return eNonThreadPrivate;
  } 
  if (addressValue < static_cast<uint64_t>(curExitFrameAddr)) {
      return eThreadPrivateBelowExit;
    } else {
      return eThreadPrivateAboveExit;
    }
  }
  return eUndefined;
}

}



