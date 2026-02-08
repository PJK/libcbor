cmake_minimum_required(VERSION 3.5)

project(libcbor VERSION 0.13.0 LANGUAGES C CXX)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
  "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules/")
include(CTest)
include(GNUInstallDirs) # Provides CMAKE_INSTALL_ variables

option(CMAKE_SKIP_INSTALL_ALL_DEPENDENCY
  "cmake --build --target install does not depend on cmake --build" true)
option(BUILD_SHARED_LIBS "Build as a shared library" false)

include(CheckIncludeFiles)

include(TestBigEndian)
test_big_endian(BIG_ENDIAN)

option(CBOR_PRETTY_PRINTER "Include a pretty-printing routine" ON)
set(CBOR_BUFFER_GROWTH
  "2"
  CACHE STRING "Factor for buffer growth & shrinking")
set(CBOR_MAX_STACK_SIZE
  "2048"
  CACHE STRING "maximum size for decoding context stack")

option(WITH_TESTS "[TEST] Build unit tests (requires CMocka)" OFF)

option(WITH_EXAMPLES "Build examples" ON)

option(HUGE_FUZZ "[TEST] Fuzz through 8GB of data in the test.\
       Do not use with memory instrumentation!"  OFF)

option(SANE_MALLOC
  "[TEST] Assume that malloc will not allocate multi-GB blocks.\
       Tests only, platform specific"  OFF)

option(PRINT_FUZZ "[TEST] Print the fuzzer input" OFF)

option(SANITIZE "Enable ASan & a few compatible sanitizers in Debug mode" ON)

set(CPACK_GENERATOR "DEB" "TGZ" "RPM")
find_program(DPKG_CMD dpkg)
if(DPKG_CMD)
  execute_process(COMMAND ${DPKG_CMD} --print-architecture
    OUTPUT_VARIABLE CPACK_DEBIAN_PACKAGE_ARCHITECTURE
    OUTPUT_STRIP_TRAILING_WHITESPACE)
endif()
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Pavel Kalvoda")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6")
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})

include(CPack)

#
# Configure compilation flags and language features
#

include(CheckCSourceCompiles)

check_c_source_compiles("
  #include <stdio.h>
  [[nodiscard]] int f(void) { return 42; }
  int main(void) { return f(); }
"  HAS_NODISCARD_ATTRIBUTE)

if(HAS_NODISCARD_ATTRIBUTE)
  message(STATUS "[[nodiscard]] is supported.")
  # Assume that if we have [[nodiscard]], we have some C23 support. May fail.
  if(NOT DEFINED CMAKE_C_STANDARD)
    message(STATUS "Switching to C23-like mode. To prevent this, pass -DCMAKE_C_STANDARD explicitly.")
    # On Clang 16, this is resolved to -std=c2x
    set(CMAKE_C_STANDARD 23 CACHE STRING "C language standard")
  endif()
endif()

if(MINGW)
  # https://github.com/PJK/libcbor/issues/13
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99")
elseif(NOT MSVC)
  # Default to C99
  if(NOT DEFINED CMAKE_C_STANDARD)
    set(CMAKE_C_STANDARD 99 CACHE STRING "C language standard")
  endif()
endif()

# CMAKE_C_STANDARD set above
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

if(MSVC)
  # This just doesn't work right --
  # https://msdn.microsoft.com/en-us/library/5ft82fed.aspx
  set(CBOR_RESTRICT_SPECIFIER "")
else()
  set(CBOR_RESTRICT_SPECIFIER "restrict")

  # Sanitizer flags must stay in CMAKE_C_FLAGS because they need to be passed
  # to both the compiler and linker, and target_link_options requires CMake 3.13
  if(SANITIZE)
    set(CMAKE_C_FLAGS_DEBUG
      "${CMAKE_C_FLAGS_DEBUG} \
            -fsanitize=undefined -fsanitize=address \
            -fsanitize=bounds -fsanitize=alignment")
  endif()

  set(CMAKE_EXE_LINKER_FLAGS_DEBUG "-g")
endif()

add_library(cbor_project_options INTERFACE)

if(MINGW)
  target_compile_options(cbor_project_options INTERFACE -std=gnu99)
elseif(NOT MSVC)
  target_compile_options(cbor_project_options INTERFACE -pedantic)
endif()

if(MSVC)
  target_compile_options(cbor_project_options INTERFACE
    $<$<CONFIG:Debug>:/sdl>)
else()
  target_compile_options(cbor_project_options INTERFACE
    $<$<CONFIG:Debug>:-O0 -Wall -Wextra -g -ggdb>
    $<$<CONFIG:Release>:-O3 -Wall -Wextra>)
  target_compile_definitions(cbor_project_options INTERFACE
    $<$<CONFIG:Debug>:DEBUG=true>)
endif()

include(CheckTypeSize)
check_type_size("size_t" SIZEOF_SIZE_T)
if(SIZEOF_SIZE_T LESS 8)
  message(
    WARNING
    "Your size_t is less than 8 bytes. \
      Decoding of huge items that would exceed the memory address space \
      will always fail. Consider implementing a custom streaming \
      decoder if you need to deal with huge items.")
else()
  set(HAVE_EIGHT_BYTE_SIZE_T TRUE)
endif()

check_c_source_compiles("
    int main() {
        __builtin_unreachable();
        return 0;
    }
"  HAS_BUILTIN_UNREACHABLE)

# CMake >= 3.9.0 enables LTO for GCC and Clang with INTERPROCEDURAL_OPTIMIZATION
# Policy CMP0069 enables this behavior when we set the minimum CMake version <
# 3.9.0 Checking for LTO support before setting INTERPROCEDURAL_OPTIMIZATION is
# mandatory with CMP0069 set to NEW.
set(LTO_SUPPORTED FALSE)
if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.9.0")
  cmake_policy(SET CMP0069 NEW)
  # Require LTO support to build libcbor with newer CMake versions
  include(CheckIPOSupported)
  check_ipo_supported(RESULT LTO_SUPPORTED)
endif()

if(LTO_SUPPORTED AND NOT DEFINED CMAKE_INTERPROCEDURAL_OPTIMIZATION)
  set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
endif()

if(LTO_SUPPORTED)
  message(
    STATUS
    "LTO is supported and CMAKE_INTERPROCEDURAL_OPTIMIZATION=${CMAKE_INTERPROCEDURAL_OPTIMIZATION}"
  )
else()
  message(STATUS "LTO is not supported")
endif()

#
# Testing and validation
#

enable_testing()

set(CTEST_MEMORYCHECK_COMMAND "/usr/bin/valgrind")
set(MEMORYCHECK_COMMAND_OPTIONS
  "--tool=memcheck --track-origins=yes --leak-check=full --error-exitcode=1")

# TODO: Rename the option to be in line with the rest (CBOR_...)
option(COVERAGE "Enable code coverage instrumentation" OFF)
if(COVERAGE)
  add_custom_target(
    coverage
    COMMAND ctest
    COMMAND lcov --capture --directory . --output-file coverage.info
    COMMAND genhtml coverage.info --highlight --legend --output-directory
    coverage_html
    COMMAND
    echo
    "Coverage report ready: ${CMAKE_CURRENT_BINARY_DIR}/coverage_html/index.html"
    COMMENT "Generate coverage report using the GNU toolchain"
  )

  add_custom_target(
    llvm-coverage
    COMMAND cmake --build . --parallel
    COMMAND rm -rf coverage_profiles
    COMMAND mkdir coverage_profiles
    COMMAND
    bash -c
    [[ for TEST in $(ls test/*_test); do LLVM_PROFILE_FILE="coverage_profiles/$(basename -- ${TEST}).profraw" ./${TEST}; done ]]
    # VERBATIM makes escaping working, but breaks shell expansions, so we need to
    # explicitly use bash
    COMMAND
    bash -c
    [[ llvm-profdata merge -sparse $(ls coverage_profiles/*.profraw) -o coverage_profiles/combined.profdata ]]
    COMMAND
    bash -c
    [[ llvm-cov show -instr-profile=coverage_profiles/combined.profdata test/*_test -format=html > coverage_profiles/report.html ]]
    COMMAND
    bash -c
    [[ llvm-cov report -instr-profile=coverage_profiles/combined.profdata test/*_test ]]
    COMMAND
    echo
    "Coverage report ready: ${CMAKE_CURRENT_BINARY_DIR}/coverage_profiles/report.html"
    VERBATIM
    COMMENT "Generate coverage report using the LLVM toolchain")

  message("Configuring code coverage instrumentation")
  if(CMAKE_C_COMPILER_ID MATCHES "GNU")
    # https://gcc.gnu.org/onlinedocs/gcc/Debugging-Options.html
    set(CMAKE_C_FLAGS
      "${CMAKE_C_FLAGS} -g -fprofile-arcs -ftest-coverage --coverage")
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG
      "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -g -fprofile-arcs -ftest-coverage --coverage"
    )
  elseif(CMAKE_C_COMPILER_ID MATCHES "Clang")
    set(CMAKE_C_FLAGS
      "${CMAKE_C_FLAGS} -fprofile-instr-generate -fcoverage-mapping")
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG
      "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -fprofile-instr-generate")
  else()
    message(
      WARNING
      "Code coverage build not implemented for compiler ${CMAKE_C_COMPILER_ID}"
    )
  endif()
endif()

#
# Configure build and targets
#

# We want to generate configuration.h from the template and make it so that it
# is accessible using the same path during both library build and installed
# header use, without littering the source dir.
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/cbor/configuration.h.in
  ${PROJECT_BINARY_DIR}/cbor/configuration.h)
install(FILES ${PROJECT_BINARY_DIR}/cbor/configuration.h
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/cbor)

add_subdirectory(src)

if(WITH_TESTS)
  add_subdirectory(test)
endif()

if(WITH_EXAMPLES)
  add_subdirectory(examples)
endif()
