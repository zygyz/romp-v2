#pragma once

/*
 * This header file declares ShadowMemory class template for managing shadow 
 * memory. Type T is the type of struct of access history. We use class 
 * template here to decouple the implementation of shadow memory management
 * and the actual form of access history. We assume the shadow memory works
 * on 64 bits system. So we use uint64_t to represent void*
 */
namespace romp {

enum Granularity {
  eByteLevel,
  eWordLevel,
};

template<typename T>
class ShadowMemory {

public:
  ShadowMemory(uint64_t numL1PageTableBits, 
               uint64_t numL2PageTableBits,
               Granularity granularity);

public:
  T* getAllocatedPageForMemAddr(const uint64_t address);
  T* getOrCreatePageForMemAddr(const uint64_t address);   

private:
  uint64_t _getPageOffset(const uint64_t address);
  void* _getShadowPage(); 
  void _saveShadowPage(const uint64_t pageBaseAddress);

private:
  void*** pageTable;  
  uint64_t _pageOffsetBits; 
  uint64_t _pageOffsetMask;

  uint64_t _numL1PageTableBits;
  uint64_t _numL1PageTableEntries;
  uint64_t _numL1PageTableSize;

  uint64_t _numL2PageTableBits;
  uint64_t _numL2PageTableEntries;
  uint64_t _numL2PageTableSize;




};



}


