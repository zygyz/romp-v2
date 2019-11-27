#include "LockSet.h"

#include <algorithm>
#include <glog/logging.h>
#include <glog/raw_logging.h>

namespace romp {

SmallLockSet::SmallLockSet() {
  for (int i = 0; i < 4; ++i) {
    _locks[i] = 0;
  }
  _numLocks = 0;
}

SmallLockSet::SmallLockSet(const SmallLockSet& lockset) {
  for (int i = 0; i < lockset._numLocks; ++i) {
    _locks[i] = lockset._locks[i];
  }
  _numLocks = lockset._numLocks;
}

std::string SmallLockSet::toString() const {
  std::stringstream stream;
  for (int i = 0; i < _numLocks; ++i) {
    stream << std::hex << _locks[i] << "|";
  }
  auto result = "<" + stream.str() + ">";
  return result;
}

void SmallLockSet::addLock(uint64_t lock) {
  if (_numLocks == 4) {
    RAW_LOG(FATAL, "number of nesting locks exceeds limit of 4");
  }  
  _locks[_numLocks] = lock; 
  _numLocks++;
}

/*
 * Compute the intersect of two set of locks
 * Return true if two set of locks have common lock
 * Return false otherwise
 */
bool SmallLockSet::hasCommonLock(const LockSet& other) const {
  auto otherLockSet = dynamic_cast<const SmallLockSet&>(other);
  auto otherNumLocks = otherLockSet._numLocks;
  for (int i = 0; i < _numLocks; ++i) {
    auto lock = _locks[i];
    for (int j = 0; j < otherNumLocks; ++j) {
      auto otherLock = otherLockSet._locks[j]; 
      if (otherLock == lock) {
        return true;
      }
    } 
  }
  return false;
}

void* SmallLockSet::getLocks() {
  return static_cast<void*>(_locks);
}
/*
 * Return true if current lockset is the subset of `other` lockset
 */
bool SmallLockSet::isSubsetOf(const LockSet& other) const {
  auto otherLockSet = dynamic_cast<const SmallLockSet&>(other);
  auto otherNumLocks = otherLockSet._numLocks;
  auto otherLocks = otherLockSet.getLocks();
  if (otherNumLocks < _numLocks) {
    return false;
  }
  for (int i = 0; i < _numLocks; ++i) {
    auto lock = _locks[i];
    for (int j = 0; j < otherNumLocks; ++j) {
      auto otherLock = static_cast<uint64_t*>(otherLocks)[j];
      if (otherLock != lock) {
        return false; 
      }
    } 
  }
  return true;
}

void SmallLockSet::removeLock(uint64_t lock) {
  auto index = -1;
  for (int i = 0; i < _numLocks; ++i) {
    if (_locks[i] == lock) {
      index = i; 
      break;
    }
  }
  if (index == -1) {
    RAW_LOG(FATAL, "cannot find lock to delete: %lu", lock);
  }
  for (int i = index + 1; i < _numLocks; ++i) {
    _locks[i - 1] = _locks[i];
  }
  _numLocks--;
}

}
