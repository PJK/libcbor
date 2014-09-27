file(GLOB TESTS "*_test.c")

add_custom_command(
  PRE_BUILD
  OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/stream_expectations.c
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMAND erb stream_expectations.c.erb > stream_expectations.c
  COMMENT "Creating templated file ${CMAKE_CURRENT_SOURCE_DIR}/stream_expectations.c"
)

foreach(TEST ${TESTS})
  string(REGEX REPLACE ".*/([^/]+).c" "\\1" NAME ${TEST})
  message("Adding test ${NAME}")
  add_executable(${NAME} "${NAME}.c" assertions.c stream_expectations.c)
  target_link_libraries(${NAME} cmocka)
  target_link_libraries(${NAME} cbor)
  add_test(${NAME} ${NAME})

  add_custom_command(TARGET ${NAME} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_CURRENT_SOURCE_DIR}/stream_expectations.c
      VERBATIM
      COMMENT "Removing templated file ${CMAKE_CURRENT_SOURCE_DIR}/stream_expectations.c"
  )
endforeach(TEST)


