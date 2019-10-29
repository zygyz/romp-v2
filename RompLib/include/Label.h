#pragma once
#include <memory>
#include <vector>
#include "Segment.h"

namespace romp {

/*
 * Label class implements the high level representation of task label.
 * A task label consists of a series of label segments. Each label segment is 
 * represented by a derived class from Segment. 
 */
class Label {

public:
  Label() {}
  Label(const Label& label);
  ~Label() {} 
  std::string toString() const;
  void appendSegment(const std::shared_ptr<Segment>& segment);
  std::shared_ptr<Segment> popSegment();
  std::shared_ptr<Segment> getLastKthSegment(int k);
  void setLastKthSegment(int k, const std::shared_ptr<Segment>& segment);
private:
  std::vector<std::shared_ptr<Segment> > _label;

};

std::shared_ptr<Label> genImpTaskLabel(
                          const std::shared_ptr<Label>& parentLabel, 
                          unsigned int index,
                          unsigned int actualParallelism);

std::shared_ptr<Label> mutateParentImpEnd(
        const std::shared_ptr<Label>& parentLabel,
        const std::shared_ptr<Label>& childLabel);

std::shared_ptr<Label> mutateBarrierEnd(const std::shared_ptr<Label>& l);
std::shared_ptr<Label> mutateTaskWait(const std::shared_ptr<Label>& l);
std::shared_ptr<Label> mutateOrderSection(const std::shared_ptr<Label>& l);
std::shared_ptr<Label> mutateLoopBegin(const std::shared_ptr<Label>& l);
std::shared_ptr<Label> mutateLoopEnd(const std::shared_ptr<Label>& l);
std::shared_ptr<Label> mutateSectionBegin(const std::shared_ptr<Label>& l);
std::shared_ptr<Label> mutateSectionEnd(const std::shared_ptr<Label>& l);
std::shared_ptr<Label> mutateSingleExecBegin(const std::shared_ptr<Label>& l);
std::shared_ptr<Label> mutateSingleOtherBegin(const std::shared_ptr<Label>& l);
std::shared_ptr<Label> mutateSingleEnd(const std::shared_ptr<Label>& l);


}
