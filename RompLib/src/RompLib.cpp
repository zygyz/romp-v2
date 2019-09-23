#include "Initialize.h"
#include <iostream>

using namespace std;

namespace romp {




extern "C" {

/** 
 *  initialize OMPT interface by registering callback functions
 */
int omptInitialize(ompt_function_lookup_t functionLookup,
                   int initialDeviceNum,
                   ompt_data_t* toolData) {
  google::InitGoogleLogging("romp");
  LOG(INFO) << "start initializing ompt";      
  cout << "start initializing ompt" << endl;

}

/**
 *  release resources and log info upon finalization of tool
 */
void omptFinalize(ompt_data_t* toolData) {
  LOG(INFO) << "finalizing ompt";
  cout << "finalizing ompt" << endl;
}


/** 
 * implement ompt_start_tool which is defined in OpenMP spec 5.0
 */
ompt_start_tool_result_t* ompt_start_tool(
        unsigned int ompVersion,
        const char* runtimeVersion) {
  ompt_data_t data;
  static ompt_start_tool_result_t startToolResult = { 
      &omptInitialize, &omptFinalize, data}; 
  LOG(INFO) << "ompt_start_tool";
  cout << "ompt_start_tool" << endl;
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

