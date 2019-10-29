#include "Callbacks.h"

#include <glog/logging.h>
#include <glog/raw_logging.h>

#include "AccessHistory.h"
#include "Label.h"
#include "ParRegionData.h"
#include "QueryFuncs.h"
#include "ShadowMemory.h"
#include "TaskData.h"
#include "ThreadData.h"

namespace romp {   

extern ShadowMemory<AccessHistory> shadowMemory;
   
void on_ompt_callback_implicit_task(
       ompt_scope_endpoint_t endPoint,
       ompt_data_t* parallelData,
       ompt_data_t* taskData,
       unsigned int actualParallelism,
       unsigned int index,
       int flags) {
  RAW_LOG(INFO, "%s", "on_ompt_callback_implicit_task called");
  if (flags == ompt_task_initial || actualParallelism == 1) {
    /*
     * TODO: looks like ompt_task_initial never shows up. Could be 
     * some bug in runtime libray. If the actualParallelism is 1, it 
     * should be an initial task.
     */
    return;
  }
  auto taskDataPtr = static_cast<TaskData*>(taskData->ptr);
  if (actualParallelism == 0 && index != 0) {
    /* 
     * Parallelism is 0 means that it is end of task, index != 0 means
     * that it is not the master thread, simply release the memory and
     * return. implicit-task-end and initial-task-end events.
     * We have to do it here before getting parent task because somehow 
     * the runtime library won't be able to get parent task for this case.
    */
    if (!taskDataPtr) {
      RAW_LOG(FATAL, "%s", "task data pointer is null");
    }
    delete taskDataPtr; 
    taskData->ptr = nullptr;
    return;
  }
  int parentTaskType, parentThreadNum;
  void* parentDataPtr;
  if (!queryTaskInfo(1, eTaskData, parentTaskType, parentThreadNum,
             parentDataPtr)) {
    RAW_LOG(FATAL, "%s", "cannot get parent task info");     
    return;
  }   
  auto parentTaskData = static_cast<TaskData*>(parentDataPtr);
  if (endPoint == ompt_scope_begin) {
    // begin of implcit task, create the label for this new task
    auto newTaskLabel = genImpTaskLabel(parentTaskData->label, index, 
            actualParallelism);
    auto newTaskDataPtr = new TaskData();
    newTaskDataPtr->label = newTaskLabel;
    taskData->ptr = static_cast<void*>(newTaskDataPtr);
  } else if (endPoint == ompt_scope_end) {
    /* 
     * End of the current implicit task, modify parent task's label
     * only one worker thread with index 0 is responsible for mutating
     * the parent task label. The mutated label should be created separately
     * because access history referred to labels by pointer.
     */
    if (!taskDataPtr) { 
      RAW_LOG(FATAL, "%s", "task data pointer is null");
    }
    auto parentLabel = parentTaskData->label; 
    auto mutatedLabel = mutateParentImpEnd(parentLabel, taskDataPtr->label);
    parentTaskData->label = mutatedLabel;
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
  auto label = taskDataPtr->label;
  if (endPoint == ompt_scope_begin) {
    
  } else if (endPoint == ompt_scope_end) {

  }
  if (kind == ompt_sync_region_barrier && endPoint == ompt_scope_end) {
    auto mutatedLabel = mutateBarrierEnd(label);
    taskDataPtr->label = mutatedLabel; 
  } else if (kind == ompt_sync_region_taskwait && endPoint == ompt_scope_end) {
    auto mutatedLabel = mutateTaskWait(label);
    taskDataPtr->label = mutatedLabel; 
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
  auto label = taskDataPtr->label;
  std::shared_ptr<Label> mutatedLabel = nullptr;
  if (kind == ompt_mutex_ordered) {
    mutatedLabel = mutateOrderSection(label); 
  } else {
    if (taskDataPtr->lockSet == nullptr) {
      // TODO: set the lockset
    }
    // TODO add the lock to the lockset
  }
  taskDataPtr->label = mutatedLabel;
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
  auto label = taskDataPtr->label;
  std::shared_ptr<Label> mutatedLabel = nullptr; 
  if (kind == ompt_mutex_ordered) {
    mutatedLabel = mutateOrderSection(label);
  } else {
    // TODO: remove the lock from lockset
  }
  taskDataPtr->label = mutatedLabel; 
}

/*
 * Create a mutated label upon entering/exiting workshare loop construct
 */
inline std::shared_ptr<Label> handleOmpWorkLoop(
                             ompt_scope_endpoint_t endPoint, 
                             const std::shared_ptr<Label>& label) {
  std::shared_ptr<Label> mutatedLabel = nullptr;
  if (endPoint == ompt_scope_begin) {
    mutatedLabel = mutateLoopBegin(label);
  } else if (endPoint == ompt_scope_end) {
    mutatedLabel = mutateLoopEnd(label);
  }  
  return mutatedLabel;
}

/*
 * Create a mutated label label upon entering/exiting workshare 
 * section construct
 */
inline std::shared_ptr<Label> handleOmpWorkSections(
        ompt_scope_endpoint_t endPoint, 
        const std::shared_ptr<Label>& label,
        uint64_t count) {
  std::shared_ptr<Label> mutatedLabel = nullptr;
  if (endPoint == ompt_scope_begin) {
    mutatedLabel = mutateSectionBegin(label);
  } else if (endPoint == ompt_scope_end) {
    mutatedLabel = mutateSectionEnd(label);
  }
  return mutatedLabel;
}

inline std::shared_ptr<Label> handleOmpWorkSingleExecutor(
        ompt_scope_endpoint_t endPoint, 
        const std::shared_ptr<Label>& label) {
  std::shared_ptr<Label> mutatedLabel = nullptr;
  if (endPoint == ompt_scope_begin) {
    mutatedLabel = mutateSingleExecBegin(label);
  } else if (endPoint == ompt_scope_end) {
    mutatedLabel = mutateSingleEnd(label);  
  }
  return mutatedLabel;
}

inline std::shared_ptr<Label> handleOmpWorkSingleOther(
        ompt_scope_endpoint_t endPoint, 
        const std::shared_ptr<Label>& label) {
  std::shared_ptr<Label> mutatedLabel = nullptr;
  if (endPoint == ompt_scope_begin) {
    mutatedLabel = mutateSingleOtherBegin(label);
  } else if (endPoint == ompt_scope_end) {
    mutatedLabel = mutateSingleEnd(label);
  }
  return mutatedLabel;
}
    
inline std::shared_ptr<Label> handleOmpWorkWorkShare(
        ompt_scope_endpoint_t endPoint, 
        const std::shared_ptr<Label>& label, 
        uint64_t count) {
  RAW_LOG(FATAL, "%s", "c++ openmp does not support workshare construct");
  return nullptr;
}

inline std::shared_ptr<Label> handleOmpWorkDistribute(
        ompt_scope_endpoint_t endPoint, 
        const std::shared_ptr<Label>& label, 
        uint64_t count) {
  //TODO: This is assoicated with target and team construct
  RAW_LOG(FATAL, "%s", "not implemented yet");
  return nullptr;
}

/*
 * Taskloop is another worksharing construct that is like the worksharing
 * for-loop. The difference is that taskloop construct creates explicit 
 * tasks to execute the logical iterations in the loops.  
 */
inline std::shared_ptr<Label> handleOmpWorkTaskLoop(
        ompt_scope_endpoint_t endPoint, 
        const std::shared_ptr<Label>& label, 
        uint64_t count) {
  // TODO: determine label mutation rule for taskloop begin
  RAW_LOG(INFO, "task loop %lu", count);
  /*
  std::shared_ptr<Label> mutatedLabel = nullptr;
  if (endPoint == ompt_scope_begin) {
    mutatedLabel = mutateTaskLoopBegin(label);
  } else if (endPoint == ompt_scope_end) {
    mutatedLabel = mutateTaskLoopEnd(label);
  }
  return mutatedLabel;
  */
  return nullptr;
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
  auto taskDataPtr = static_cast<TaskData*>(taskData->ptr);
  auto label = taskDataPtr->label;
  std::shared_ptr<Label> mutatedLabel = nullptr;
  switch(wsType) {
    case ompt_work_loop: 
      mutatedLabel = handleOmpWorkLoop(endPoint, label);
      break;
    case ompt_work_sections:
      mutatedLabel = handleOmpWorkSections(endPoint, label, count);
      break;
    case ompt_work_single_executor:
      mutatedLabel = handleOmpWorkSingleExecutor(endPoint, label);
      break;
    case ompt_work_single_other:
      mutatedLabel = handleOmpWorkSingleOther(endPoint, label);
      break;
    case ompt_work_workshare:
      mutatedLabel = handleOmpWorkWorkShare(endPoint, label, count);
      break;
    case ompt_work_distribute:
      mutatedLabel = handleOmpWorkDistribute(endPoint, label, count);
      break;
    case ompt_work_taskloop:
      mutatedLabel = handleOmpWorkTaskLoop(endPoint, label, count);
      break;
    default:
      break;
  }
  taskDataPtr->label = mutatedLabel;
}

void on_ompt_callback_parallel_begin(
       ompt_data_t *encounteringTaskData,
       const ompt_frame_t *encounteringTaskFrame,
       ompt_data_t *parallelData,
       unsigned int requestedParallelism,
       int flags,
       const void *codePtrRa) {
  auto parRegionData = new ParRegionData(requestedParallelism, flags);
  parallelData->ptr = static_cast<void*>(parRegionData);  
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
  if (flags == ompt_task_initial) {
    auto taskData = new TaskData();
    auto label = std::make_shared<Label>();
    auto segment = std::make_shared<BaseSegment>(eImplicit, 0, 1);
    label->appendSegment(segment);
    taskData->label = label;    
    newTaskData->ptr = static_cast<void*>(taskData);
  } else if (flags == ompt_task_explicit) {
    // TODO: prepare the task data pointer for newly created explicit task 
  } else if (flags == ompt_task_target) {
    // TODO: prepare the task data pointer for target 
  }
}

void on_ompt_callback_task_schedule(
        ompt_data_t *priorTaskData,
        ompt_task_status_t priorTaskStatus,
        ompt_data_t *nextTaskData) {
  if (!priorTaskStatus || !priorTaskData->ptr) {
    RAW_LOG(FATAL, "%s", "prior task data pointer is null"); 
    return;
  }
  if (!nextTaskData || !nextTaskData->ptr) {
    RAW_LOG(INFO, "%s", "next task data pointer is null");
    return;
  }
  if (priorTaskStatus == ompt_task_early_fulfill || 
          priorTaskStatus == ompt_task_late_fulfill) {
    RAW_LOG(INFO, "%s", "prior task status is early/late fulfill");
    return;
  }
  void* threadDataPtr = nullptr;
  if (!queryOmpThreadInfo(threadDataPtr)) {
    RAW_LOG(FATAL, "%s", "cannot get thread data");
    return;
  }
  auto threadData = static_cast<ThreadData*>(threadDataPtr); 
  auto priorTaskUpperBound = threadData->activeTaskExitFrame; 
  auto priorTaskLowerBound = threadData->lowestAccessedAddr;
  // mark the memory region [lowerbound, upperbound] as recycled  
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
  if (!taskData || !taskData->ptr) {
    RAW_LOG(FATAL, "%s", "task data pointer is null");
    return;
  }  
  auto taskDataPtr = static_cast<TaskData*>(taskData->ptr);
  if (endPoint == ompt_scope_begin) {
    taskDataPtr->inReduction = true;
  } else if (endPoint == ompt_scope_end) {
    taskDataPtr->inReduction = false;
  }
}


}




