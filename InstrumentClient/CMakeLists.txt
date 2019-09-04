find_package(Boost REQUIRED)
find_package(Dyninst REQUIRED SymtabAPI InstructionAPI ParseAPI)

include_directories(${DYNINST_INCLUDE_DIR})

add_executable(InstrumentMain InstrumentMain.cpp 
                              InstrumentClient.cpp)

add_dependencies(InstrumentMain parseAPI symtabAPI instructionAPI)

target_link_libraries(InstrumentMain parseAPI symtabAPI instructionAPI) 

install(TARGETS InstrumentMain DESTINATION bin/client)
