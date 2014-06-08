cmake_minimum_required (VERSION 2.8)
project(libcbor)

set(CMAKE_C_FLAGS "-std=c99")

set(CMAKE_C_FLAGS_DEBUG "-O0 -Wall -pedantic -g -ggdb") 

include_directories(src)
subdirs(src)
subdirs(demo)
