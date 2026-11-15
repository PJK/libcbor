# SPDX-FileCopyrightText: 2014-2026 Pavel Kalvoda <me@pavelkalvoda.com>
# SPDX-License-Identifier: MIT

# Builds the benchmark harness, either as part of libcbor (WITH_BENCHMARKS)
# or on its own against an installed libcbor, which bench/compare.sh uses to
# build one harness against two library versions:
#
#   cmake -S bench -B build-bench -DCMAKE_PREFIX_PATH=<libcbor prefix>

if(NOT TARGET cbor)
  cmake_minimum_required(VERSION 3.5)
  # Honor CMAKE_INTERPROCEDURAL_OPTIMIZATION (compare.sh --lto).
  if(POLICY CMP0069)
    cmake_policy(SET CMP0069 NEW)
  endif()
  project(libcbor_bench C)
  if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
  endif()
  find_package(libcbor REQUIRED)
  set(BENCH_LIBCBOR libcbor::libcbor)
else()
  set(BENCH_LIBCBOR cbor)
endif()

add_executable(libcbor_bench bench.c)
target_link_libraries(libcbor_bench ${BENCH_LIBCBOR})
target_compile_definitions(libcbor_bench PRIVATE
  BENCH_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data")
if(NOT MSVC)
  target_compile_options(libcbor_bench PRIVATE -Wall -Wextra)
endif()
