#pragma once 
#include "Record.h"

namespace romp {

/*
 * Different sub cases for happens before analysis. Each case represents the 
 * segment type of corresponding next segment after the first pair of different
 * segments.
 */
#define CASE_SHIFT 2
#define DBEUG_CORE

enum CheckCase {
  eImpImp = eImplicit | (eImplicit << CASE_SHIFT),
  eImpExp = eImplicit | (eExplicit << CASE_SHIFT),
  eImpWork = eImplicit | (eWorkShare << CASE_SHIFT),
  eExpImp = eExplicit | (eImplicit << CASE_SHIFT),
  eExpExp = eExplicit | (eExplicit << CASE_SHIFT),
  eExpWork = eExplicit | (eWorkShare<< CASE_SHIFT),
  eWorkImp = eWorkShare | (eImplicit << CASE_SHIFT),
  eWorkExp = eWorkShare | (eExplicit << CASE_SHIFT),
  eWorkWork = eWorkShare | (eWorkShare << CASE_SHIFT),
}; 

bool happensBefore(Label* histLabel, Label* curLabel, int& diffIndex);
bool analyzeRaceCondition(const Record& histRecord, const Record& curRecord);
bool analyzeSiblingImpTask(Label* histLabel, Label* curLabel, int diffIndex);
bool analyzeSameImpTask(Label* histLabel, Label* curLabel, int diffIndex);
bool analyzeOrderedSection(Label* histLabel, Label* curLabel, int startIndex);
bool analyzeNextImpExp(Label* histLabel, Label* curLabel, int diffIndex);
bool analyzeNextImpWork(Label* histLabel, Label* curLabel, int diffIndex);
bool analyzeNextExpImp(Label* histLabel, Label* curLabel, int diffIndex);
bool analyzeNextExpExp(Label* histLabel, Label* curLabel, int diffIndex);
bool analyzeNextExpWork(Label* histLabel, Label* curLabel, int diffIndex);
bool analyzeNextWorkImp(Label* histLabel, Label* curLabel, int diffIndex);
bool analyzeNextWorkExp(Label* histLabel, Label* curLabel, int diffIndex);
bool analyzeNextWorkWork(Label* histLabel, Label* curLabel, int diffIndex);


bool inFinishScope(Label* label, int startIndex);
bool dispatchAnalysis(CheckCase checkCase, Label* hist, Label* cur, int index);
uint64_t computeExitRank(uint64_t phase);
uint64_t computeEnterRank(uint64_t phase);
inline CheckCase buildCheckCase(SegmentType histType, SegmentType curType);

}
