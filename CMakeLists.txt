cmake_minimum_required(VERSION 3.10)
project(mini_sta CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

add_library(sta_core
  src/lib_parser.cpp
  src/netlist_parser.cpp
  src/timing_graph.cpp
  src/sta_engine.cpp
  src/report.cpp)
target_include_directories(sta_core PUBLIC src)
target_compile_options(sta_core PUBLIC -Wall -Wextra)

add_executable(sta src/main.cpp)
target_link_libraries(sta sta_core)

 enable_testing()
add_executable(test_sta tests/test_sta.cpp)
target_link_libraries(test_sta sta_core)
 add_test(NAME test_sta COMMAND test_sta)