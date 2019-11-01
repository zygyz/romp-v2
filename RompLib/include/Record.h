#pragma once
#include "Label.h"


namespace romp {

/*
 * `Record` class stores a sync info associated with a single memory access.
 */
class Record {
  
public:
  Record(): _state(0), _label(nullptr), _taskPtr(nullptr), _instnAddr(0){}
  Record(bool isWrite, 
         std::shared_ptr<Label> label, 
         void* taskPtr, 
         uint64_t instnAddr): 
      _label(label), _taskPtr(taskPtr), _instnAddr(instnAddr) { 
        setAccessType(isWrite); 
      }
  void setAccessType(bool isWrite);
  bool isWrite() const;
  std::string toString() const;
private:
  uint8_t _state; // store state information
  std::shared_ptr<Label> _label; // task label associated with the record
  void* _taskPtr; // pointer to data of encountering task
  uint64_t _instnAddr;
};


}

