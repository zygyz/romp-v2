#include "AccessHistory.h"

#include <glog/logging.h>
#include <glog/raw_logging.h>

namespace romp {

void AccessHistory::_initRecords() {
  _records = std::make_unique<std::vector<Record>>();
}

std::mutex& AccessHistory::getMutex() {
  return _mutex;
}


RecordState AccessHistory::getRecordState() const {
  return _recordState;
}

void AccessHistory::setRecordState(RecordState state) {
  _recordState = state;
}
/*
 * If records pointer has not been initialized, initialize the record first.
 * Then return the raw pointer to the records vector.
 * We assume the access history is under mutual exclusion.
 */
std::vector<Record>* AccessHistory::getRecords() {
  if (!_records.get()) {
    _initRecords();
  }
  return _records.get();
}

void AccessHistory::setFlag(AccessHistoryFlag flag) {
  _state |= flag;
}

bool AccessHistory::dataRaceFound() const {
  return (_state & eDataRaceFound) != 0;
}

}
