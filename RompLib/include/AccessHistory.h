#pragma once
#include <cstdint>
#include <memory>
#include <vector>

#include "Record.h"

namespace romp {

class AccessHistory {

public: 
  AccessHistory() : state(0) {  
    _records.reset(new std::vector<Record>()); // be c++11 compatible 
  }
  void setState(uint8_t state);
private:
  uint8_t state;  
  std::unique_ptr<std::vector<Record>> _records; 

};

}
