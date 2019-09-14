option(CUSTOM_DYNINST "use custom dyninst build" OFF)
find_package(Boost REQUIRED)
find_package(gflags REQUIRED)
find_package(glog REQUIRED)

add_executable(InstrumentMain InstrumentMain.cpp 
                              InstrumentClient.cpp)

if (CUSTOM_DYNINST MATCHES "ON")
  find_package(Dyninst REQUIRED dyninstAPI symtabAPI instructionAPI parseAPI)
  target_link_libraries(InstrumentMain dyninstAPI parseAPI symtabAPI instructionAPI gflags glog) 
else()
  find_package(Dyninst REQUIRED dyninstAPI symtabAPI instructionAPI parseAPI)
  target_link_libraries(InstrumentMain dyninstAPI parseAPI symtabAPI instructionAPI gflags glog) 
endif()

install(TARGETS InstrumentMain DESTINATION bin)
