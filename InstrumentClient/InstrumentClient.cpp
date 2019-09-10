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

vector<BPatch_function*>
InstrumentClient::getCheckAccessFuncs(
        unique_ptr<BPatch_addressSpace> addrSpacePtr) {
  if (addrSpacePtr == nullptr) {
    LOG(FATAL) << "null pointer";
  }
  auto appImage = addrSpacePtr->getImage();
  vector<BPatch_function*> checkAccessFuncs;
  appImage->findFunction("checkAccess", checkAccessFuncs);
  if (checkAccessFuncs.size() == 0) {
     LOG(FATAL) << "cannot find function `checkAccess` in romp lib";
  }
  return checkAccessFuncs;
}
