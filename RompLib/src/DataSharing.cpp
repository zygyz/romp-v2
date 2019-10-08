#include "DataSharing.h"

#include <glog/logging.h>
#include <glog/raw_logging.h>

#include "ThreadData.h"

namespace romp {
  
/*
 * Analayze data sharing property of current memory access. 
 * Return true if the analysis is successful, otherwise return false.
 * If analysis is successful, data sharing type is set in dataSharingType.
 * We assert threadDataPtr is not nullptr;
 */
bool analyzeDataSharing(const void* threadDataPtr, 
                        const void* address,
                        const ompt_frame_t& currentExitFrame,
                        DataSharingType& dataSharingType) {
  if (currentExitFrame.exit_frame == ompt_data_none || 
      currentExitFrame.exit_frame.ptr == nullptr) {
    RAW_LOG(INFO, "%s\n", "exit frame is not set");      
    return false;
  }
  const auto curExitFrameAddr = currentExitFrame.exit_frame.ptr;
  const auto threadData = static_cast<ThreadData*>(threadDataPtr);
  const auto stackTopAddr = threadData->stackTopAddr;
  const auto stackBaseAddr = threadData->stackBaseAddr;
  if (!stackTopAddr || !stackBaseAddr) {
    RAW_LOG(INFO, "%s\n", "thread stack bound is not completely set");
    return false;
  }
  const auto addressValue = static_cast<uint64_t>(address);
  if (addressValue < static_cast<uint64_t>(stackBaseAddr) || 
      addressValue > static_cast<uint64_t>(stackTopAddr)) {
    dataSharingType = eNonThreadPrivate;
  } else {
    if (addressValue < static_cast<uint64_t>(curExitFrameAddr)) {
      dataSharingType = eThreadPrivateBelowExit;
    } else {
      dataSharingType = eThreadPrivateAboveExit;
    }
  }
  return true;
}

}



