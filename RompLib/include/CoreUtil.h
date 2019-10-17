#pragma once
#include "QueryFuncs.h"
/*
 * This header files defines a set of functions used in romp that are not 
 * closely related to the core data race detection algorithm. For example, 
 * we declare a function that is dedicated to calling ompt query wrapper 
 * functions to setup useful information on every call to the instrumented 
 * `checkAccess` function. 
 * The goal is to help cleaning up the core checking function code and to
 * ease the maintenance effort.
 */

namespace romp {

/* 
 * Wrap all necessary information for data race checking.
 */
typedef struct CheckInfo {
  AllTaskInfo allTaskInfo;
  uint32_t bytesAccessed;
  uint64_t instnAddr;
  int taskType;
  bool hwLock; 
  
} CheckInfo; 

bool prepareAllInfo(int& taskType, 
                    int& teamSize, 
                    int& threadNum, 
                    void*& curParRegionData,
                    void*& curThreadData,
                    AllTaskInfo& allTaskInfo);

}
