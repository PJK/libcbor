#include "cbor.h"
#include "cbor_internal.h"
#include "magic.h"

size_t cbor_serialize(cbor_item_t *, unsigned char *, size_t);
size_t cbor_serialize_uint(cbor_item_t *, unsigned char *, size_t);
size_t cbor_serialize_negint(cbor_item_t *, unsigned char *, size_t);
size_t cbor_serialize_bytestring(cbor_item_t *, unsigned char *, size_t);
size_t cbor_serialize_string(cbor_item_t *, unsigned char *, size_t);
size_t cbor_serialize_array(cbor_item_t *, unsigned char *, size_t);
size_t cbor_serialize_map(cbor_item_t *, unsigned char *, size_t);
size_t cbor_serialize_tag(cbor_item_t *, unsigned char *, size_t);
size_t cbor_serialize_float_ctrl(cbor_item_t *, unsigned char *, size_t);
