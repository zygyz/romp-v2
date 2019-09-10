#pragma once
#include "InstrumentClient.h"

#include <memory>
#include <string>
#include <vector>

#include "BPatch.h"
#include "BPatch_addressSpace.h"
#include "BPatch_process.h"

namespace romp {
  class InstrumentClient {
    public:
      InstrumentClient(
              const std::string& programName, 
              const std::string& rompLibPath,
              std::shared_ptr<BPatch> bpatchPtr);
    private:
      std::unique_ptr<BPatch_addressSpace> initInstrumenter(
              const std::string& programName,
              const std::string& rompLibPath); 
//      void instrumentMemoryAccess(const string& rompPath);    
      std::vector<BPatch_function*> getCheckAccessFuncs(
              std::unique_ptr<BPatch_addressSpace>& addrSpacePtr);
    private:    
      std::unique_ptr<BPatch_addressSpace> addrSpacePtr_;
      std::shared_ptr<BPatch> bpatchPtr_;
      std::vector<BPatch_function*> checkAccessFuncs_;
  };
}
