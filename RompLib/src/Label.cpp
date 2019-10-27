#include "Label.h"

#include <glog/logging.h>
#include <glog/raw_logging.h>

namespace romp {

/* 
 * Use a shallow copy so that the new label does not create separate new 
 * segments. If later on some label segment changes, one should erase that 
 * label segment and create a new one otherwise the other label's content 
 * will be affected.
 */
Label::Label(const Label& label) {
  _label = label._label; 
}

std::string Label::toString() const {
  auto result = std::string("");
  for (const auto& segment : _label) {
    result += segment->toString();
    result += std::string(" | ");
  }
  return result;
}

void Label::appendSegment(std::shared_ptr<Segment> segment) {
  _label.push_back(segment);
}

std::shared_ptr<Segment> Label::popSegment() {
  if (_label.empty()) {
    RAW_LOG(FATAL, "%s", "label is empty");
  }
  auto lastSegment = _label.back();
  _label.pop_back();
  return lastSegment;
}

std::shared_ptr<Segment> Label::getLastKthSegment(int k) {
  if (k > _label.size()) {
    RAW_LOG(FATAL, "%s", "index is out of bound");
    return nullptr;
  }
  auto len = _label.size();
  return _label.at(len - k);
}

void Label::setLastKthSegment(int k, std::shared_ptr<Segment> segment) { 
  if (k > _label.size()) {
    RAW_LOG(FATAL, "%s %d", "set value out of bound", k);
    return;
  }
  auto len = _label.size();
  _label[len - k] = segment;
}

std::shared_ptr<Label> genImpTaskLabel(
                           const std::shared_ptr<Label>& parentLabel,
                           unsigned int index,
                           unsigned int actualParallelism) {
  // create the new label by copy constructing from parent label
  auto newLabel = std::make_shared<Label>(*parentLabel.get());
  // create a new label segment
  auto newSegment = std::make_shared<BaseSegment>(eImplicit, 
          static_cast<uint64_t>(index), 
          static_cast<uint64_t>(actualParallelism));
  newLabel->appendSegment(newSegment);
  return newLabel;
}

std::shared_ptr<Label> mutateParentImpEnd(
        const std::shared_ptr<Label>& parentLabel,
        const std::shared_ptr<Label>& childLabel) {
  auto newLabel = std::make_shared<Label>(*parentLabel.get());
  newLabel->popSegment();       
  auto childSegment = childLabel->getLastKthSegment(2);
  newLabel->appendSegment(childSegment);
  return newLabel;
}

/*
 * Given the task label `label`, generate the mutated label for encounteing 
 * the barrier. This mutation is done by adding span to the offset field of 
 * the second last segment of the label.
 */
std::shared_ptr<Label> mutateBarrierEnd(const std::shared_ptr<Label>& label) {
  auto newLabel = std::make_shared<Label>(*label.get());
  auto segment = newLabel->getLastKthSegment(2); //get the second last segment
  uint64_t offset, span; 
  segment->getOffsetSpan(offset, span); //get the offset and span value
  offset += span;
  //because we don't know the actual derived type of segment, should do a clone
  auto newSegment = segment->clone(); 
  newSegment->setOffsetSpan(offset, span); //set the new offset and span
  newLabel->setLastKthSegment(2, newSegment); 
  return newLabel; 
} 

/*
 * Given the label `label`, create a new label which is a mutation for after 
 * encountering taskwait clause. This is done by incrementing the taskwait 
 * field counter in the last label segment
 */ 
std::shared_ptr<Label> mutateTaskWait(const std::shared_ptr<Label>& label) {
  auto newLabel = std::make_shared<Label>(*label.get());
  auto lastSegment = newLabel->popSegment(); // replace the last segment
  uint64_t taskwait;
  lastSegment->getTaskwait(taskwait);
  taskwait += 1;
  auto newSegment = lastSegment->clone();
  newSegment->setTaskwait(taskwait);
  newLabel->appendSegment(newSegment);
  return newLabel;
}

/*
 * Given the label `label`, create a new label which is a mutation for 
 * encountering begin of ordered section. This is done by incrementing
 * the `phase` counter value by one.
 */
std::shared_ptr<Label> mutateOrderAcquire(std::shared_ptr<Label>& label) {
  auto newLabel = std::make_shared<Label>(*label.get());
  auto lastSegment = newLabel->popSegment(); // replace the last segment
  uint64_t phase;
  lastSegment->getPhase(phase);
  phase += 1;
  auto newSegment = lastSegment->clone();
  newSegment->setPhase(phase);
  newLabel->appendSegment(newSegment);
  return newLabel;
}

}
