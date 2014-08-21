file(GLOB BENCHES "*.c")

foreach(BENCH ${BENCHES})
  string(REGEX REPLACE ".*/([^/]+).c" "\\1" NAME ${BENCH})
  add_executable(${NAME} "${NAME}.c")
  target_link_libraries(${NAME} jansson)
  target_link_libraries(${NAME} cbor)
endforeach(BENCH)
