cmake_minimum_required (VERSION 2.8)
project(libcbor)
include(CTest)

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -Wall -pedantic -g -ggdb --coverage -DDEBUG=true")
set(CMAKE_C_FLAGS_RELEASE "-O3 -Wall -pedantic -DNDEBUG")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11")


set(CMAKE_EXE_LINKER_FLAGS "-g --coverage")

enable_testing()
add_custom_target(test ${CMAKE_CTEST_COMMAND} -V)

set(CTEST_MEMORYCHECK_COMMAND "/usr/bin/valgrind")
set(MEMORYCHECK_COMMAND_OPTIONS "")

include_directories(src)
subdirs(src)
subdirs(demo)
subdirs(test)
subdirs(bench)
