option(CUSTOM_DYNINST "use custom dyninst build" OFF)
set(DYNINST_LIBRARIES_PATH "" CACHE STRING "dyninst library string")
find_package(Boost REQUIRED)
find_package(gflags REQUIRED)
find_package(glog REQUIRED)

add_executable(InstrumentMain InstrumentMain.cpp 
                              InstrumentClient.cpp)
  
if (CUSTOM_DYNINST MATCHES "ON")
    include_directories(${DYNINST_LIBRARIES_PATH}/include)
    target_link_libraries(InstrumentMain -L/home/yg31/working-dir/dyninst-10.1.0/dyninst_install/lib)
    target_link_libraries(InstrumentMain -ldyninstAPI -linstructionAPI 
                          -lparseAPI -lsymtabAPI)
    target_link_libraries(InstrumentMain gflags glog) 
    target_link_libraries(InstrumentMain boost_system)
else()
  find_package(Dyninst REQUIRED dyninstAPI symtabAPI instructionAPI parseAPI)
  target_link_libraries(InstrumentMain dyninstAPI parseAPI symtabAPI 
                        instructionAPI gflags glog) 
endif()

install(TARGETS InstrumentMain DESTINATION bin)
