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
  RAW_LOG(INFO, "on_ompt_callback_implicit_task called:%u p:%lx t:%lx %u %u %d",
          endPoint, parallelData, taskData, actualParallelism, index, flags);

  if (flags == ompt_task_initial || actualParallelism == 1) {
    /*
     * TODO: looks like ompt_task_initial never shows up. Could be 
     * some bug in runtime libray. If the actualParallelism is 1, it 
     * should be an initial task according to spec.
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
      RAW_LOG(FATAL, "task data pointer is null");
    }
    delete taskDataPtr; 
    taskData->ptr = nullptr;
    return;
  }
  int parentTaskType, parentThreadNum;
  void* parentDataPtr;
  if (!queryTaskInfo(1, eTaskData, parentTaskType, parentThreadNum,
             parentDataPtr)) {
    RAW_LOG(FATAL, "cannot get parent task info");     
    return;
  }   
  auto parentTaskData = static_cast<TaskData*>(parentDataPtr);
  if (endPoint == ompt_scope_begin) {
    // begin of implcit task, create the label for this new task
    auto newTaskLabel = genImpTaskLabel((parentTaskData->label).get(), index, 
            actualParallelism); 
    // return value optimization should avoid the ref count mod
    auto newTaskDataPtr = new TaskData();
    RAW_DLOG(INFO, "created task data ptr: 0x%lx", newTaskDataPtr);
    // cast to rvalue and avoid atomic ref count modification
    newTaskDataPtr->label = std::move(newTaskLabel); 
    taskData->ptr = static_cast<void*>(newTaskDataPtr);
  } else if (endPoint == ompt_scope_end) {
    /* 
     * End of the current implicit task, modify parent task's label
     * only one worker thread with index 0 is responsible for mutating
     * the parent task label. The mutated label should be created separately
     * because access history referred to labels by pointer.
     * At this point, only one implicit task should reach here.
     */
    if (!taskDataPtr) { 
      RAW_LOG(FATAL, "task data pointer is null");
    }
    auto parentLabel = parentTaskData->label; 
    auto mutatedLabel = mutateParentImpEnd(
            parentLabel.get(), (taskDataPtr->label).get());
    parentTaskData->label = std::move(mutatedLabel);
    delete taskDataPtr; 
    taskData->ptr = nullptr;
  }
}

/*
 * Once a task encounters a taskwait clause, mark the task's explicit children
 * to be taskwaited. So that the anlaysis algorithm knows that the explicit
 * child is synchronized with taskwait.
 */
void markExpChildrenTaskwait(TaskData* taskData) {
  for (const auto& child : taskData->childExpTaskData) {
    auto childTaskData = static_cast<const TaskData*>(child); 
    auto lenLabel = childTaskData->label->getLabelLength(); 
    childTaskData->label->getKthSegment(lenLabel - 1)->setTaskwaited();
  }
  taskData->childExpTaskData.clear(); // clear the children after taskwait
}

void on_ompt_callback_sync_region(
       ompt_sync_region_t kind,
       ompt_scope_endpoint_t endPoint,
       ompt_data_t *parallelData,
       ompt_data_t *taskData,
       const void* codePtrRa) {
  RAW_LOG(INFO,  "on_ompt_callback_sync_region called");
  if (!taskData || !taskData->ptr) {
    RAW_LOG(FATAL, "task data pointer is null");  
    return;
  }
  auto taskDataPtr = static_cast<TaskData*>(taskData->ptr);
  auto labelPtr = (taskDataPtr->label).get();  // never std::move here!
  std::shared_ptr<Label> mutatedLabel = nullptr;
  if (endPoint == ompt_scope_begin && kind == ompt_sync_region_taskgroup) {
    mutatedLabel = mutateTaskGroupBegin(labelPtr);
  } else if (endPoint == ompt_scope_end) {
    switch(kind) {
      case ompt_sync_region_taskwait:
        mutatedLabel = mutateTaskWait(labelPtr);
        markExpChildrenTaskwait(taskDataPtr);
        break;
      case ompt_sync_region_barrier:
        mutatedLabel = mutateBarrierEnd(labelPtr);
        break;
      case ompt_sync_region_taskgroup:
        mutatedLabel = mutateTaskGroupEnd(labelPtr);
        break;
      default:
        RAW_LOG(FATAL, "unknown endpoint type");
        break;
    }
  }
  taskDataPtr->label = std::move(mutatedLabel);
  return;
}

void on_ompt_callback_mutex_acquired(
        ompt_mutex_t kind,
        ompt_wait_id_t waitId,
        const void *codePtrRa) {
  RAW_LOG(INFO, "on_ompt_callback_mutex_acquired called");
  int taskType, threadNum;
  void* dataPtr;
  if (!queryTaskInfo(0, eTaskData, taskType, threadNum, dataPtr)) {
    RAW_LOG(FATAL, "task data pointer is null");
    return;
  }
  auto taskDataPtr = static_cast<TaskData*>(dataPtr);
  auto label = taskDataPtr->label;
  std::shared_ptr<Label> mutatedLabel = nullptr;
  if (kind == ompt_mutex_ordered) {
    mutatedLabel = mutateOrderSection(label.get()); 
  } else {
    if (taskDataPtr->lockSet == nullptr) {
      // TODO: set the lockset
    }
    // TODO add the lock to the lockset
  }
  taskDataPtr->label = std::move(mutatedLabel);
}

void on_ompt_callback_mutex_released(
        ompt_mutex_t kind,
        ompt_wait_id_t waitId,
        const void *codePtrRa) {
  RAW_LOG(INFO, "on_ompt_callback_mutex_released called");
  int taskType, threadNum;
  void* dataPtr;
  if (!queryTaskInfo(0, eTaskData, taskType, threadNum, dataPtr)) {
    RAW_LOG(FATAL, "task data pointer is null");
    return;
  } 
  auto taskDataPtr = static_cast<TaskData*>(dataPtr);
  auto label = taskDataPtr->label;
  std::shared_ptr<Label> mutatedLabel = nullptr; 
  if (kind == ompt_mutex_ordered) {
    mutatedLabel = mutateOrderSection(label.get());
  } else {
    // TODO: remove the lock from lockset
  }
  taskDataPtr->label = std::move(mutatedLabel);
}

/*
 * Create a mutated label upon entering/exiting workshare loop construct
 */
inline std::shared_ptr<Label> handleOmpWorkLoop(
                             ompt_scope_endpoint_t endPoint, 
                             const std::shared_ptr<Label>& label) {
  std::shared_ptr<Label> mutatedLabel = nullptr;
  auto labelPtr = label.get();
  if (endPoint == ompt_scope_begin) {
    mutatedLabel = mutateLoopBegin(labelPtr);
  } else if (endPoint == ompt_scope_end) {
    mutatedLabel = mutateLoopEnd(labelPtr);
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
  auto labelPtr = label.get();
  if (endPoint == ompt_scope_begin) {
    mutatedLabel = mutateSectionBegin(labelPtr);
  } else if (endPoint == ompt_scope_end) {
    mutatedLabel = mutateSectionEnd(labelPtr);
  }
  return mutatedLabel;
}

inline std::shared_ptr<Label> handleOmpWorkSingleExecutor(
        ompt_scope_endpoint_t endPoint, 
        const std::shared_ptr<Label>& label) {
  std::shared_ptr<Label> mutatedLabel = nullptr;
  auto labelPtr = label.get();
  if (endPoint == ompt_scope_begin) {
    mutatedLabel = mutateSingleExecBegin(labelPtr);
  } else if (endPoint == ompt_scope_end) {
    mutatedLabel = mutateSingleEnd(labelPtr);  
  }
  return mutatedLabel;
}

inline std::shared_ptr<Label> handleOmpWorkSingleOther(
        ompt_scope_endpoint_t endPoint, 
        const std::shared_ptr<Label>& label) {
  std::shared_ptr<Label> mutatedLabel = nullptr;
  auto labelPtr = label.get();
  if (endPoint == ompt_scope_begin) {
    mutatedLabel = mutateSingleOtherBegin(labelPtr);
  } else if (endPoint == ompt_scope_end) {
    mutatedLabel = mutateSingleEnd(labelPtr);
  }
  return mutatedLabel;
}
    
inline std::shared_ptr<Label> handleOmpWorkWorkShare(
        ompt_scope_endpoint_t endPoint, 
        const std::shared_ptr<Label>& label, 
        uint64_t count) {
  RAW_LOG(FATAL, "c++ openmp does not support workshare construct");
  return nullptr;
}

inline std::shared_ptr<Label> handleOmpWorkDistribute(
        ompt_scope_endpoint_t endPoint, 
        const std::shared_ptr<Label>& label, 
        uint64_t count) {
  //TODO: This is assoicated with target and team construct
  RAW_LOG(FATAL, "not implemented yet");
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
  RAW_LOG(INFO, "on_ompt_callback_work called");
  if (!taskData || !taskData->ptr) {
    RAW_LOG(FATAL, "task data pointer is null");
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
  taskDataPtr->label = std::move(mutatedLabel);
}

void on_ompt_callback_parallel_begin(
       ompt_data_t *encounteringTaskData,
       const ompt_frame_t *encounteringTaskFrame,
       ompt_data_t *parallelData,
       unsigned int requestedParallelism,
       int flags,
       const void *codePtrRa) {
  RAW_DLOG(INFO, "parallel begin et:%lx p:%lx %u %d", encounteringTaskData, 
           parallelData, requestedParallelism, flags);
  auto parRegionData = new ParRegionData(requestedParallelism, flags);
  parallelData->ptr = static_cast<void*>(parRegionData);  
}

void on_ompt_callback_parallel_end( 
       ompt_data_t *parallelData,
       ompt_data_t *encounteringTaskData,
       int flags,
       const void *codePtrRa) {
  RAW_LOG(INFO, "parallel end et:%lx p:%lx %d", encounteringTaskData, 
           parallelData, flags);
  auto parRegionData = parallelData->ptr;
  delete static_cast<ParRegionData*>(parRegionData);
}  

void on_ompt_callback_task_create(
        ompt_data_t *encounteringTaskData,
        const ompt_frame_t *encounteringTaskFrame,
        ompt_data_t *newTaskData,
        int flags,
        int hasDependences,
        const void *codePtrRa) {
  auto taskData = new TaskData();
  if (flags == ompt_task_initial) {
    RAW_LOG(INFO, "generating initial task: %lx", taskData);
    auto newTaskLabel = genInitTaskLabel();
    taskData->label = std::move(newTaskLabel);
  } else if (flags == ompt_task_explicit) {
    // create label for explicit task
    auto parentTaskData = static_cast<TaskData*>(encounteringTaskData->ptr);
    if (!parentTaskData || !parentTaskData->label) {
      RAW_LOG(FATAL, "cannot get parent task label");
      return;
    }
    auto parentLabel = (parentTaskData->label).get();
    auto newTaskLabel = genExpTaskLabel(parentLabel);
    taskData->label = std::move(newTaskLabel);
    parentTaskData->childExpTaskData.push_back(static_cast<void*>(taskData));
  } else if (flags == ompt_task_target) {
    // TODO: prepare the task data pointer for target 
    RAW_LOG(FATAL, "ompt_task_target not implemented yet");
  }
  newTaskData->ptr = static_cast<void*>(taskData);
}

void on_ompt_callback_task_schedule(
        ompt_data_t *priorTaskData,
        ompt_task_status_t priorTaskStatus,
        ompt_data_t *nextTaskData) {
  if (!priorTaskStatus || !priorTaskData->ptr) {
    RAW_LOG(FATAL, "prior task data pointer is null"); 
    return;
  }
  if (!nextTaskData || !nextTaskData->ptr) {
    RAW_LOG(INFO, "next task data pointer is null");
    return;
  }
  if (priorTaskStatus == ompt_task_early_fulfill || 
          priorTaskStatus == ompt_task_late_fulfill) {
    RAW_LOG(INFO, "prior task status is early/late fulfill");
    return;
  }
  void* threadDataPtr = nullptr;
  if (!queryOmpThreadInfo(threadDataPtr)) {
    RAW_LOG(FATAL, "cannot get thread data");
    return;
  }
  auto threadData = static_cast<ThreadData*>(threadDataPtr); 
  auto priorTaskUpperBound = threadData->activeTaskExitFrame; 
  auto priorTaskLowerBound = threadData->lowestAccessedAddr;
  // mark the memory region [lowerbound, upperbound] as recycled  
  // TODO
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
    RAW_LOG(WARNING, "thread data is null");
    return;
  }
  auto newThreadData = new ThreadData();
  if (!newThreadData) {
    RAW_LOG(FATAL, "failed to create thread data");
    return;
  }
  threadData->ptr = static_cast<void*>(newThreadData);
  void* stackAddr = nullptr;
  uint64_t stackSize = 0;
  if (!queryThreadStackInfo(stackAddr, stackSize)) {
    RAW_LOG(WARNING, "failed to get thread stack info");
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
  if (!taskData || !taskData->ptr) {
    RAW_LOG(FATAL, "cannot get task data info");
    return;
  }
  auto taskDataPtr = static_cast<TaskData*>(taskData->ptr);
  auto parentLabel = (taskDataPtr->label).get();
  std::shared_ptr<Label> mutatedLabel = nullptr;
  if (kind == ompt_dispatch_iteration) {
    RAW_DLOG(INFO, "on dispatch iter, parent label: %s", 
            parentLabel->toString().c_str());
    mutatedLabel = mutateIterDispatch(parentLabel, instance.value);
  } else if (kind == ompt_dispatch_section) {
    mutatedLabel = mutateSectionDispatch(parentLabel, instance.ptr);
  }
  taskDataPtr->label = std::move(mutatedLabel);
}

void on_ompt_callback_reduction(
       ompt_sync_region_t kind,
       ompt_scope_endpoint_t endPoint,
       ompt_data_t *parallelData,
       ompt_data_t *taskData,
       const void *codePtrRa) {
  if (!taskData || !taskData->ptr) {
    RAW_LOG(FATAL, "task data pointer is null");
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
