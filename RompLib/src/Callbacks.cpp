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

void on_ompt_callback_mutex_acquired(
        ompt_mutex_t kind,
        ompt_wait_id_t waitId,
        const void *codePtrRa) {
  LOG(INFO) << "on_ompt_callback_mutex_acquired called";

}

void on_ompt_callback_mutex_released(
        ompt_mutex_t kind,
        ompt_wait_id_t waitId,
        const void *codePtrRa) {
  LOG(INFO) << "on_ompt_callback_mutex_released called";
}

void on_ompt_callback_work(
    ompt_work_t wsType,
    ompt_scope_endpoint_t endPoint,
    ompt_data_t *parallelData,
    ompt_data_t *taskData,
    uint64_t count,
    const void *codePtrRa) {
  LOG(INFO) << "on_ompt_callback_work called";

}


}

