#pragma once
#include <memory>
#include <vector>

namespace romp {
class Label;
class SmallLockSet;

/*
 * TaskData struct records information related to a task.
 * A pointer to this struct is stored in openmp runtime 
 * data structure and could be retrieved through ompt query 
 * functions.
 */
typedef struct TaskData {
  std::shared_ptr<Label> label;
  std::shared_ptr<SmallLockSet> lockSet;
  bool inReduction;
  std::vector<void*> childExpTaskData;
  TaskData() {
    label = nullptr;
    lockSet = nullptr;
    inReduction = false;
  }
} TaskData;

}
