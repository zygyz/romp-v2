#pragma once
#include <cstdint>
#include <vector>

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
  uint64_t _genOffsetMask(const uint64_t numBits, const uint64_t lowZeros);

private:
  std::vector<std::vector<void*> > _pageTable;
  uint64_t _numEntriesPerPage;
  uint64_t _pageOffsetMask;
  uint64_t _pageOffsetShift;
  uint64_t _numL1PageTableEntries;
  uint64_t _numL2PageTableEntries;


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
  auto remainBits = numMemAddrBits - l1PageTableBits - l2PageTableBits;
  auto shadowPageBits = remainBits - lowZeroMask;
  _numEntriesPerPage = 1 << shadowPageBits;  
  _pageOffsetMask = _genOffsetMask(remainBits, lowZeroMask);

  _numL1PageTableEntries = 1 << l1PageTableBits;
  _numL2PageTableEntries = 1 << l2PageTableBits;
     
  std::vector<std::vector<void*> > tmp(_numL1PageTableEntries, 
          std::vector<void*>(_numL2PageTableEntries, nullptr));
  _pageTable = std::move(tmp);
}

template<typename T>
ShadowMemory<T>::~ShadowMemory() {
  // we should explicitly delete the shadow page
  for (const auto& l1Page : _pageTable) {
    for (const auto& l2Page : l1Page) {
      if (!l2Page) {
        delete l2Page;
      } 
    }
  }
}

template<typename T>
T* ShadowMemory<T>::getAllocatedPageForMemAddr(const uint64_t address) {
  T* shadowPage;
  return shadowPage;
}

template<typename T>
T* ShadowMemory<T>::getOrCreatePageForMemAddr(const uint64_t address) {
  //TODO
  T* tmp;

  return tmp;
}

template<typename T>
uint64_t ShadowMemory<T>::getPageOffset(const uint64_t address) {
  return (address & _pageOffsetMask) >> _pageOffsetShift;
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
uint64_t ShadowMemory<T>::_genOffsetMask(uint64_t numBits, uint64_t lowZeros) {
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


