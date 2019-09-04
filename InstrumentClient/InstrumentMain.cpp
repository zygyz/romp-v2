#include <gflags/gflags.h>
#include <glog/logging.h>
#include "BPatch.h"
#include "BPatch_addressSpace.h"
#include "BPatch_process.h"
using namespace std;
using namespace Dyninst;

DEFINE_string(program, "", "program to be instrumented");

int main(int argc, char* argv[]) {
  BPatch_addressSpace* app;
  google::InitGoogleLogging(argv[0]);
  FLAGS_alsologtostderr = 1;
  LOG(INFO) << "TEST TEST ";
  return 0;
}
