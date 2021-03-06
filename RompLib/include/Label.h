#pragma once
#include <memory>
#include <vector>
#include "Segment.h"

namespace romp {

enum LabelCompare {
  eSameLabel = -3,
  eLeftIsPrefix = -1,
  eRightIsPrefix = -2, 
};
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
  Segment* getKthSegment(int k);
  friend int compareLabels(Label* left, Label* right);
  int getLabelLength() const;
private:
  std::vector<std::shared_ptr<Segment> > _label;
};

int compareLabels(Label* left, Label* right);

std::shared_ptr<Label> genImpTaskLabel(
                          Label* parentLabel, 
                          unsigned int index,
                          unsigned int actualParallelism);

std::shared_ptr<Label> genInitTaskLabel();
std::shared_ptr<Label> genExpTaskLabel(Label* parentLabel);

std::shared_ptr<Label> mutateParentImpEnd(Label* childLabel);
std::shared_ptr<Label> mutateParentTaskCreate(Label* parentLabel);

std::shared_ptr<Label> mutateBarrierEnd(Label* label);
std::shared_ptr<Label> mutateTaskWait(Label* label);
std::shared_ptr<Label> mutateOrderSection(Label* label);
std::shared_ptr<Label> mutateLoopBegin(Label* label);
std::shared_ptr<Label> mutateLoopEnd(Label* label);
std::shared_ptr<Label> mutateSectionBegin(Label* label);
std::shared_ptr<Label> mutateSectionEnd(Label* label);
std::shared_ptr<Label> mutateSingleExecBegin(Label* label);
std::shared_ptr<Label> mutateSingleOtherBegin(Label* label);
std::shared_ptr<Label> mutateSingleEnd(Label* parentLabel);
std::shared_ptr<Label> mutateTaskLoopBegin(Label* label);
std::shared_ptr<Label> mutateTaskLoopEnd(Label* label);
std::shared_ptr<Label> mutateIterDispatch(Label* label, uint64_t id);
std::shared_ptr<Label> mutateSectionDispatch(Label* label, void* id);
std::shared_ptr<Label> mutateWorkShareDispatch(Label* l, uint64_t id, bool sec);
std::shared_ptr<Label> mutateTaskGroupBegin(Label* label);
std::shared_ptr<Label> mutateTaskGroupEnd(Label* label);
std::shared_ptr<Label> mutateTaskComplete(Label* label);
std::shared_ptr<Label> mutateTaskGroupSyncChild(Label* label);
}
