#include "Core.h"

#include <glog/logging.h>
#include <glog/raw_logging.h>

namespace romp {

bool analyzeRaceCondition(const Record& histRecord, const Record& curRecord) {
  auto histLabel = histRecord.getLabel(); 
  auto curLabel = curRecord.getLabel(); 
  // compare two labels
  auto diffIndex = compareLabels(histLabel, curLabel);

  return false;
}

}
