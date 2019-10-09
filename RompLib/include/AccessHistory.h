#pragma once

namespace romp {

class AccessHistory {

public: 
  AccessHistory() : state(0) {}
  void setState(const uint8_t& state);
private:
  uint8_t state;  
};

}
