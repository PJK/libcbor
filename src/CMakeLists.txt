add_library(cbor cbor.c cbor_internal.c)
add_library(cbor_shared SHARED cbor.c cbor_internal.c)

set_target_properties(cbor_shared PROPERTIES OUTPUT_NAME libcbor)