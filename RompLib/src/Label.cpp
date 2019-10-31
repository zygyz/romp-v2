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

void Label::appendSegment(const std::shared_ptr<Segment>& segment) {
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

void Label::setLastKthSegment(int k, const std::shared_ptr<Segment>& segment) { 
  if (k > _label.size()) {
    RAW_LOG(FATAL, "%s %d", "set value out of bound", k);
    return;
  }
  auto len = _label.size();
  _label[len - k] = std::move(segment);
}

std::shared_ptr<Label> genImpTaskLabel(
                           Label* parentLabel,
                           unsigned int index,
                           unsigned int actualParallelism) {
  // create the new label by copy constructing from parent label
  auto newLabel = std::make_shared<Label>(*parentLabel);
  // create a new label segment
  auto newSegment = std::make_shared<BaseSegment>(eImplicit, 
          static_cast<uint64_t>(index), 
          static_cast<uint64_t>(actualParallelism));
  newLabel->appendSegment(newSegment);
  return newLabel;
}

std::shared_ptr<Label> genInitTaskLabel() {
  auto label = std::make_shared<Label>();
  auto segment = std::make_shared<BaseSegment>(eImplicit, 0, 1);
  label->appendSegment(segment);
  return label;
}

/*
 * Given the parent task label, generate the label for the explicit task.
 */
std::shared_ptr<Label> genExpTaskLabel(Label* parentLabel) {
  auto newLabel = std::make_shared<Label>(*parentLabel);
  auto segment = std::make_shared<BaseSegment>(eExplicit, 0, 1); 
  newLabel->appendSegment(segment);
  return newLabel;
}

std::shared_ptr<Label> mutateParentImpEnd(
        Label* parentLabel,
        Label* childLabel) {
  auto newLabel = std::make_shared<Label>(*parentLabel);
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
std::shared_ptr<Label> mutateBarrierEnd(Label* label) {
  auto newLabel = std::make_shared<Label>(*label);
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
std::shared_ptr<Label> mutateTaskWait(Label* label) {
  auto newLabel = std::make_shared<Label>(*label);
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
 * encountering begin/endof ordered section. This is done by incrementing
 * the `phase` counter value by one.
 */
std::shared_ptr<Label> mutateOrderSection(Label* label) {
  auto newLabel = std::make_shared<Label>(*label);
  auto lastSegment = newLabel->popSegment(); // replace the last segment
  uint64_t phase;
  lastSegment->getPhase(phase);
  phase += 1;
  auto newSegment = lastSegment->clone();
  newSegment->setPhase(phase);
  newLabel->appendSegment(newSegment);
  return newLabel;
}

/*
 * Mutate the label when workshare loop begin. Append a place holder segment
 * to mark the begin of the workshare loop.
 */
std::shared_ptr<Label> mutateLoopBegin(Label* label) {
  auto newLabel = std::make_shared<Label>(*label); 
  auto newSegment = std::make_shared<WorkShareSegment>(); 
  newSegment->setPlaceHolderFlag(true);
  newLabel->appendSegment(newSegment);   
  return newLabel;
}

/*
 * Mutate the label when workshare loop ends. Pop the last segment which should
 * be a workshare segment. Increment the loop count of the current last 
 * segment by one (should replace the old one)
 */
std::shared_ptr<Label> mutateLoopEnd(Label* label) {
  auto newLabel = std::make_shared<Label>(*label); 
  newLabel->popSegment();
  uint64_t loopCount = 0;
  auto segment = newLabel->popSegment();
  segment->getLoopCount(loopCount);
  loopCount += 1;
  auto newSegment = segment->clone(); 
  newSegment->setLoopCount(loopCount);
  newLabel->appendSegment(newSegment);
  return newLabel;
}

/*
 * Sections construct is implemented as a dynamically scheduled workshare loop.
 * So we treat the sections as a workshared loop 
 */
std::shared_ptr<Label> mutateSectionBegin(Label* label) { 
  return mutateLoopBegin(label);
}

std::shared_ptr<Label> mutateSectionEnd(Label* label) {
  return mutateLoopEnd(label);
}

/*
 * Mutate the label upon entering the single contruct and the task is the 
 * executor. Append a workshare segment to the current label and set the 
 * single executor bit.
 */
std::shared_ptr<Label> mutateSingleExecBegin(Label* label) {
  auto newLabel = std::make_shared<Label>(*label); 
  auto newSegment = std::make_shared<WorkShareSegment>(); 
  newSegment->setSingleFlag(true);
  newLabel->appendSegment(newSegment);   
  return newLabel;
}

/*
 * Mutate the label upon exiting the single construct and the task is the 
 * executor. Pop the workshare segment.
 */
std::shared_ptr<Label> mutateSingleEnd(Label* label) {
  auto newLabel = std::make_shared<Label>(*label);
  newLabel->popSegment();
  return newLabel;
}

/*
 * Mutate the label upon entering the single construct and the task is other. 
 * If nowait is specified, it does not wait for the single executor to finish.
 * If no nowait, these 'other' tasks wait on an implicit barrier for the single
 * executor to finish. Here we append a workshare segment and mark the single
 * other bit.
 */
std::shared_ptr<Label> mutateSingleOtherBegin(Label* label) {
  auto newLabel = std::make_shared<Label>(*label); 
  auto newSegment = std::make_shared<WorkShareSegment>(); 
  newSegment->setSingleFlag(false);
  newLabel->appendSegment(newSegment);   
  return newLabel;
}

/*
 * Mutate the label upon entering the taskloop construct. If nogroup is not 
 * specified, an implicit taskgroup is generated. If nogroup is specified,
 * no implicit taskgroup is generated. This callback happens before any actual
 * execution of logical iteration and creation of explicit tasks
 */
std::shared_ptr<Label> mutateTaskLoopBegin(Label* label) {
  return nullptr;
}

std::shared_ptr<Label> mutateTaskLoopEnd(Label* label) {
  return nullptr;
}
   


}
