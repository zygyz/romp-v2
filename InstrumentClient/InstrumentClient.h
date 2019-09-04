#pragma once
#include "InstrumentClient.h"

#include <memory>
#include <string>

#include "BPatch.h"
#include "BPatch_addressSpace.h"
#include "BPatch_process.h"

namespace romp {
  class InstrumentClient {
    public:
      InstrumentClient(const std::string& programName, std::shared_ptr<BPatch> bpatchPtr);
    private:
      std::unique_ptr<BPatch_addressSpace> initInstrumenter(const std::string& programName); 
//      void instrumentMemoryAccess(const string& rompPath);
    private:    
      std::unique_ptr<BPatch_addressSpace> addrSpacePtr_;
      std::shared_ptr<BPatch> bpatchPtr_;
  };
}
