#pragma once
#include "Label.h"

namespace romp {

/*
 * `Record` class stores a sync info associated with a single memory access.
 */
class Record {
  
public:
  Record() { _state = 0; _label = nullptr; _taskPtr = nullptr;} 
  void setAccessType(bool isWrite);
  bool isWrite() const;
private:
  uint8_t _state; // store state information
  std::shared_ptr<Label> _label; // task label associated with the record
  void* _taskPtr; // pointer to data of encountering task
};


}

