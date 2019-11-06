#pragma once 
#include "Record.h"

namespace romp {

bool analyzeRaceCondition(const Record& histRecord, const Record& curRecord);
bool happensBefore(Label* histLabel, Label* curLabel, int& diffIndex);
bool analyzeDiffSegImpImp(Label* histLabel, Label* curLabel, int diffIndex);


}
