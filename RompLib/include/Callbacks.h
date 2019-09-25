#pragma once
#include <glog/logging.h>
#include <ompt.h>

namespace romp {
/*
 * ompt callback functions declarations
 */

void on_ompt_callback_implicit_task(
        ompt_scope_endpoint_t endPoint,
        ompt_data_t * parallelData,
        ompt_data_t * taskData,
        unsigned int actualParallelism,
        unsigned int index,
        int flags);

void on_ompt_callback_sync_region(
        ompt_sync_region_t kind,
        ompt_scope_endpoint_t endPoint,
        ompt_data_t *parallelData,
        ompt_data_t *taskData,
        const void* codePtrRa); 

// has signature of ompt_callback_mutex_t
void on_ompt_callback_mutex_acquired(
        ompt_mutex_t kind,
        ompt_wait_id_t waitId,
        const void *codePtrRa);   
     
void on_ompt_callback_mutex_released(
        ompt_mutex_t kind,
        ompt_wait_id_t waitId,
        const void *codePtrRa);   

}
