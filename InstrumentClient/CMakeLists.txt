option(CUSTOM_DYNINST "use custom dyninst build" OFF)
SET(DYNINST_LIBRARIES_PATH "")
find_package(Boost REQUIRED)
find_package(gflags REQUIRED)
find_package(glog REQUIRED)

add_executable(InstrumentMain InstrumentMain.cpp 
                              InstrumentClient.cpp)
  
if (CUSTOM_DYNINST MATCHES "ON")
    message(STATUS "dyninst library path: ${DYNINST_LIBRARIES_PATH}")
    target_link_libraries(InstrumentMain "${DYNINST_LIBRARIES_PATH}" gflags glog) 
else()
  find_package(Dyninst REQUIRED dyninstAPI symtabAPI instructionAPI parseAPI)
  target_link_libraries(InstrumentMain dyninstAPI parseAPI symtabAPI instructionAPI gflags glog) 
endif()

install(TARGETS InstrumentMain DESTINATION bin)
