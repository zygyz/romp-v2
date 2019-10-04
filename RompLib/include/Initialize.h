#pragma once
#include <glog/logging.h>
#include <ompt.h>

#include "Callbacks.h"
#include "QueryFuncs.h"

/* 
 * This header file defines functions that are used 
 * to initialize OMPT interface. 
*/
namespace romp{

bool gOmptInitialized = false; 
ompt_get_task_info_t omptGetTaskInfo;
ompt_get_parallel_info_t omptGetParallelInfo;
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

/** 
 *  initialize OMPT interface by registering callback functions
 */
int omptInitialize(ompt_function_lookup_t lookup,
                   int initialDeviceNum,
                   ompt_data_t* toolData) {
  google::InitGoogleLogging("romp");
  LOG(INFO) << "start initializing ompt";
  auto ompt_set_callback = 
      (ompt_set_callback_t)lookup("ompt_set_callback");

  register_callback_t(ompt_callback_mutex_acquired, ompt_callback_mutex_t);
  register_callback_t(ompt_callback_mutex_released, ompt_callback_mutex_t);
  register_callback_t(ompt_callback_reduction, ompt_callback_sync_region_t);
  register_callback(ompt_callback_implicit_task);
  register_callback(ompt_callback_sync_region);
  register_callback(ompt_callback_work);
  register_callback(ompt_callback_parallel_begin);
  register_callback(ompt_callback_parallel_end);
  register_callback(ompt_callback_task_create);
  register_callback(ompt_callback_task_schedule);
  register_callback(ompt_callback_dependences);
  register_callback(ompt_callback_thread_begin);
  register_callback(ompt_callback_thread_end);
  register_callback(ompt_callback_dispatch);

  omptGetTaskInfo = (ompt_get_task_info_t)lookup("ompt_get_task_info");
  omptGetParallelInfo = (ompt_get_parallel_info_t)lookup("ompt_get_parallel_info");
  omptGetThreadData = (ompt_get_thread_data_t)lookup("ompt_get_thread_data");

  gOmptInitialized = true;
  return 1;
}

/**
 *  release resources and log info upon finalization of tool
 */
void omptFinalize(ompt_data_t* toolData) {
  LOG(INFO) << "finalizing ompt";
}

}
