#pragma once
#include "Callbacks.h"
/* 
 * This header file defines functions that are used 
 * to initialize OMPT interface. 
*/
namespace romp{

/* 
 * Define macro for registering ompt callback functions. 
 */
#define register_callback(name, type)                        \
do {                                                         \
  type f_##name = &on_##name;                                \
  if (ompt_set_callback(name, (ompt_callback_t)f_##name) ==  \
      ompt_set_never)                                        \
    LOG(ERROR) << "Could not register callback";             \
} while(0)

#define register_callback(name) register_callback_t(name, name##_t)

/** 
 *  initialize OMPT interface by registering callback functions
 */
int omptInitialize(ompt_function_lookup_t functionLookup,
                   int initialDeviceNum,
                   ompt_data_t* toolData) {
  google::InitGoogleLogging("romp");
  LOG(INFO) << "start initializing ompt";      
  auto ompt_set_callback = (ompt_set_callback_t)lookup("ompt_set_callback");
  register_callback(ompt_callback_implicit_task);
  return 1;

}

/**
 *  release resources and log info upon finalization of tool
 */
void omptFinalize(ompt_data_t* toolData) {
  LOG(INFO) << "finalizing ompt";
}


}

