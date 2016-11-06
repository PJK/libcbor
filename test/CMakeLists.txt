file(GLOB TESTS "*_test.c")

find_package(CMocka REQUIRED)

include_directories(${CMocka_INCLUDE_DIR})

foreach (TEST ${TESTS})
    string(REGEX REPLACE ".*/([^/]+).c" "\\1" NAME ${TEST})
    message("Adding test ${NAME}")
    add_executable(${NAME} "${NAME}.c" assertions.c stream_expectations.c)
    target_link_libraries(${NAME} ${CMOCKA_LIBRARIES})
    target_link_libraries(${NAME} cbor)
    add_test(${NAME} ${NAME})
    add_dependencies(coverage ${NAME})
endforeach (TEST)
