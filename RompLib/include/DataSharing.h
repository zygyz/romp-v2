#pragma once
#include <ompt.h>

namespace romp {

enum DataSharingType { 
    eNonThreadPrivate,
    eThreadPrivateBelowExit, 
    eThreadPrivateAboveExit,
    eUndefined,
};

DataSharingType analyzeDataSharing(const void* threadDataPtr, 
                                   const void* address,
                                   const ompt_frame_t& taskFrame);


}