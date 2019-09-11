#include "InstrumentClient.h"

#include <glog/logging.h>

using namespace Dyninst;
using namespace romp;
using namespace std;

InstrumentClient::InstrumentClient(
        const string& programName, 
        const string& rompLibPath,
        shared_ptr<BPatch> bpatchPtr,
        const string& arch) {
  bpatchPtr_ = move(bpatchPtr);
  arch_ = arch;
  addrSpacePtr_ = initInstrumenter(programName, rompLibPath);
  checkAccessFuncs_ = getCheckAccessFuncs(addrSpacePtr_);
  LOG(INFO) << "InstrumentClient initialized with arch: " << arch_;
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
      LOG(WARNING) << "no load/store points for function " 
          << function->getName();    
      continue;
    } else if (pointsVecPtr->size() == 0) {
      LOG(WARNING) << "load/store points vector size is 0 for function " 
          << function->getName();
      continue;
    }
    insertSnippet(addrSpacePtr, pointsVecPtr, function);
  }
}

/* 
 * Determine if the instruction contains a hardware lock for X86 architecture.
 * Could be modified for other architeture.
 */
bool
InstrumentClient::hasHardWareLock(
        const InstructionAPI::Instruction& instruction,
        const std::string& arch) {
  if (arch == "x86") { 
      // check first byte of the instruction for x86 arch
    auto firstByte = instruction.rawByte(0);
    return firstByte == 0xf0;
  } 
  LOG(FATAL) << "unexpected architecture: " << arch;
  return false;
}

/*
 * Insert checkAccess code snippet to load/store point
 */
void
InstrumentClient::insertSnippet(
        unique_ptr<BPatch_addressSpace>& addrSpacePtr,
        vector<BPatch_point*>* pointsVecPtr,
        BPatch_function* function) {
  if (!pointsVecPtr) {
    LOG(FATAL) << "null pointer";
  } 
  for (auto& point : *pointsVecPtr) {
    auto memoryAccess = point->getMemoryAccess();
    if (!memoryAccess) {
      LOG(FATAL) << "null memory access";
    }
    
    if (memoryAccess->isAPrefetch_NP()) {
      LOG(INFO) << "current point is a prefetch, continue";
      continue;
    }  
    auto isWrite = true;
    // sometimes an access is both a read and a write 
    // for this case, treat it as a write
    if (memoryAccess->isAStore()) { // is a store
      isWrite = true; 
    } else if (memoryAccess->isALoad()) { // is a pure load
      isWrite = false;
    } else {
      LOG(WARNING) << "unknown memory access type in function: " 
                   << point->getCalledFunctionName();
                    << " continue"; 
      continue;
    }
    auto instructionAddress = point->getAddress();         
    auto instruction = point->getInsnAtPoint();
    auto hardWareLock = hasHardwareLock(instruction, arch_);

    vector<BPatch_snippet*> funcArgs;
    // memory address 
    funcArgs.push_back(new BPatch_effectiveAddressExpr()); 
    // number of bytes accessed
    funcArgs.push_back(new BPatch_bytesAccessedExpr());    
    // address of instruction
    funcArgs.push_back(new BPatch_constExpr(instructionAddress)); 
    // instruction contains hardware lock or not
    funcArgs.push_back(new BPatch_constExpr(hardWareLock));
    // is write access or not
    funcArgs.push_back(new BPatch_constExpr(isWrite));
    BPatch_funcCallExpr checkAccessCall(*(checkAccessFuncs_[0]), funcArgs);
    if (!addrSpacePtr->insertSnippet(
                checkAccessCall, *point, BPatch_callBefore)) {
        LOG(FATAL) << "snippet insertion failed";
    }
  }
  LOG(INFO) << "check access snippet insertion is a success";
}
