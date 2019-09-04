find_package(Boost REQUIRED)
find_package(Dyninst REQUIRED dyninstAPI symtabAPI instructionAPI parseAPI)
find_package(glog REQUIRED)

include_directories(${DYNINST_INCLUDE_DIR})

add_executable(InstrumentMain InstrumentMain.cpp 
                              InstrumentClient.cpp)

#add_dependencies(InstrumentMain DyninstAPI ParseAPI SymtabAPI InstructionAPI glog)

target_link_libraries(InstrumentMain dyninstAPI parseAPI symtabAPI instructionAPI glog) 

install(TARGETS InstrumentMain DESTINATION bin/client)
