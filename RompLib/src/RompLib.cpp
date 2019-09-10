#include "Initialize.h"


using namespace std;

namespace romp {

extern "C" {

void 
checkAccess(void* address,
            uint32_t bytesAccessed,
            uint64_t instnAddr,
            bool hwLock) {
   
 LOG(INFO) << "test check access call";

}

}

}

