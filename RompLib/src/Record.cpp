#include "Record.h"

namespace romp {
  
/*
 * If current access is write, set the lowest bit to 1. Otherwise, set to 0.
 * _state variable is 8-bit wide.
 */
void Record::setAccessType(bool isWrite) {
  if (isWrite) {
    _state |= 0x1;
  } else {
    _state &= 0xfe; 
  }
}

bool Record::isWrite() const {
  return (_state & 0x1) == 0x1;
}

/*
 * toString() is mainly for debugging
 */
std::string Record::toString() const {
  std::string result = "";
  auto labelStr = _label? _label->toString() : std::string("[empty label]");
  result += std::string("Label:") + labelStr;
  result += isWrite()? std::string("@write") : std::string("@read");
  return result;
}

}
