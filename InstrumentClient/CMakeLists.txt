find_package(Boost REQUIRED)
find_package(Dyninst REQUIRED dyninstAPI SymtabAPI InstructionAPI ParseAPI)
find_package(Folly REQUIRED)

include_directories(${DYNINST_INCLUDE_DIR})

add_executable(InstrumentMain InstrumentMain.cpp 
                              InstrumentClient.cpp)

add_dependencies(InstrumentMain dyninstAPI parseAPI symtabAPI instructionAPI)

target_link_libraries(InstrumentMain dyninstAPI parseAPI symtabAPI instructionAPI) 

install(TARGETS InstrumentMain DESTINATION bin/client)
