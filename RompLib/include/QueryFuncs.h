#pragma once
#include <ompt.h>

/*
 * This header file defines a set of wrapper for ompt query functions.
 */
namespace romp {

ompt_get_task_info_t omptGetTaskInfo;

enum OmptTaskQueryType { eTaskData, eTaskFrame, eParallelData };

bool isAvailable(const int& retVal);

void* omptGetTaskInfo(
        const int& ancestorLevel, 
        const OmptTaskQueryType& queryType,
        int& taskType,
        int& threadNum); 

}
