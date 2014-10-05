set(SOURCES cbor.c cbor_internal.c cbor_stack.c cbor_unicode.c)

add_library(cbor ${SOURCES})
add_library(cbor_shared SHARED ${SOURCES})

set_target_properties(cbor_shared PROPERTIES OUTPUT_NAME cbor)
