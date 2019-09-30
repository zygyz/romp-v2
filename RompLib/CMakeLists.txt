find_package(glog REQUIRED)

file(GLOB SOURCES src/*.cpp)

add_library(omptrace SHARED ${SOURCES})

find_path(LLVM_PATH omp.h)                    
find_path(GLOG_PATH "glog/logging.h")
find_path(GFLAGS_PATH "gflags/gflags.h")
          
target_include_directories(omptrace PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include  
                                    PUBLIC ${LLVM_PATH}
                                    PUBLIC ${GLOG_PATH}
                                    PUBLIC ${GFLAGS_PATH})
target_link_libraries(omptrace glog)
install(TARGETS omptrace 
        LIBRARY DESTINATION lib)
