#include "InstrumentClient.h"

#include <glog/logging.h>

using namespace Dyninst;
using namespace romp;
using namespace std;

InstrumentClient::InstrumentClient(
        const string& programName, 
        const string& rompLibPath,
        shared_ptr<BPatch> bpatchPtr) {
  bpatchPtr_ = move(bpatchPtr);
  addrSpacePtr_ = initInstrumenter(programName, rompLibPath);
  checkAccessFuncs_ = getCheckAccessFuncs(addrSpacePtr_);
  LOG(INFO) << "InstrumentClient initialized";
}

unique_ptr<BPatch_addressSpace> 
InstrumentClient::initInstrumenter(
        const string& programName,
        const string& rompLibPath) {
  auto handle = bpatchPtr_->openBinary(programName.c_str(), true);
  if (!handle) {
    LOG(FATAL) << "cannot open binary: " << programName;    
  }
  unique_ptr<BPatch_addressSpace> ptr(handle);  
  // load romp library 
  if (!ptr->loadLibrary(rompLibPath.c_str())) {
    LOG(FATAL) << "cannot load romp library"; 
  } else {
    LOG(INFO) << "loaded romp library at: " << rompLibPath;
  }
  return ptr;
}

/* 
 * Get the dyninst representation of the `checkAccess` function
 * defined in romp library code RompLib.cpp.
 */
vector<BPatch_function*>
InstrumentClient::getCheckAccessFuncs(
        unique_ptr<BPatch_addressSpace>& addrSpacePtr) {
  if (!addrSpacePtr) {
    LOG(FATAL) << "null pointer";
  }
  auto appImage = addrSpacePtr->getImage();
  if (!appImage) {
    LOG(FATAL) << "cannot get image";
  }
  vector<BPatch_function*> checkAccessFuncs;
  appImage->findFunction("checkAccess", checkAccessFuncs);
  if (checkAccessFuncs.size() == 0) {
     LOG(FATAL) << "cannot find function `checkAccess` in romp lib";
  }
  return checkAccessFuncs;
}

/* 
 * Get dyninst representation of all all functions in the 
 * program being instrumented. Ideally, no function should 
 * be skipped.
 */ 
vector<BPatch_function*> 
InstrumentClient::getFunctionsVector(
        unique_ptr<BPatch_addressSpace>& addrSpacePtr) {
  vector<BPatch_function*> funcVec;
  auto appImage = addrSpacePtr->getImage();
  if (!appImage) {
    LOG(FATAL) << "cannot get image";
  }
  auto appModules = appImage->getModules();
  if (!appModules) {
    LOG(FATAL) << "cannot get modules";
  }
  for (auto& module : *appModules) {
    auto procedures = module->getProcedures();
    for (auto& procedure : *procedures) {
        funcVec.push_back(procedure);
    }
  }
  return funcVec;
}

/* 
 * Public interface for InstrumentClient, wraps the internal 
 * implementation of instrumentation of memory accesses
 */
void
InstrumentClient::instrumentMemoryAccess() {  
  auto functions = getFunctionsVector(addrSpacePtr_);
  instrumentMemoryAccessInternal(addrSpacePtr_, functions);
}

/*
 * Instrument memory accesses in each function by inserting the 
 * `checkAccess` function call 
 */ 
void
InstrumentClient::instrumentMemoryAccessInternal(
    unique_ptr<BPatch_addressSpace>& addrSpacePtr,
    vector<BPatch_function*>& funcVec) {   
  BPatch_Set<BPatch_opCode> opcodes;
  opcodes.insert(BPatch_opLoad);
  opcodes.insert(BPatch_opStore);
  addrSpacePtr->beginInsertionSet();
  for (auto& function : funcVec) {
    auto pointsVecPtr = function->findPoint(opcodes);
    if (!pointsVecPtr) {
      LOG(WARNING) << "no load/store points for function " << function->getName();    
      continue;
    } else if (pointsVecPtr->size() == 0) {
      LOG(WARNING) << "load/store points vector size is 0 for function " << function->getName();
      continue;
    }
    //insertSnippet(addrSpacePtr, pointsVecPtr, function);
  }
}
