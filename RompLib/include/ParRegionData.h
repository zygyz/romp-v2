#pragma once
#include <ompt.h>

#include "McsLock.h"
#include "TaskDepGraph.h"

namespace romp {

typedef struct ParRegionData {
  void* dataPtr;  
  unsigned int numParallelism;
  int parallelFlag;
  McsLock lock;      
  std::atomic_int expTaskCount; 
  ParRegionData() { mcsInit(&lock); }
  ParRegionData(unsigned int n, int p): numParallelism(n), parallelFlag(p) {
    dataPtr = nullptr; 
    expTaskCount = 0;
    mcsInit(&lock);
  } 
  TaskDepGraph taskDepGraph;
} ParRegionData;

void maintainTaskDeps(const ompt_dependence_t& dependence, 
		      void* taskPtr, 
		      ParRegionData* parRegionData);

}
