#include "Callbacks.h"

namespace romp {   

void on_ompt_callback_implicit_task(
       ompt_scope_endpoint_t endPoint,
       ompt_data_t* parallelData,
       ompt_data_t* taskData,
       unsigned int actualParallelism,
       unsigned int index,
       int flags) {
  LOG(INFO) << "on_ompt_callback_implicit_task called"; 
}

void on_ompt_callback_sync_region(
       ompt_sync_region_t kind,
       ompt_scope_endpoint_t endPoint,
       ompt_data_t *parallelData,
       ompt_data_t *taskData,
       const void* codePtrRa) {
  LOG(INFO) << "on_ompt_callback_sync_region called";

}

}
