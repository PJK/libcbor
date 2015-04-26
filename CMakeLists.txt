cmake_minimum_required(VERSION 2.8)
project(libcbor)
include(CTest)

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -Wall -pedantic -g -ggdb -DDEBUG=true")
set(CMAKE_C_FLAGS_RELEASE "-O3 -Wall -pedantic -DNDEBUG")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11")

set(CMAKE_EXE_LINKER_FLAGS_DEBUG "-g")

enable_testing()

set(CTEST_MEMORYCHECK_COMMAND "/usr/bin/valgrind")
set(MEMORYCHECK_COMMAND_OPTIONS "")

add_custom_target(coverage
                  erb ${CMAKE_CURRENT_SOURCE_DIR}/test/stream_expectations.c.erb > ${CMAKE_CURRENT_SOURCE_DIR}/test/stream_expectations.c
                  COMMAND ctest
                  COMMAND lcov --capture --directory . --output-file coverage.info
                  COMMAND genhtml coverage.info --highlight --legend --output-directory coverage_html
                  COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_CURRENT_SOURCE_DIR}/test/stream_expectations.c
                  COMMAND echo "Coverage report ready: file://${CMAKE_CURRENT_BINARY_DIR}/coverage_html/index.html")

include_directories(src)


option(CUSTOM_ALLOC "Custom, dynamically defined allocator support" OFF)
if(CUSTOM_ALLOC)
    add_definitions(-DCBOR_CUSTOM_ALLOC=1)
endif(CUSTOM_ALLOC)

option(COVERAGE "Enable code coverage instrumentation" OFF)
if(COVERAGE)
    message("Configuring code coverage instrumentation")
    if(NOT CMAKE_C_COMPILER EQUAL "gcc")
        message(WARNING "Gcov instrumentation only works with GCC")
    endif()
    # https://gcc.gnu.org/onlinedocs/gcc/Debugging-Options.html
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g --coverage")
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -g --coverage")
endif(COVERAGE)

subdirs(src)
subdirs(demo)
subdirs(test)
subdirs(bench)
