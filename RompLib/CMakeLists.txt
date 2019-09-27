find_package(glog REQUIRED)

file(GLOB SOURCES src/*.cpp)

add_library(omptrace SHARED ${SOURCES})

find_path(LLVM_PATH omp.h)                    
target_include_directories(omptrace PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include  
                                    PUBLIC ${LLVM_PATH})
target_link_libraries(omptrace glog)
install(TARGETS omptrace 
        LIBRARY DESTINATION lib)
