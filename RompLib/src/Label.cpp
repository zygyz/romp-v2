#include "Label.h"

#include <glog/logging.h>
#include <glog/raw_logging.h>

namespace romp {

Label::Label() { 
  RAW_LOG(INFO, "%s\n", "Label() constructor");
}

/* 
 * Use a shallow copy so that the new label does not create separate new 
 * segments. If later on some label segment changes, one should erase that 
 * label segment and create a new one otherwise the other label's content 
 * will be affected.
 */
Label::Label(const Label& label) {
  _label = label._label; 
}

Label::~Label() {
  RAW_LOG(INFO, "%s\n", "Label() destructor");
}

void Label::print() const {
  for (const auto& segment : _label) {
    segment->print();
  }
}

void Label::appendSegment(std::shared_ptr<Segment> segment) {
  _label.push_back(segment);
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

}
