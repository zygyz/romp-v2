#include <folly/logging/xlog.h>
#include "BPatch.h"
#include "BPatch_addressSpace.h"
#include "BPatch_process.h"
using namespace std;
using namespace Dyninst;

int main(int argc, const char* argv[]) {
  BPatch_addressSpace* app;
  //google::InitGoogleLogging(argv[0]);
  //LOG(INFO) << "TEST TEST ";
  XLOG(INFO) << "test test ";
  return 0;
}
