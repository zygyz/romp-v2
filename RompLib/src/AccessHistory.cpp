#include "AccessHistory.h"

#include <glog/logging.h>
#include <glog/raw_logging.h>

namespace romp {

void AccessHistory::setState(uint8_t state) {
  this->state = state;
}

std::mutex& AccessHistory::getMutex() {
  return _mutex;
}

std::vector<Record>* AccessHistory::getRecords() {
  return _records.get();
}

}
