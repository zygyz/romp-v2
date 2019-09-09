find_package(gflags REQUIRED)
find_package(glog REQUIRED)
find_package(llvm-openmp REQUIRED)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O2 -std=c++11 -fpermissive -fno-builtin-malloc -fno-builtin-calloc -fno-builtin-realloc -fno-builtin-free -L../../gperftools/gperftools-install/lib -L../../llvm-openmp/openmp/llvm-openmp-install/lib  -lpthread -ltcmalloc -lelf")

file(GLOB SOURCES src/*.cpp)

#message(STATUS "source = ${SOURCES}")
add_library(omptrace SHARED ${SOURCES})

target_include_directories(omptrace ${CMAKE_CURRENT_SOURCE_DIR}/include)

install(TARGETS omptrace 
        LIBRARY DESTINATION lib)
