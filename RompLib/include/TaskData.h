#pragma once
#include <memory>

namespace romp {
class Label;

typedef struct TaskData {
  std::shared_ptr<Label> label;
} TaskData;

}
