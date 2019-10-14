#include "ShadowMemory.h"


namespace romp {

template<typename T>
ShadowMemory<T>::ShadowMemory(uint64_t numL1PageTableBits, 
                              uint64_t numL2PageTableBits,
                              Granularity granularity) {


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
uint64_t ShadowMemory<T>::_getPageOffset(const uint64_t address) {
  //TODO 
  return 0;
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

