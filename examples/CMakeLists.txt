add_executable(readfile readfile.c)
target_link_libraries(readfile cbor cbor_project_options)

add_executable(create_items create_items.c)
target_link_libraries(create_items cbor cbor_project_options)

add_executable(streaming_parser streaming_parser.c)
target_link_libraries(streaming_parser cbor cbor_project_options)

add_executable(streaming_array streaming_array.c)
target_link_libraries(streaming_array cbor cbor_project_options)

add_executable(sort sort.c)
target_link_libraries(sort cbor cbor_project_options)

add_executable(hello hello.c)
target_link_libraries(hello cbor cbor_project_options)

add_executable(cbor_sequence cbor_sequence.c)
target_link_libraries(cbor_sequence cbor cbor_project_options)

add_executable(crash_course crash_course.c)
target_link_libraries(crash_course cbor cbor_project_options)

find_package(CJSON)

if(CJSON_FOUND)
  add_executable(cjson2cbor cjson2cbor.c)
  target_include_directories(cjson2cbor PUBLIC ${CJSON_INCLUDE_DIRS})
  target_link_libraries(cjson2cbor cbor cbor_project_options ${CJSON_LIBRARY})

  add_executable(cbor2cjson cbor2cjson.c)
  target_include_directories(cbor2cjson PUBLIC ${CJSON_INCLUDE_DIRS})
  target_link_libraries(cbor2cjson cbor cbor_project_options ${CJSON_LIBRARY})
endif()

file(COPY data DESTINATION .)
