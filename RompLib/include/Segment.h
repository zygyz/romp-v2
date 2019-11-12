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
  virtual void setTaskwait(uint64_t taskwait) = 0;
  virtual void setTaskcreate(uint64_t taskcreate) = 0; 
  virtual void setPhase(uint64_t phase) = 0;
  virtual void setLoopCount(uint64_t loopCount) = 0;
  virtual void setTaskGroupId(uint32_t taskGroupId) = 0;
  virtual void setTaskGroupLevel(uint32_t taskGroupLevel) = 0;
  virtual void getOffsetSpan(uint64_t& offset, uint64_t& span) const = 0;
  virtual uint64_t getTaskwait() const = 0;
  virtual uint64_t getTaskcreate() const = 0;
  virtual uint64_t getPhase() const = 0;
  virtual uint64_t getLoopCount() const = 0;  
  virtual uint32_t getTaskGroupId() const = 0;
  virtual uint32_t getTaskGroupLevel() const = 0;

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
  void setTaskwait(uint64_t taskwait) override;
  void setTaskcreate(uint64_t taskcreate) override;
  void setPhase(uint64_t phase) override;
  void setLoopCount(uint64_t loopCount) override;
  void setTaskGroupId(uint32_t taskGroupId) override;
  void setTaskGroupLevel(uint32_t taskGroupLevel) override;
  void getOffsetSpan(uint64_t& offset, uint64_t& span) const override;
  uint64_t getTaskwait() const override;
  uint64_t getTaskcreate() const override;
  uint64_t getPhase() const override;
  uint64_t getLoopCount() const override;
  uint32_t getTaskGroupId() const override;
  uint32_t getTaskGroupLevel() const override;
  bool operator==(const Segment& rhs) const override; 
  bool operator!=(const Segment& rhs) const override;
  uint64_t getValue() const;
protected:
  uint64_t _value;
  uint64_t _taskGroup;
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
  uint64_t getWorkShareId() const;
  std::string toString() const override;
  std::shared_ptr<Segment> clone() const override;
  bool operator==(const Segment& rhs) const override;
  bool operator!=(const Segment& rhs) const override;
private: 
  uint64_t _workShareId; 
};


}
