#include "LockSet.h"

#include <glog/logging.h>
#include <glog/raw_logging.h>

namespace romp {

LockSet::LockSet() {
  RAW_LOG(INFO, "LockSet() constructor");
}

LockSet::~LockSet() {
  RAW_LOG(INFO, "LockSet() destructor");
}


}
