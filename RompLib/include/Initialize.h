#pragma once
#include <glog/logging.h>

/* 
 * This header file defines functions that are used 
 * to initialize OMPT interface. 
*/
namespace romp{

bool gOmptInitialized = false; 
/* 
 * Define macro for registering ompt callback functions. 
 */
#define register_callback_t(name, type)                      \
do {                                                         \
  type f_##name = &on_##name;                                \
  if (ompt_set_callback(name, (ompt_callback_t)f_##name) ==  \
      ompt_set_never)                                        \
    LOG(ERROR) << "Could not register callback";             \
} while(0)

#define register_callback(name) register_callback_t(name, name##_t)

int omptInitialize(ompt_function_lookup_t lookup,
                   int initialDeviceNum,
                   ompt_data_t* toolData);

void omptFinalize(ompt_data_t* toolData);

}

