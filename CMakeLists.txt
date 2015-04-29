cmake_minimum_required(VERSION 2.8)
project(libcbor)
include(CTest)

SET(CPACK_GENERATOR "DEB" "TGZ" "RPM")
set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "Pavel Kalvoda")
SET(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6")

include(CPack)

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -Wall -pedantic -g -ggdb -DDEBUG=true")
set(CMAKE_C_FLAGS_RELEASE "-O3 -Wall -pedantic -DNDEBUG")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11")

set(CMAKE_EXE_LINKER_FLAGS_DEBUG "-g")

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

subdirs(src)
subdirs(demo)
subdirs(test)
