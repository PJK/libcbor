file(GLOB TESTS "*_test.c")

find_package(CMocka REQUIRED)

get_cmake_property(_variableNames VARIABLES)
foreach (_variableName ${_variableNames})
    message(STATUS "${_variableName}=${${_variableName}}")
endforeach()

foreach (TEST ${TESTS})
    string(REGEX REPLACE ".*/([^/]+).c" "\\1" NAME ${TEST})
    message("Adding test ${NAME}")
    add_executable(${NAME} "${NAME}.c" assertions.c stream_expectations.c)
    target_link_libraries(${NAME} ${CMOCKA_LIBRARIES})
    target_link_libraries(${NAME} cbor)
    target_include_directories(${NAME} PUBLIC ${CMocka_INCLUDE_DIR})
    add_test(${NAME} ${NAME})
    add_dependencies(coverage ${NAME})
endforeach (TEST)
