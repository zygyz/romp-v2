#include "InstrumentClient.h"
#include <glog/logging.h>



using namespace Dyninst;
using namespace romp;
using namespace std;

InstrumentClient::InstrumentClient(const string& programName, shared_ptr<BPatch> bpatchPtr) {
 LOG(INFO) << "constructor of InstrumentClient";
  bpatchPtr_ = move(bpatchPtr);
  addrSpacePtr_ = initInstrumenter(programName);
  LOG(INFO) << "init instrumentation success";
}

unique_ptr<BPatch_addressSpace> 
InstrumentClient::initInstrumenter(const string& programName) {
  LOG(INFO) << "start init instrumentation: " << programName.c_str();
  auto handle = bpatchPtr_->openBinary(programName.c_str(), true);
  if (!handle) {
    LOG(FATAL) << "cannot open binary: " << programName;    
  }
  unique_ptr<BPatch_addressSpace> ptr(handle);  
  return ptr;
}
