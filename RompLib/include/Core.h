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

bool happensBefore(Label* hl, Label* cl, int& di, void* hp, void* cp);
bool analyzeRaceCondition(const Record& histRecord, const Record& curRecord);
bool analyzeSiblingImpTask(Label* hl, Label* cl, int di, void* hp, void* cp);
bool analyzeSameImpTask(Label* hl, Label* cl, int di, void* hp, void* cp);
bool analyzeOrderedSection(Label* hl, Label* cl, int di, void* hp,void* cp);
bool analyzeNextImpExp(Label* hl, Label* cl, int di);
bool analyzeNextImpWork(Label* hl, Label* cl, int di);
bool analyzeNextExpImp(Label* hl, Label* cl, int di);
bool analyzeNextExpExp(Label* hl, Label* cl, int di);
bool analyzeNextExpWork(Label* hl, Label* cl, int di);
bool analyzeNextWorkImp(Label* hl, Label* cl, int di);
bool analyzeNextWorkExp(Label* hl, Label* cl, int di);
bool analyzeNextWorkWork(Label* hl, Label* cl, int di);
bool analyzeOrderedDescendents(Label* hl, int si, void* hp, void* cp);
bool analyzeSyncChain(Label* label, int index);

bool inFinishScope(Label* label, int startIndex);
bool dispatchAnalysis(CheckCase checkCase, Label* hist, Label* cur, int index);
uint64_t computeExitRank(uint64_t phase);
uint64_t computeEnterRank(uint64_t phase);
inline CheckCase buildCheckCase(SegmentType histType, SegmentType curType);

}
