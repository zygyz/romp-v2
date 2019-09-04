find_package(Boost REQUIRED)
find_package(Dyninst REQUIRED DyninstAPI SymtabAPI InstructionAPI ParseAPI)
find_package(glog REQUIRED)

include_directories(${DYNINST_INCLUDE_DIR})

add_executable(InstrumentMain InstrumentMain.cpp 
                              InstrumentClient.cpp)

#add_dependencies(InstrumentMain DyninstAPI ParseAPI SymtabAPI InstructionAPI glog)

target_link_libraries(InstrumentMain DyninstAPI ParseAPI SymtabAPI InstructionAPI glog) 

install(TARGETS InstrumentMain DESTINATION bin/client)
