option(CUSTOM_DYNINST "use custom dyninst build" OFF)
set(DYNINST_LIBRARIES_PATH "" CACHE STRING "dyninst library string")
find_package(Boost REQUIRED)
find_package(gflags REQUIRED)
find_package(glog REQUIRED)

add_executable(InstrumentMain InstrumentMain.cpp 
                              InstrumentClient.cpp)
  
if (CUSTOM_DYNINST MATCHES "ON")
    include_directories(${DYNINST_LIBRARIES_PATH}/include)
    find_library(DYNINST_LIB dyninstAPI)
    find_library(INSTRUCTION_LIB instructionAPI)
    find_library(SYMTAB_LIB symtabAPI)
    find_library(PARSE_LIB parseAPI)
    find_library(BOOST_SYSTEM_LIB boost_system)
    #target_link_libraries(InstrumentMain -L${DYNINST_LIBRARIES_PATH}/lib)
    target_link_libraries(InstrumentMain "${DYNINST_LIB}")
    target_link_libraries(InstrumentMain "${INSTRUCTION_LIB}")
    target_link_libraries(InstrumentMain "${SYMTAB_LIB}")
    target_link_libraries(InstrumentMain "${PARSE_LIB}")
    target_link_libraries(InstrumentMain "${BOOST_SYSTEM_LIB}")
    #target_link_libraries(InstrumentMain -ldyninstAPI -linstructionAPI 
    #                       -lsymtabAPI -lparseAPI -lboost_system)
    target_link_libraries(InstrumentMain gflags glog) 
else()
  find_package(Dyninst REQUIRED dyninstAPI symtabAPI instructionAPI parseAPI)
  target_link_libraries(InstrumentMain dyninstAPI parseAPI symtabAPI 
                        instructionAPI gflags glog) 
endif()

install(TARGETS InstrumentMain DESTINATION bin)
