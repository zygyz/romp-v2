#pragma once
#include <ompt.h>

/*
 * This header file defines a set of wrapper for ompt query functions.
 */
namespace romp {

extern ompt_get_task_info_t omptGetTaskInfo;
extern ompt_get_parallel_info_t omptGetParallelInfo;
extern ompt_get_thread_data_t omptGetThreadInfo;

enum OmptTaskQueryType { eTaskData, eTaskFrame, eParallelData };

bool infoIsAvailable(const int& retVal);

void* queryTaskInfo(
        const int& ancestorLevel, 
        const OmptTaskQueryType& queryType,
        int& taskType,
        int& threadNum); 

void* queryParallelInfo(
        const int& ancestorLevel,
        int& teamSize);
}
