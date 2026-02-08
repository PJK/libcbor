file(GLOB TESTS "*_test.c")

find_package(CMocka REQUIRED)

message(STATUS "CMocka vars: ${CMOCKA_LIBRARIES} ${CMOCKA_INCLUDE_DIR}")

find_library(MATH_LIBRARY m)

check_include_file("execinfo.h" HAS_EXECINFO)

foreach(test_file ${TESTS})
  string(REGEX REPLACE ".*/([^/]+).c" "\\1" NAME ${test_file})
  message("Adding test ${NAME}")
  add_executable(${NAME} "${NAME}.c" assertions.c stream_expectations.c
    test_allocator.c)
  target_link_libraries(${NAME} ${CMOCKA_LIBRARIES})
  target_link_libraries(${NAME} cbor)
  if(MATH_LIBRARY)
    target_link_libraries(${NAME} ${MATH_LIBRARY})
  endif()
  target_include_directories(${NAME} PUBLIC ${CMOCKA_INCLUDE_DIR})
  target_compile_definitions(${NAME} PRIVATE
    $<$<BOOL:${HUGE_FUZZ}>:HUGE_FUZZ>
    $<$<BOOL:${SANE_MALLOC}>:SANE_MALLOC>
    $<$<BOOL:${PRINT_FUZZ}>:PRINT_FUZZ>
    $<$<BOOL:${HAVE_EIGHT_BYTE_SIZE_T}>:EIGHT_BYTE_SIZE_T>)
  add_test(NAME ${NAME} COMMAND ${NAME})
  if(COVERAGE)
    add_dependencies(coverage ${NAME})
  endif()
endforeach()

add_executable(cpp_linkage_test cpp_linkage_test.cpp)
target_link_libraries(cpp_linkage_test cbor)
