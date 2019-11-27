#pragma once
#include <string>

namespace romp {

class LockSet {
public:
  virtual std::string toString() const = 0;
  virtual bool hasCommonLock(const LockSet& other) const = 0;
  virtual void addLock(uint64_t lock) = 0;
  virtual void removeLock(uint64_t lock) = 0;
  virtual void* getLocks() = 0;
  virtual uint16_t getNumLocks() const = 0; 
  virtual ~LockSet() = default;
};

/*
 * SmallLockSet class is for recording the set of locks held upon
 * a memory access. This class holds at most four nested locks
 */
class SmallLockSet : public LockSet {
public:
  SmallLockSet();
  SmallLockSet(const SmallLockSet& lockset);
  std::string toString() const override;
  bool hasCommonLock(const LockSet& other) const override;
  void addLock(uint64_t lock) override;
  void removeLock(uint64_t lock) override;
  void* getLocks() override; 
  uint16_t getNumLocks() const;
private:
  uint64_t _locks[4];
  uint16_t _numLocks;
};

bool isSubset(LockSet* me, LockSet* other);

}
