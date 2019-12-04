#include <filesystem>
#include <glog/logging.h>
#include <glog/raw_logging.h>
#include <limits.h>
#include <Symtab.h>
#include <unistd.h>

#include "AccessHistory.h"
#include "Core.h"
#include "CoreUtil.h"
#include "DataSharing.h"
#include "Initialize.h"
#include "Label.h"
#include "LockSet.h"
#include "ShadowMemory.h"
#include "TaskData.h"
#include "ThreadData.h"

namespace fs = std::filesystem;
using namespace Dyninst;
using namespace SymtabAPI;

namespace romp {

using LabelPtr = std::shared_ptr<Label>;
using LockSetPtr = std::shared_ptr<LockSet>;

ShadowMemory<AccessHistory> shadowMemory;
Symtab* obj = nullptr;

/*
 * Driver function to do data race checking and access history management.
 */
void checkDataRace(AccessHistory* accessHistory, const LabelPtr& curLabel, 
                   const LockSetPtr& curLockSet, const CheckInfo& checkInfo) {
  std::unique_lock<std::mutex> guard(accessHistory->getMutex());
  auto records = accessHistory->getRecords();
  if (accessHistory->dataRaceFound()) {
    /* data race has already been found on this memory location, romp only 
     * reports one data race on any memory location in one run. Once the data 
     * race is reported, romp clears the access history with respect to this
     * memory location and mark this memory location as found. Future access 
     * to this memory location does not go through data race checking.
     * TODO: implement the logic described above.
     */
    auto instnAddr = reinterpret_cast<uint64_t>(checkInfo.instnAddr);
    std::vector<LineNoTuple> lines;
    obj->getSourceLines(lines, instnAddr);
    if (lines.empty()) {
      RAW_LOG(WARNING, "cannot get source lines info for address %lx", instnAddr); 
    } else {
      auto fileName = lines[0].getFile();
      auto line = lines[0].getLine();
      auto column = lines[0].getColumn();
      RAW_LOG(INFO, "data race found: %s:%d, %d@%lx", 
            fileName.c_str(), line, column, instnAddr);
    }
    //lineInfoReader->lookup(instnAddr, line, column, fileName);
    if (!records->empty()) {
      records->clear();
    }
    return;
  }
  //RAW_LOG(INFO, "access record length: %d", records->size());
  auto curRecord = Record(checkInfo.isWrite, curLabel, curLockSet, 
          checkInfo.taskPtr, checkInfo.instnAddr);
  if (records->empty()) {
    // no access record, add current access to the record
    //RAW_DLOG(INFO, "records list is empty, add record");
    records->push_back(curRecord);
  } else {
    // check previous access records with current access
    auto isHistBeforeCurrent = false;
    auto it = records->begin();
    std::vector<Record>::const_iterator cit;
    auto skipAddCur = false;
    int diffIndex;
    while (it != records->end()) {
      cit = it; 
      auto histRecord = *cit;
      if (analyzeRaceCondition(histRecord, curRecord, isHistBeforeCurrent, 
                  diffIndex)) {
        // TODO: report line info
        gDataRaceFound = true;
        accessHistory->setFlag(eDataRaceFound);  
      }
      auto decision = manageAccessRecord(histRecord, curRecord, 
              isHistBeforeCurrent, diffIndex);
      if (decision == eSkipAddCur) {
        skipAddCur = true;
      }
      modifyAccessHistory(decision, records, it);
    }
    if (!skipAddCur) {
      records->push_back(curRecord); 
    }
  }
}

extern "C" {

/** 
 * implement ompt_start_tool which is defined in OpenMP spec 5.0
 */
ompt_start_tool_result_t* ompt_start_tool(
        unsigned int ompVersion,
        const char* runtimeVersion) {
  ompt_data_t data;
  static ompt_start_tool_result_t startToolResult = { 
      &omptInitialize, &omptFinalize, data}; 
  char result[PATH_MAX];
  auto count = readlink("/proc/self/exe", result, PATH_MAX);
  if (count == 0) {
    LOG(FATAL) << "cannot get current executable path";
  }
  auto curAppPath = std::string(result, count);
  LOG(INFO) << "ompt_start_tool on executable: " << curAppPath;
  auto success = Symtab::openFile(obj, curAppPath);
  if (!success) {
    LOG(FATAL) << "cannot parse executable into symtab: " << curAppPath;
  }
  return &startToolResult;
}

void checkAccess(void* address,
                 uint32_t bytesAccessed,
                 void* instnAddr,
                 bool hwLock,
                 bool isWrite) {
  /*
  RAW_LOG(INFO, "address:%lx bytesAccessed:%u instnAddr: %lx hwLock: %u,"
                "isWrite: %u", address, bytesAccessed, instnAddr, 
                 hwLock, isWrite);
                 */
  if (!gOmptInitialized) {
    //RAW_LOG(INFO, "ompt not initialized yet");
    return;
  }
  AllTaskInfo allTaskInfo;
  int threadNum = -1;
  int taskType = -1;
  int teamSize = -1;
  void* curThreadData = nullptr;
  void* curParRegionData = nullptr;
  if (!prepareAllInfo(taskType, teamSize, threadNum, curParRegionData, 
              curThreadData, allTaskInfo)) {
    return;
  }
  if (taskType == ompt_task_initial) { 
    // don't check data race for initial task
    return;
  }
  // query data  
  auto dataSharingType = analyzeDataSharing(curThreadData, address, 
                                           allTaskInfo.taskFrame);
  if (!allTaskInfo.taskData->ptr) {
    RAW_LOG(WARNING, "pointer to current task data is null");
    return;
  }
  auto curTaskData = static_cast<TaskData*>(allTaskInfo.taskData->ptr);
  auto& curLabel = curTaskData->label;
  auto& curLockSet = curTaskData->lockSet;
  
  CheckInfo checkInfo(allTaskInfo, bytesAccessed, instnAddr, 
          static_cast<void*>(curTaskData), taskType, isWrite, hwLock, 
          dataSharingType);
  for (uint64_t i = 0; i < bytesAccessed; ++i) {
    auto curAddress = reinterpret_cast<uint64_t>(address) + i;      
    auto accessHistory = shadowMemory.getShadowMemorySlot(curAddress);
    checkDataRace(accessHistory, curLabel, curLockSet, checkInfo);
  }
}

}

}

