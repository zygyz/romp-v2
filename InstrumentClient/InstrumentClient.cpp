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
  if (!ptr->loadLibrary(rompLibPath.c_str())) {
    LOG(FATAL) << "cannot load romp library"; 
  } else {
    LOG(INFO) << "loaded romp library at: " << rompLibPath;
  }
  return ptr;
}

