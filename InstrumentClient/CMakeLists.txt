find_package(Boost REQUIRED)
find_package(Dyninst REQUIRED dyninstAPI symtabAPI instructionAPI parseAPI)
find_package(gflags REQUIRED)
find_package(glog REQUIRED)

add_executable(InstrumentMain InstrumentMain.cpp 
                              InstrumentClient.cpp)

target_link_libraries(InstrumentMain dyninstAPI parseAPI symtabAPI instructionAPI gflags glog) 

install(TARGETS InstrumentMain DESTINATION bin)
