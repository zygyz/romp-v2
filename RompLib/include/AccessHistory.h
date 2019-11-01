#pragma once
#include <cstdint>
#include <vector>

#include "AccessRecord.h"

namespace romp {

class AccessHistory {

public: 
  AccessHistory() : state(0) {}
  void setState(uint8_t state);
private:
  uint8_t state;  
};

}
