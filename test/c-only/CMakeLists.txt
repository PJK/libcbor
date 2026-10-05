cmake_minimum_required(VERSION 3.12)
project(libcbor_c_only_test LANGUAGES C)

# Model a C-only toolchain even on CI machines with a C++ compiler installed.
set(CMAKE_CXX_COMPILER "${CMAKE_CURRENT_BINARY_DIR}/cxx-not-available"
    CACHE FILEPATH "Unavailable C++ compiler for this test" FORCE)
set(WITH_TESTS OFF CACHE BOOL "Do not build the optional C++ test" FORCE)
add_subdirectory(../.. libcbor)

get_property(enabled_languages GLOBAL PROPERTY ENABLED_LANGUAGES)
if("CXX" IN_LIST enabled_languages)
  message(FATAL_ERROR "Building libcbor without tests must not require C++")
endif()

add_executable(c_only_linkage ../linkage/main.c)
target_link_libraries(c_only_linkage PRIVATE libcbor::libcbor)
enable_testing()
add_test(NAME c_only_linkage COMMAND c_only_linkage)
