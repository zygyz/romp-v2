#include "Callbacks.h"

namespace romp {   

void on_ompt_callback_implicit_task(
       ompt_scope_endpoint_t endPoint,
       ompt_data_t* parallelData,
       ompt_data_t* taskData,
       unsigned int actualParallelism,
       unsigned int index) {
  LOG(INFO) << "on_ompt_callback_implicit_task called"; 
}

}


