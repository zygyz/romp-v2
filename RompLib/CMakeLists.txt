find_package(glog REQUIRED)

file(GLOB SOURCES src/*.cpp)

add_library(omptrace SHARED ${SOURCES})

execute_process(COMMAND spack location --install-dir llvm-openmp 
                        OUTPUT_VARIABLE LLVM_DIR 
                        OUTPUT_STRIP_TRAILING_WHITESPACE)
                    
target_include_directories(omptrace PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include  
                                    PUBLIC ${LLVM_DIR}/include)
target_link_libraries(omptrace glog)
install(TARGETS omptrace 
        LIBRARY DESTINATION lib)
