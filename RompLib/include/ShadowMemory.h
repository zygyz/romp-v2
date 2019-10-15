#pragma once
#include <cstdint>
#include <glog/logging.h>
#include <glog/raw_logging.h>

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
  eWordLevel, // aligned four bytes treated as the same memory access
  eLongWordLevel, // aligned eight bytes treated as the same memory access
};

template<typename T>
class ShadowMemory {

public:
  ShadowMemory(uint64_t l1PageTableBits, 
               uint64_t l2PageTableBits,
               uint64_t numMemAddrBits,
               Granularity granularity);

  ~ShadowMemory();
public:
  T* getAllocatedPageForMemAddr(const uint64_t address);
  T* getOrCreatePageForMemAddr(const uint64_t address);   
  uint64_t getPageOffset(const uint64_t address);
  uint64_t getNumEntriesPerPage();

private:
  uint64_t _getPageOffset(const uint64_t address);
  uint64_t _getShadowPage(); 
  void _saveShadowPage(const uint64_t pageBaseAddress);
  uint64_t _genPageIndexMask(const uint64_t numBits, const uint64_t lowZeros);
  uint64_t _getL1PageIndex(const uint64_t address);
  uint64_t _getL2PageIndex(const uint64_t address);

private:
  uint64_t*** _pageTable; 
  uint64_t _numEntriesPerPage;
  uint64_t _shadowPageIndexMask;
  uint64_t _pageOffsetShift;
  uint64_t _numL1PageTableEntries;
  uint64_t _numL2PageTableEntries;
  uint64_t _l1PageTableShift;
  uint64_t _l2PageTableShift;


};


/*
 * numMemAddrBits: number of effective bits in a memory address. For x86-64, 
 *                 the value is 48. 
 * l1PageTableBits: number of higher part of the memory address bits. This 
 *                 part of memory address is used as index into the first 
 *                 level page table.
 * l2PageTableBits: number of middle part of the memory address bits. This 
 *                 part of memory address is used as index into the second 
 *                 level page table. 
 * Each page contains 2^(numMemAddrBits - l1PageTableBits - l2PageTableBits)
 * entries. For byte level granularity, each byte is associated with its own
 * entry. For word level granularity, every aligned four bytes are associated 
 * with one entry. For long word level granularity, every aligned eight bytes 
 * are associated with one entry.
 */
template<typename T>
ShadowMemory<T>::ShadowMemory(uint64_t l1PageTableBits,
                              uint64_t l2PageTableBits,
                              uint64_t numMemAddrBits, 
                              Granularity granularity) {
  uint64_t lowZeroMask = 0;
  switch(granularity) {
    case eByteLevel:
      lowZeroMask = 0;
      break;
    case eWordLevel:
      lowZeroMask = 2;
      break;
    case eLongWordLevel:
      lowZeroMask = 3;
      break;
    default:
      lowZeroMask = 0;
      break;
  }
  _l1PageTableShift = numMemAddrBits - l1PageTableBits;  
  _l2PageTableShift = _l1PageTableShift - l2PageTableBits; 

  _numEntriesPerPage = 1 << (_l2PageTableShift - lowZeroMask);  

  _shadowPageIndexMask = _genPageIndexMask(_l2PageTableShift, lowZeroMask);

  _numL1PageTableEntries = 1 << l1PageTableBits;
  _numL2PageTableEntries = 1 << l2PageTableBits;
     
  // For l1PageTableBits = 20, this allocates a chunk of memory of size 
  // 2^20 * 8 = 8 Mb, which is managable.
  auto tmp = calloc(1, sizeof(uint64_t**) * _numL1PageTableEntries);
  if (tmp == NULL) {
    LOG(FATAL) << "cannot create page table";
  }
  _pageTable = static_cast<uint64_t***>(tmp); 
}

template<typename T>
ShadowMemory<T>::~ShadowMemory() {
  // we should explicitly delete the shadow page
  for (int i = 0; i < _numL1PageTableEntries; ++i) {
    if (_pageTable[i] != 0) {
      for (int j = 0; j < _numL2PageTableEntries; ++j) {
        if (_pageTable[i][j] != 0) {
          free(_pageTable[i][j]); //free the leaf shadow page
        }
      }
      free(_pageTable[i]);
    }
  }
  free(_pageTable);
}

/* 
 * Given the memory address, using the high bits to get the index into the 
 * first level page table.
 */
template<typename T>
uint64_t ShadowMemory<T>::_getL1PageIndex(const uint64_t address) {  
  return static_cast<uint64_t>(address >> _l1PageTableShift);
}

template<typename T>
uint64_t ShadowMemory<T>::_getL2PageIndex(const uint64_t address) {
  return static_cast<uint64_t>(address >> _l2PageTableShift);
}

template<typename T>
T* ShadowMemory<T>::getAllocatedPageForMemAddr(const uint64_t address) {
  T* shadowPage;
  return shadowPage;
}

template<typename T>
T* ShadowMemory<T>::getOrCreatePageForMemAddr(const uint64_t address) {
  //TODO
  auto l1PageTableIndex = _getL1PageIndex(address);
  return nullptr;
}

template<typename T>
uint64_t ShadowMemory<T>::getPageOffset(const uint64_t address) {
  return (address & _shadowPageIndexMask) >> _pageOffsetShift;
}


template<typename T>
uint64_t ShadowMemory<T>::getNumEntriesPerPage() {
  return _numEntriesPerPage;
}

template<typename T>
uint64_t ShadowMemory<T>::_getPageOffset(const uint64_t address) {
  //TODO 
  return 0;
}

/* 
 * Generate the mask value that is composed of `numBits` consequtive bits,
 * while masking lowest `mask` bits as 0.
 */
template<typename T>
uint64_t ShadowMemory<T>::_genPageIndexMask(uint64_t numBits, uint64_t lowZeros) {
  return (1 << numBits) - (1 << lowZeros);
}

template<typename T>
uint64_t ShadowMemory<T>::_getShadowPage() { 
  //TODO
  return 0;
}

template<typename T>
void ShadowMemory<T>::_saveShadowPage(const uint64_t  pageBaseAddress) {
  //TODO
}


}


