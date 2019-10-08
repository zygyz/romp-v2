#pragma once
#include <ompt.h>

namespace romp {

enum DataSharingType { 
    eNonThreadPrivate,
    eThreadPrivateBelowExit, 
    eThreadPrivateAboveExit,
    eUndefined,
};

bool analyzeDataSharing(const void* threadDataPtr, 
                        const void* address,
                        const ompt_frame_t& currentExitFrame,
                        DataSharingType& dataSharingType);


}
