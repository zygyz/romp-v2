//#include "Initialize.h"

#include <glog/logging.h>
#include <ompt.h>


using namespace std;

namespace romp {



extern "C" {

int omptInitialize(ompt_function_lookup_t functionLookup,
                   int initialDeviceNum,
                   ompt_data_t* toolData) {
  google::InitGoogleLogging("romp");
  LOG(INFO) << "start initializing ompt";      

}

void omptFinalize(ompt_data_t* toolData) {
  LOG(INFO) << "finalizing ompt";
}

ompt_start_tool_result_t* ompt_start_tool(
        unsigned int ompVersion,
        const char* runtimeVersion) {
  ompt_data_t data;
  static ompt_start_tool_result_t startToolResult = { 
      &omptInitialize, &omptFinalize, data}; 
  LOG(INFO) << "ompt_start_tool";
  return &startToolResult;
}

void 
checkAccess(void* address,
            uint32_t bytesAccessed,
            uint64_t instnAddr,
            bool hwLock,
            bool isWrite) {
   
    /*
 LOG(INFO) << "address: " << address 
           << " bytesAccessed: " << bytesAccessed 
           << " instnAddr: " << instnAddr
           << " hwlock: " << hwLock
           << " isWrite: " << isWrite;
           */

}

}

}

