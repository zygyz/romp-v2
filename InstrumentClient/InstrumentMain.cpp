#include <gflags/gflags.h>
#include <glog/logging.h>
#include <memory>

#include "BPatch.h"
#include "BPatch_addressSpace.h"
#include "BPatch_process.h"

#include "InstrumentClient.h"

using namespace Dyninst;
using namespace romp;
using namespace std;

DEFINE_string(program, "", "program to be instrumented");

int main(int argc, char* argv[]) {
  google::InitGoogleLogging(argv[0]);
  FLAGS_alsologtostderr = 1;
  if (FLAGS_program == "") {
    LOG(FATAL) << "no program name specified";
  } 
  auto bpatchPtr = make_shared<BPatch>(); 
  unique_ptr<InstrumentClient> client(new InstrumentClient(FLAGS_program, bpatchPtr));
  LOG(INFO) << "TEST TEST ";
  return 0;
}
