
add_executable(InstrumentMain InstrumentMain.cpp 
                              InstrumentClient.cpp)
add_dependencies(InstrumentMain parseAPI symtabAPI instructionAPI)

target_link_libraries(InstrumentMain parseAPI symtabAPI instructionAPI) 

install(TARGETS InstrumentMain DESTINATION bin/client)



