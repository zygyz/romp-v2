#include "Segment.h"

#include <glog/logging.h>
#include <glog/raw_logging.h>
#include <sstream>

#define SEG_TYPE_MASK        0x0000000000000003
#define OFFSET_MASK          0xffff000000000000
#define SPAN_MASK            0x0000ffff00000000
#define TASKWAIT_MASK        0x00000000f0000000
#define PHASE_MASK           0x000000000f000000
#define WS_PLACE_HOLDER_MASK 0xfffffffffffffffb
#define LOOP_CNT_MASK        0x0000000000f00000
#define TASK_CREATE_MASK     0x00000000000fffe0
#define SINGLE_MASK          0xc000000000000000
#define WORKSHARE_TYPE_MASK  0x0000000000000004
#define TASKWAIT_SYNC_MASK   0x0000000000000008
#define TASKGROUP_LEVEL_MASK 0x00000000ffffffff

#define OFFSET_SPAN_WIDTH 16

#define OFFSET_SHIFT 48
#define SPAN_SHIFT 32
#define TASKWAIT_SHIFT 28
#define PHASE_SHIFT 24
#define LOOP_CNT_SHIFT 20
#define TASK_CREATE_SHIFT 5 // can handle spawn <= 2^15 exp tasks
#define WS_PLACE_HOLDER_POS 2  // least significant bit index is 0
#define SINGLE_EXEC_SHIFT 63
#define SINGLE_OTHER_SHIFT 62


namespace romp {

/*
 * Each segment contains a 64 bit value. From low to high, assign index 0-63
 * [0,1]: segment type 
 * [48, 63]: offset 
 * [32, 47]: span 
 * [28, 31]: taskwait count
 * [24, 27]: phase count
 * [20, 23]: loop count
 * [5, 19]: task create count
 * [3]: mark if current task (must be explicit) syncs with taskwait 
 * [2]: mark if current workshare semgent is section, bit set: yes. 
 *      otherwise, sgment is iteration
 *
 * For workshare segment, we use the extra _workShareId to store information
 * [0,31]: work share id
 * [62,63]: single construct flag bits 
 */
std::string BaseSegment::toString() const {
  std::stringstream stream;
  stream << std::hex << _value;
  auto result = "[" + stream.str() + "]";
  return result;
}

BaseSegment::BaseSegment(SegmentType type, uint64_t offset, 
        uint64_t span) {
  RAW_CHECK(span < (1 << OFFSET_SPAN_WIDTH), "span is overflowing");
  _value = 0;
  setType(type);
  setOffsetSpan(offset, span);
}

std::shared_ptr<Segment> BaseSegment::clone() const {
  return std::make_shared<BaseSegment>(*this);
}

uint64_t BaseSegment::getValue() const {
  return _value;
}

void BaseSegment::setOffsetSpan(uint64_t offset, uint64_t span) {
  _value &= ~(OFFSET_MASK | SPAN_MASK);  // clear the offset, span field
  _value |= (offset << OFFSET_SHIFT) & OFFSET_MASK; 
  _value |= (span << SPAN_SHIFT) & SPAN_MASK; 
}

void BaseSegment::getOffsetSpan(uint64_t& offset, uint64_t& span) const {
  offset = (_value & OFFSET_MASK) >> OFFSET_SHIFT;
  span = (_value & SPAN_MASK) >> SPAN_SHIFT;
}

/* 
 * Taskgroup id increases monotonically. It is at the upper half of the
 * 64 bits long word _taskGroup 
 */
uint32_t BaseSegment::getTaskGroupId() const {
  return static_cast<uint32_t>(_taskGroup >> 32);
}
                                   
void BaseSegment::setTaskGroupId(uint32_t taskGroupId) {
  _taskGroup &= TASKGROUP_LEVEL_MASK; // clear the upper half 
  _taskGroup |= (static_cast<uint64_t>(taskGroupId) << 32) & 
      ~TASKGROUP_LEVEL_MASK;
}

/*
 * Taskgroup level marks the nested number of level of taskgorup. 
 * It is the lower half of the 64 bits long word _taskGroup
 */
uint32_t BaseSegment::getTaskGroupLevel() const {
  return static_cast<uint32_t>(_taskGroup & TASKGROUP_LEVEL_MASK); 
}

void BaseSegment::setTaskwaited() {
  _value |= TASKWAIT_SYNC_MASK; 
}

bool BaseSegment::isTaskwaited() const {
  return (_value & TASKWAIT_SYNC_MASK) != 0;
}

void BaseSegment::setTaskGroupLevel(uint32_t taskGroupLevel) {
  _taskGroup |= static_cast<uint64_t>(taskGroupLevel) & TASKGROUP_LEVEL_MASK;
}

bool BaseSegment::operator==(const Segment& segment) const {
  return _value == dynamic_cast<const BaseSegment&>(segment)._value;
}

bool BaseSegment::operator!=(const Segment& segment) const {
  return !(*this == segment);
}
/*
 * Taskwait field is four bits. So if taskwait is more than 16, it overflows.
 */
void BaseSegment::setTaskwait(uint64_t taskwait) {
  RAW_CHECK(taskwait < 16, "taskwait count is overflowing");
  _value &= ~TASKWAIT_MASK; // clear the taskwait field
  _value |= (taskwait << TASKWAIT_SHIFT) & TASKWAIT_MASK;
}

uint64_t BaseSegment::getTaskwait() const {
  uint64_t taskwait = (_value & TASKWAIT_MASK) >> TASKWAIT_SHIFT;
  return taskwait;
}

void BaseSegment::setTaskcreate(uint64_t taskcreate) { 
  RAW_CHECK(taskcreate < (1 << 15), "taskcreate count is overflowing");
  _value &= ~TASK_CREATE_MASK;
  _value |= (taskcreate << TASK_CREATE_SHIFT) & TASK_CREATE_MASK;
}

uint64_t BaseSegment::getTaskcreate() const {
  uint64_t taskcreate = (_value & TASK_CREATE_MASK) >> TASK_CREATE_SHIFT;
  return taskcreate;
}

void BaseSegment::setPhase(uint64_t phase) {
  RAW_CHECK(phase < 16, "phase count is overflowing");
  _value &= ~PHASE_MASK;
  _value |= (phase << PHASE_SHIFT) & PHASE_MASK;

}

uint64_t BaseSegment::getPhase() const {
  uint64_t phase = (_value & PHASE_MASK) >> PHASE_SHIFT;
  return phase;
}

void BaseSegment::setLoopCount(uint64_t loopCount) {
  RAW_CHECK(loopCount < 16, "loop count is overflowing");
  _value &= ~LOOP_CNT_MASK;
  _value |= (loopCount << LOOP_CNT_SHIFT) & LOOP_CNT_MASK;
}

uint64_t BaseSegment::getLoopCount() const {
  uint64_t loopCount = (_value & LOOP_CNT_MASK) >> LOOP_CNT_SHIFT;
}

void BaseSegment::setType(SegmentType type) {
  _value |= static_cast<uint64_t>(type);
}

SegmentType BaseSegment::getType() const {
  auto mask = _value & SEG_TYPE_MASK;
  switch(mask) {
    case 0x1:
      return eImplicit;
    case 0x2:
      return eExplicit;
    case 0x3:
      return eWorkShare; 
  }
  RAW_LOG(FATAL, "undefined segment type: %d", mask);
}

std::string WorkShareSegment::toString() const {
  std::stringstream stream;
  stream << std::hex << _value;
  auto result = "[" + stream.str() + "," + 
      std::to_string(_workShareId) + "]";
  return result;
}

std::shared_ptr<Segment> WorkShareSegment::clone() const {
  return std::make_shared<WorkShareSegment>(*this);
}

/*
 * Set place holder flag for the workshare segment. If toggle is true,
 * set the flag, otherwise, clear the flag.
 */
void WorkShareSegment::setPlaceHolderFlag(bool toggle) {
  if (toggle) {
    _value |= (1 << WS_PLACE_HOLDER_POS);
  } else {
    _value &= WS_PLACE_HOLDER_MASK;
  }
}

bool WorkShareSegment::operator==(const Segment& segment) const {
  if (_value == dynamic_cast<const BaseSegment&>(segment).getValue()) {
    // we know `segment` is also a workshare segment
    return _workShareId == 
        dynamic_cast<const WorkShareSegment&>(segment)._workShareId;
  } 
  return false;
}

bool WorkShareSegment::operator!=(const Segment& segment) const {
  return !(*this == segment);
}

bool WorkShareSegment::isPlaceHolder() const {
  return ((_value & WS_PLACE_HOLDER_MASK) >> WS_PLACE_HOLDER_POS) == 1;
}

bool WorkShareSegment::isSingleExecutor() const {
  return ((_workShareId & SINGLE_MASK) >> SINGLE_EXEC_SHIFT) == 1;
}

bool WorkShareSegment::isSingleOther() const {
  return ((_workShareId & SINGLE_MASK) >> SINGLE_OTHER_SHIFT) == 1;
}

uint64_t WorkShareSegment::getWorkShareId() const {
  return _workShareId;
}

void WorkShareSegment::setSingleFlag(bool isExecutor) {
  _workShareId &= ~SINGLE_MASK;
  uint64_t b = 1;
  if (isExecutor) {
    // toggle the higher bit to 1
    _workShareId |= (b << SINGLE_EXEC_SHIFT);
  } else {
    // single other, toggle the lower bit to 1
    _workShareId |= (b << SINGLE_OTHER_SHIFT);
  }
}

void WorkShareSegment::setWorkShareType(bool isSection) {
  _value &= ~WORKSHARE_TYPE_MASK; // clear the bit first
  if (isSection) {
    _value |= WORKSHARE_TYPE_MASK;  // set the bit
  } 
}

bool WorkShareSegment::isSection() const {
  return (_value & WORKSHARE_TYPE_MASK) != 0;
}


}
