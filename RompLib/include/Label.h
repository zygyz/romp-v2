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
  Label(); 
  Label(const Label& label);
  ~Label(); 
  void print() const;
  void appendSegment(std::shared_ptr<Segment>& segment);

private:
  std::vector<std::shared_ptr<Segment> > _label;

};

std::shared_ptr<Label> genImpTaskLabel(
                          const std::shared_ptr<Label>& parentLabel, 
                          unsigned int index,
                          unsigned int actualParallelism);


}
