cmake_minimum_required(VERSION 3.5)
project(libcbor_linkage_test C)

find_package(libcbor REQUIRED)

add_executable(linkage_test main.c)
target_link_libraries(linkage_test libcbor::libcbor)

enable_testing()
add_test(NAME linkage_test COMMAND linkage_test)
