#include "DataSharing.h"

#include <glog/logging.h>
#include <glog/raw_logging.h>
#include <memory>

#include "AccessHistory.h"
#include "CoreUtil.h"
#include "QueryFuncs.h"
#include "ShadowMemory.h"
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
                                   const ompt_frame_t* taskFrame) {
  if (!threadDataPtr) {
    RAW_LOG(ERROR, "thread data pointer is null");
    return eUndefined;
  }
  if (!taskFrame->exit_frame.ptr) {
    // note that exit_frame is a union
    RAW_LOG(WARNING, "exit frame is not set");      
    return eUndefined;
  }
  const auto curExitFrameAddr = taskFrame->exit_frame.ptr;
  const auto threadData = reinterpret_cast<const ThreadData*>(threadDataPtr);
  const auto stackTopAddr = threadData->stackTopAddr;
  const auto stackBaseAddr = threadData->stackBaseAddr;
  if (!stackTopAddr || !stackBaseAddr) {
    RAW_LOG(INFO, "thread stack bound is not completely set");
    return eUndefined;
  }
  const auto addressValue = reinterpret_cast<const uint64_t>(address);
  if (addressValue < reinterpret_cast<const uint64_t>(stackBaseAddr) || 
      addressValue > reinterpret_cast<const uint64_t>(stackTopAddr)) {
    // Current memory access falls out of the thread stack's 
    // top and bottom boundary. Then the memory access is a 
    // non thread private access.
    return eNonThreadPrivate;
  } 
  if (addressValue < reinterpret_cast<const uint64_t>(curExitFrameAddr)) {
    return eThreadPrivateBelowExit;
  } else {
    return eThreadPrivateAboveExit;
  }
  return eUndefined;
}

/*
 * This function is responsible for marking memory ranges in 
 * [lowerBound, upperBound] to be deallocated.
 */
void recycleMemRange(void* lowerBound, void* upperBound) {
  auto start = reinterpret_cast<uint64_t>(lowerBound);
  auto end = reinterpret_cast<uint64_t>(upperBound);
  ShadowMemory<AccessHistory> shadowMemory;
  for (auto addr = start; addr <= end; addr++) {
    auto accessHistory = shadowMemory.getShadowMemorySlot(addr);
    std::unique_lock<std::mutex> guard(accessHistory->getMutex());
    accessHistory->setFlag(eMemoryRecycled);
  }
}

/*
 * This function is called when an explicit task is completed or is switched
 * out for other tasks. This function looks up the private memory accessed 
 * by the task, both on thread stack and in the heap allocated region by 
 * runtime. Then mark these memory location as deallocated. Thus other 
 * tasks reusing these memory regions would not trigger false positives.
 */
void recycleTaskPrivateMemory() {
  void* threadDataPtr = nullptr;
  if (!queryOmpThreadInfo(threadDataPtr)) {
    RAW_LOG(FATAL, "cannot get thread data");
    return;
  }
  auto threadData = static_cast<ThreadData*>(threadDataPtr);
  auto taskThreadUpperBound = threadData->activeTaskExitFrame;
  auto taskThreadLowerBound = threadData->lowestAccessedAddr; 
  recycleMemRange(taskThreadLowerBound, taskThreadUpperBound);
  void* taskPrivateDataBase = nullptr;
  size_t taskPrivateDataSize = 0;
  if (!queryTaskMemoryInfo(&taskPrivateDataBase, &taskPrivateDataSize)) {
    RAW_LOG(INFO, "cannot get task private data memory info");
    return;
  }
  RAW_DLOG(INFO, "task private mem base: %lx, task private data size: %lu",
           taskPrivateDataBase, taskPrivateDataSize); 
  auto taskPrivateDataEnd = computeAddressRangeEnd(taskPrivateDataBase, 
          taskPrivateDataSize);
  recycleMemRange(taskPrivateDataBase, taskPrivateDataEnd);
}

}


