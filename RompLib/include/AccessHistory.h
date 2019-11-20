#pragma once
#include <cstdint>
#include <memory>
#include <mutex>
#include <vector>

#include "Record.h"
#define DEBUG_ACCESS_HISTORY
#define STATE_MASK 0x6  

namespace romp {

enum RecordState {
//TODO: optimize this variable out after prototyping
  eSibling,
  eNonSibling,
  eInit, 
  eSingle,
};

enum AccessHistoryFlag {
  eDataRaceFound = 0x1,
};

class AccessHistory {

public: 
  AccessHistory() : _state(0) { _recordState = eInit; }
  std::mutex& getMutex();
  std::vector<Record>* getRecords();
  void setFlag(AccessHistoryFlag flag);
  bool dataRaceFound() const;
  void setRecordState(RecordState state);
  RecordState getRecordState() const;
private:
  void _initRecords();
private:
  std::mutex _mutex; // use simple mutex, avoid premature optimization
  uint8_t _state;  
  std::unique_ptr<std::vector<Record>> _records; 
  RecordState _recordState; 
};

}
