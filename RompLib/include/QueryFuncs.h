#pragma once
#include <cstddef>
#include <ompt.h>

/*
 * This header file defines a set of wrapper for ompt query functions.
 */
namespace romp {

extern ompt_get_task_info_t omptGetTaskInfo;
extern ompt_get_parallel_info_t omptGetParallelInfo;
extern ompt_get_thread_data_t omptGetThreadData;
extern ompt_get_task_memory_t omptGetTaskMemory;

typedef struct AllTaskInfo {
  ompt_data_t* taskData;
  ompt_frame_t* taskFrame;
  ompt_data_t* parallelData;
} AllTaskInfo;

enum OmptTaskQueryType { eTaskData, eTaskFrame, eParallelData };

bool infoIsAvailable(const int retVal);

bool queryAllTaskInfo( 
         const int ancestorLevel,
         int& taskType,
         int& threadNum,
         AllTaskInfo& allTaskInfo);

bool queryTaskInfo(
        const int ancestorLevel, 
        const OmptTaskQueryType& queryType,
        int& taskType,
        int& threadNum,
        void*& dataPtr); 

bool queryParallelInfo(
        const int ancestorLevel,
        int& teamSize,
        void*& dataPtr);

bool queryOmpThreadInfo(void*& dataPtr);

bool queryThreadStackInfo(void*& stackAddr, size_t& stackSize);

bool queryTaskMemoryInfo(void** addr, size_t* size);

}
