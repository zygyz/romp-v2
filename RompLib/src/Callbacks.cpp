#include "Callbacks.h"

#include <glog/logging.h>
#include <glog/raw_logging.h>

#include "QueryFuncs.h"
#include "TaskData.h"
#include "ThreadData.h"

namespace romp {   

void on_ompt_callback_implicit_task(
       ompt_scope_endpoint_t endPoint,
       ompt_data_t* parallelData,
       ompt_data_t* taskData,
       unsigned int actualParallelism,
       unsigned int index,
       int flags) {
  RAW_LOG(INFO, "%s", "on_ompt_callback_implicit_task called");
  if (flags == ompt_task_initial) {
    return;
  }
  if (actualParallelism == 1) {
    return;
  }
  int parentTaskType, parentThreadNum;
  void* parentDataPtr;
  if (endPoint == ompt_scope_begin) {
    if (!queryTaskInfo(1, eTaskData, parentTaskType, parentThreadNum,
             parentDataPtr)) {
      RAW_LOG(FATAL, "%s", "cannot get parent task info");     
      return;
    }   
    // TODO: create label for this new implicit task 
    auto newTaskDataPtr = new TaskData();
    taskData->ptr = static_cast<void*>(newTaskDataPtr);
  } else if (endPoint == ompt_scope_end) {
    // end of the current implicit task, modify parent task's label
    // only one worker thread with index 0 is responsible for modifying 
    // the parent task label
    auto taskDataPtr = static_cast<TaskData*>(taskData->ptr);
    if (!taskDataPtr) { 
      RAW_LOG(FATAL, "%s", "task data pointer is null");
    }
    if (index == 0) { 
      if (!queryTaskInfo(1, eTaskData, parentTaskType, parentThreadNum, 
                parentDataPtr)) {
        RAW_LOG(FATAL, "%s", "cannot get parent task info");
        return;
      }  
      // TODO: get the parent task label pointer and modify the label
    }
    delete taskDataPtr; 
    taskData->ptr = nullptr;
  }
}

void on_ompt_callback_sync_region(
       ompt_sync_region_t kind,
       ompt_scope_endpoint_t endPoint,
       ompt_data_t *parallelData,
       ompt_data_t *taskData,
       const void* codePtrRa) {
  RAW_LOG(INFO, "%s", "on_ompt_callback_sync_region called");
  if (!taskData || !taskData->ptr) {
    RAW_LOG(FATAL, "%s", "task data pointer is null");  
    return;
  }
  auto taskDataPtr = static_cast<TaskData*>(taskData->ptr);
  if (kind == ompt_sync_region_barrier && endPoint == ompt_scope_end) {
    // TODO: modify the current task label
  } else if (kind == ompt_sync_region_taskwait && endPoint == ompt_scope_end) {
    // TODO: modify the current task label  
  } else if (kind == ompt_sync_region_taskgroup && endPoint == ompt_scope_begin) {
    // TODO: modify the current task label
  } else if (kind == ompt_sync_region_taskgroup && endPoint == ompt_scope_end) {
    // TODO: modify the current task label
  } 
  return;
}

void on_ompt_callback_mutex_acquired(
        ompt_mutex_t kind,
        ompt_wait_id_t waitId,
        const void *codePtrRa) {
  RAW_LOG(INFO, "%s", "on_ompt_callback_mutex_acquired called");
  int taskType, threadNum;
  void* dataPtr;
  if (!queryTaskInfo(0, eTaskData, taskType, threadNum, dataPtr)) {
    RAW_LOG(FATAL, "%s", "task data pointer is null");
    return;
  }
  auto taskDataPtr = static_cast<TaskData*>(dataPtr);
  if (kind == ompt_mutex_ordered) {
    // TODO: modify label for ordered section
  } else {
    if (taskDataPtr->lockSet == nullptr) {
      // TODO: set the lockset
    }
    // TODO add the lock to the lockset
  }
}

void on_ompt_callback_mutex_released(
        ompt_mutex_t kind,
        ompt_wait_id_t waitId,
        const void *codePtrRa) {
  RAW_LOG(INFO, "%s", "on_ompt_callback_mutex_released called");
  int taskType, threadNum;
  void* dataPtr;
  if (!queryTaskInfo(0, eTaskData, taskType, threadNum, dataPtr)) {
    RAW_LOG(FATAL, "%s", "task data pointer is null");
    return;
  } 
  auto taskDataPtr = static_cast<TaskData*>(dataPtr);
  if (kind == ompt_mutex_ordered) {
    // TODO: modify label for exiting ordered section  
  } else {
    // TODO: remove the lock from lockset
  }
}

void on_ompt_callback_work(
      ompt_work_t wsType,
      ompt_scope_endpoint_t endPoint,
      ompt_data_t *parallelData,
      ompt_data_t *taskData,
      uint64_t count,
      const void *codePtrRa) {
  RAW_LOG(INFO, "%s", "on_ompt_callback_work called");
  if (!taskData || !taskData->ptr) {
    RAW_LOG(FATAL, "%s", "task data pointer is null");
  }
  auto taskDataPtr = taskData->ptr;
  switch(wsType) {
    case ompt_work_loop: 
      handleOmpWorkLoop(endPoint, taskDataPtr, count);
      break;
    case ompt_work_sections:
      handleOmpWorkSections(endPoint, taskDataPtr, count);
      break;
    case ompt_work_single_executor:
      handleOmpWorkSingleExecutor(endPoint, taskDataPtr);
      break;
    case ompt_work_single_other:
      handleOmpWorkSingleOther(endPoint, taskDataPtr);
      break;
    case ompt_work_workshare:
      handleOmpWorkWorkShare(endPoint, taskDataPtr, count);
      break;
    case ompt_work_distribute:
      handleOmpWorkDistribute(endPoint, taskDataPtr, count);
      break;
    case ompt_work_taskloop:
      handleOmpWorkTaskLoop(endPoint, taskDataPtr, count);
      break;
    default:
      break;
  }
}

void on_ompt_callback_parallel_begin(
       ompt_data_t *encounteringTaskData,
       const ompt_frame_t *encounteringTaskFrame,
       ompt_data_t *parallelData,
       unsigned int requestedParallelism,
       int flags,
       const void *codePtrRa) {

}

void on_ompt_callback_parallel_end( 
       ompt_data_t *parallelData,
       ompt_data_t *encounteringTaskData,
       int flags,
       const void *codePtrRa) {

}  

void on_ompt_callback_task_create(
        ompt_data_t *encounteringTaskData,
        const ompt_frame_t *encounteringTaskFrame,
        ompt_data_t *newTaskData,
        int flags,
        int hasDependences,
        const void *codePtrRa) {

}

void on_ompt_callback_task_schedule(
        ompt_data_t *priorTaskData,
        ompt_task_status_t priorTaskStatus,
        ompt_data_t *nextTaskData) {

}

void on_ompt_callback_dependences(
        ompt_data_t *taskData,
        const ompt_dependence_t *deps,
        int ndeps) {

}

void on_ompt_callback_thread_begin(
       ompt_thread_t threadType,
       ompt_data_t *threadData) {
  if (!threadData) {
    return;
  }
  auto newThreadData = new ThreadData();
  if (!newThreadData) {
    RAW_LOG(FATAL, "%s", "failed to create thread data");
    return;
  }
  threadData->ptr = static_cast<void*>(newThreadData);
  void* stackAddr = nullptr;
  uint64_t stackSize = 0;
  if (!queryThreadStackInfo(stackAddr, stackSize)) {
    RAW_LOG(WARNING, "%s", "failed to get thread stack info");
    return;
  }
  newThreadData->stackBaseAddr = stackAddr;
  auto stackTopAddr = reinterpret_cast<void*>(
           reinterpret_cast<uint64_t>(stackAddr) +
           static_cast<uint64_t>(stackSize));             
  newThreadData->stackTopAddr = stackTopAddr;    
}

void on_ompt_callback_thread_end(
       ompt_data_t *threadData) {
  if (!threadData) {
    return;
  }
  auto dataPtr = threadData->ptr;
  if (!dataPtr) {
    delete static_cast<ThreadData*>(dataPtr);
  }
  threadData->ptr = nullptr;
}

void on_ompt_callback_dispatch(
       ompt_data_t *parallelData,
       ompt_data_t *taskData,
       ompt_dispatch_t kind,
       ompt_data_t instance) {

}

void on_ompt_callback_reduction(
       ompt_sync_region_t kind,
       ompt_scope_endpoint_t endPoint,
       ompt_data_t *parallelData,
       ompt_data_t *taskData,
       const void *codePtrRa) {

}

inline void handleOmpWorkLoop(ompt_scope_endpoint_t endPoint, 
                             void* taskData, uint64_t count) {
  auto taskDataPtr = static_cast<TaskData*>(taskData);
  if (endPoint == ompt_scope_begin) {
    

  } else if (endPoint == ompt_scope_end) {

  }  
}

inline void handleOmpWorkSections(ompt_scope_endpoint_t endPoint, 
                                  void* taskData, uint64_t count) {

}

inline void handleOmpWorkSingleExecutor(ompt_scope_endpoint_t endPoint, 
                                        void* taskData) {

}

inline void handleOmpWorkSingleOther(ompt_scope_endpoint_t endPoint, 
                                     void* taskData) {

}
    
inline void handleOmpWorkWorkShare(ompt_scope_endpoint_t endPoint, 
                                   void* taskData, uint64_t count) {

}

inline void handleOmpWorkDistribute(ompt_scope_endpoint_t endPoint, 
                                    void* taskData, uint64_t count) {

}

inline void handleOmpWorkTaskLoop(ompt_scope_endpoint_t endPoint, 
                                  void* taskData, uint64_t count) {

}

}




