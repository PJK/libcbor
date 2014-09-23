add_library(cbor cbor.c cbor_internal.c cbor_stack.c)
add_library(cbor_shared SHARED cbor.c cbor_internal.c cbor_stack.c)

set_target_properties(cbor_shared PROPERTIES OUTPUT_NAME libcbor)