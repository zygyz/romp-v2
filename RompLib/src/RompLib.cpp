#include "Initialize.h"

using namespace std;

namespace romp {


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
  //LOG(INFO) << "ompt_start_tool";
  std::cout << "ompt_start_tool called\n";
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

