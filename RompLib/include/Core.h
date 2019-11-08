#pragma once 
#include "Record.h"

namespace romp {

bool happensBefore(Label* histLabel, Label* curLabel, int& diffIndex);
bool analyzeRaceCondition(const Record& histRecord, const Record& curRecord);
bool analyzeSiblingImpTask(Label* histLabel, Label* curLabel, int diffIndex);
bool analyzeSameImpTask(Label* histLabel, Label* curLabel, int diffIndex);
bool analyzeOrderedSection(Label* histLabel, Label* curLabel, int startIndex);
bool inFinishScope(Label* label, int startIndex);
uint64_t computeExitRank(uint64_t phase);
uint64_t computeEnterRank(uint64_t phase);

}
