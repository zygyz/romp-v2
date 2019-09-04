#include "InstrumentClient.h"
#include <glog/logging.h>

using namespace Dyninst;
using namespace romp;
using namespace std;

InstrumentClient::InstrumentClient(const string& programName, shared_ptr<BPatch> bpatchPtr) {
  bpatchPtr_ = move(bpatchPtr);
  addrSpacePtr_ = initInstrumenter(programName);
  LOG(INFO) << "InstrumentClient initialized";
}

unique_ptr<BPatch_addressSpace> 
InstrumentClient::initInstrumenter(const string& programName) {
  auto handle = bpatchPtr_->openBinary(programName.c_str(), true);
  if (!handle) {
    LOG(FATAL) << "cannot open binary: " << programName;    
  }
  unique_ptr<BPatch_addressSpace> ptr(handle);  
  return ptr;
}

void 
InstrumentClient::instrumentMemoryAccess(const string& rompPath) {

}
