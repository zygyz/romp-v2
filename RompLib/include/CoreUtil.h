#pragma once
#include <utility>
#include <Symtab.h>

#include "DataSharing.h"
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
  CheckInfo(AllTaskInfo& allTaskInfo, 
            uint32_t bytesAccessed,
            void* instnAddr,
            void* taskPtr,
            int taskType,
            bool isWrite,
            bool hwLock,
            DataSharingType dataSharingType): 
                          allTaskInfo(std::move(allTaskInfo)), 
                          bytesAccessed(bytesAccessed),
                          instnAddr(instnAddr),
                          taskPtr(taskPtr),
                          taskType(taskType),
                          isWrite(isWrite),
                          hwLock(hwLock),
                          dataSharingType(dataSharingType){}
  AllTaskInfo allTaskInfo;
  uint32_t bytesAccessed;
  void* instnAddr;
  void* taskPtr;
  int taskType;
  bool isWrite;
  bool hwLock; 
  uint64_t byteAddress;
  DataSharingType dataSharingType;
} CheckInfo; 

bool prepareAllInfo(int& taskType, 
                    int& teamSize, 
                    int& threadNum, 
                    void*& curParRegionData,
                    void*& curThreadData,
                    AllTaskInfo& allTaskInfo);

void reportDataRaceWithLineInfo(void* instnAddrPrev, 
                    void* instnAddrCur, 
                    uint64_t address, 
                    Dyninst::SymtabAPI::Symtab* obj);

void reportDataRace(void* instnAddrPrev, void* instnAddrCur, uint64_t address);

}
