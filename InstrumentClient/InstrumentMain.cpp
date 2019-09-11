#include <gflags/gflags.h>
#include <glog/logging.h>

#include "InstrumentClient.h"

using namespace Dyninst;
using namespace romp;
using namespace std;

DEFINE_string(program, "", "program to be instrumented");
DEFINE_string(rompPath, "", "path to romp library");
DEFINE_string(arch, "x86", "arch of the binary to be instrumented");
DEFINE_string(modSuffix, ".inst", "suffix for name of instrumented binary");

int main(int argc, char* argv[]) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  FLAGS_alsologtostderr = 1;
  if (FLAGS_program == "") {
    LOG(FATAL) << "no program name specified";
  } 
  if (FLAGS_rompPath == "") {
    LOG(FATAL) << "path to romp library is not specified";
  }
  auto bpatchPtr = make_shared<BPatch>(); 
  unique_ptr<InstrumentClient> client(
     new InstrumentClient(FLAGS_program, 
                          FLAGS_rompPath, 
                          bpatchPtr, 
                          FLAGS_arch,
                          FLAGS_modSuffix));
  LOG(INFO) << "TEST TEST ";
  return 0;
}
