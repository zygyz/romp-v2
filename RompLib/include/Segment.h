#pragma once
#include <cstdint>
#include <memory> 
#include <string>

namespace romp {

enum SegmentType {
  eImplicit = 0x1,
  eExplicit = 0x2,
  eWorkShare = 0x3, 
};
/*
 *  The abstract class definition for label segment 
 */
class Segment {
public: 
  virtual std::string toString() const = 0;
  virtual void setType(SegmentType type) = 0; 
  virtual SegmentType getType() const = 0;
  virtual std::shared_ptr<Segment> clone() const = 0;  
  virtual void setOffsetSpan(uint64_t offset, uint64_t span) = 0;
  virtual void getOffsetSpan(uint64_t& offset, uint64_t& span) const = 0;
  virtual void setTaskwait(uint64_t taskwait) = 0;
  virtual void getTaskwait(uint64_t& taskwait) const = 0;
  virtual void setPhase(uint64_t phase) = 0;
  virtual void getPhase(uint64_t& phase) const = 0;
  virtual void setLoopCount(uint64_t loopCount) = 0;
  virtual void getLoopCount(uint64_t& loopCount) const = 0;  
  virtual bool operator==(const Segment& rhs) const = 0;
  virtual bool operator!=(const Segment& rhs) const = 0;
  virtual ~Segment() = default;
};

/*
 * Base segment is for representing implicit task with no 
 * worksharing construct attached to it.
 */
class BaseSegment : public Segment {
public:
  BaseSegment(): _value(0) {}
  BaseSegment(const BaseSegment& segment): _value(segment._value) {}
  BaseSegment(SegmentType type, uint64_t offset, uint64_t span);
  std::string toString() const override;
  void setType(SegmentType type) override;
  SegmentType getType() const override;
  std::shared_ptr<Segment> clone() const override;
  void setOffsetSpan(uint64_t offset, uint64_t span) override;
  void getOffsetSpan(uint64_t& offset, uint64_t& span) const override;
  void setTaskwait(uint64_t taskwait) override;
  void getTaskwait(uint64_t& taskwait) const override;
  void setPhase(uint64_t phase) override;
  void getPhase(uint64_t& phase) const override;
  void setLoopCount(uint64_t loopCount) override;
  void getLoopCount(uint64_t& loopCount) const override;
  bool operator==(const Segment& rhs) const override; 
  bool operator!=(const Segment& rhs) const override;
  uint64_t getValue() const;
protected:
  uint64_t _value;
};

/*
 * Workshare segment is for representing task with worksharing 
 * construct.
 */
class WorkShareSegment: public BaseSegment {
public:
  WorkShareSegment() : _workShareId(0) { setType(eWorkShare); }
  WorkShareSegment(uint64_t id, bool isSection): _workShareId(id) { 
    setType(eWorkShare); 
    setWorkShareType(isSection);
  } 
  WorkShareSegment(const WorkShareSegment& segment): BaseSegment(segment), 
     _workShareId(segment._workShareId) { }                       
  void setPlaceHolderFlag(bool toggle);
  bool isPlaceHolder() const;
  void setWorkShareType(bool isSection);
  bool isSection() const;
  void setSingleFlag(bool isExecutor);
  bool isSingleExecutor() const;
  bool isSingleOther() const;
  std::string toString() const override;
  std::shared_ptr<Segment> clone() const override;
  bool operator==(const Segment& rhs) const override;
  bool operator!=(const Segment& rhs) const override;
private: 
  uint64_t _workShareId; 
};


}
