cmake_minimum_required(VERSION 2.8)
project(libcbor)
include(CTest)

include(CheckIncludeFiles)

check_include_files(endian.h HAVE_ENDIAN_H)
if(HAVE_ENDIAN_H)
    # BSD source needed or else GNU-y extensions will not be visible
    add_definitions(-D_BSD_SOURCE=1)
    add_definitions(-DHAVE_ENDIAN_H=1)
endif()

SET(CPACK_GENERATOR "DEB" "TGZ" "RPM")
set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "Pavel Kalvoda")
SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6")
SET(CPACK_PACKAGE_VERSION_MAJOR "0")
SET(CPACK_PACKAGE_VERSION_MINOR "2")
SET(CPACK_PACKAGE_VERSION_PATCH "1")

include(CPack)

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -Wall -pedantic -g -ggdb -DDEBUG=true")
set(CMAKE_C_FLAGS_RELEASE "-O3 -flto -Wall -pedantic -DNDEBUG")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")

set(CMAKE_EXE_LINKER_FLAGS_DEBUG "-g")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "-flto")


include(CheckTypeSize)
check_type_size("size_t" SIZEOF_SIZE_T)
if (SIZEOF_SIZE_T LESS 8)
    message(WARNING "Your size_t is less than 8 bytes. Long items with 64b length specifiers might not work as expected. Make sure to run the tests!")
endif()

enable_testing()

set(CTEST_MEMORYCHECK_COMMAND "/usr/bin/valgrind")
set(MEMORYCHECK_COMMAND_OPTIONS "--tool=memcheck --track-origins=yes --leak-check=full --error-exitcode=1")

add_custom_target(coverage
                  COMMAND ctest
                  COMMAND lcov --capture --directory . --output-file coverage.info
                  COMMAND genhtml coverage.info --highlight --legend --output-directory coverage_html
                  COMMAND echo "Coverage report ready: file://${CMAKE_CURRENT_BINARY_DIR}/coverage_html/index.html")
include_directories(src)


option(CUSTOM_ALLOC "Custom, dynamically defined allocator support" OFF)
if(CUSTOM_ALLOC)
    add_definitions(-DCBOR_CUSTOM_ALLOC=1)
endif(CUSTOM_ALLOC)

option(HUGE_FUZZ "Fuzz through 8GB of data in the test. Do not use with memory instrumentation!" OFF)
if(HUGE_FUZZ)
    add_definitions(-DCBOR_HUGE_FUZZ=1)
endif(HUGE_FUZZ)

option(SANE_MALLOC "Assume that malloc will not allocate multi-GB blocks. Tests only, platform specific" OFF)
if(SANE_MALLOC)
    add_definitions(-DCBOR_ASSUME_SANE_MALLOC=1)
endif(SANE_MALLOC)

option(COVERAGE "Enable code coverage instrumentation" OFF)
if(COVERAGE)
    message("Configuring code coverage instrumentation")
    if(NOT CMAKE_C_COMPILER MATCHES "gcc")
        message(WARNING "Gcov instrumentation only works with GCC")
    endif()
    # https://gcc.gnu.org/onlinedocs/gcc/Debugging-Options.html
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -fprofile-arcs -ftest-coverage --coverage")
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -g -fprofile-arcs -ftest-coverage --coverage")
endif(COVERAGE)

option(PRETTY_PRINTER "Include a debug pretty-printing routine" ON)
if(PRETTY_PRINTER)
    add_definitions(-DPRETTY_PRINTER=1)
endif(PRETTY_PRINTER)

set(BUFFER_GROWTH "2" CACHE STRING "Factor for buffer growth & shrinking")
add_definitions(-DCBOR_BUFFER_GROWTH=${BUFFER_GROWTH})


subdirs(src)
subdirs(test)
subdirs(examples)
