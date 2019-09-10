#pragma once

#include <ompt.h>


/** 
 *  initialize OMPT interface by registering callback functions
 */
int omptInitialize(ompt_function_lookup_t functionLookup,
                   int initialDeviceNum,
                   ompt_data_t* toolData) {
   
}

/**
 *  release resources and log info upon finalization of tool
 */
void omptFinalize(ompt_data_t* toolData) {

}

/** 
 * implement ompt_start_tool which is defined in OpenMP spec 5.0
 */
ompt_start_tool_result_t* ompt_start_tool(
        unsigned int ompVersion,
        const char* runtimeVersion) {
  ompt_data_t data;
  static ompt_start_tool_result_t startToolResult = { &omptInitialize, &omptFinalize, data}; 
  return &startToolResult;
}
