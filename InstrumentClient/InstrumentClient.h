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
      void instrumentMemoryAccess();    
    private:
      std::unique_ptr<BPatch_addressSpace> initInstrumenter(
              const std::string& programName,
              const std::string& rompLibPath); 
      std::vector<BPatch_function*> getCheckAccessFuncs(
              std::unique_ptr<BPatch_addressSpace>& addrSpacePtr);
      void getFunctionsVector(
              std::unique_ptr<BPatch_addressSpace>& addrSpacePtr,
              std::vector<BPatch_function*>& funcVec);
    private:    
      std::unique_ptr<BPatch_addressSpace> addrSpacePtr_;
      std::shared_ptr<BPatch> bpatchPtr_;
      std::vector<BPatch_function*> checkAccessFuncs_;
  };
}
