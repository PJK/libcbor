#include "cbor.h"
#include "cbor_internal.h"
#include "magic.h"
#include <assert.h>
#include <string.h>

size_t cbor_serialize(const cbor_item_t * item, unsigned char * buffer, size_t buffer_size)
{
	switch (cbor_typeof(item)) {
	case CBOR_TYPE_UINT:
		return cbor_serialize_uint(item, buffer, buffer_size);
	case CBOR_TYPE_NEGINT:
		return cbor_serialize_negint(item, buffer, buffer_size);
	case CBOR_TYPE_BYTESTRING:
		return cbor_serialize_bytestring(item, buffer, buffer_size);
	case CBOR_TYPE_STRING:
		return cbor_serialize_string(item, buffer, buffer_size);
	case CBOR_TYPE_ARRAY:
		return cbor_serialize_array(item, buffer, buffer_size);
	case CBOR_TYPE_MAP:
		return cbor_serialize_map(item, buffer, buffer_size);
	case CBOR_TYPE_TAG:
		return cbor_serialize_tag(item, buffer, buffer_size);
	case CBOR_TYPE_FLOAT_CTRL:
		return cbor_serialize_float_ctrl(item, buffer, buffer_size);
	}
}

size_t cbor_serialize_uint(const cbor_item_t * item, unsigned char * buffer, size_t buffer_size)
{
	assert(cbor_isa_uint(item));
	switch (cbor_int_get_width(item)) {
	case CBOR_INT_8:
		return cbor_encode_uint8(cbor_get_uint8(item), buffer, buffer_size);
	case CBOR_INT_16:
		return cbor_encode_uint16(cbor_get_uint16(item), buffer, buffer_size);
	case CBOR_INT_32:
		return cbor_encode_uint32(cbor_get_uint32(item), buffer, buffer_size);
	case CBOR_INT_64:
		return cbor_encode_uint64(cbor_get_uint64(item), buffer, buffer_size);
	}
}

size_t cbor_serialize_negint(const cbor_item_t * item, unsigned char * buffer, size_t buffer_size)
{
	assert(cbor_isa_negint(item));
	switch (cbor_int_get_width(item)) {
	case CBOR_INT_8:
		return cbor_encode_negint8(cbor_get_uint8(item), buffer, buffer_size);
	case CBOR_INT_16:
		return cbor_encode_negint16(cbor_get_uint16(item), buffer, buffer_size);
	case CBOR_INT_32:
		return cbor_encode_negint32(cbor_get_uint32(item), buffer, buffer_size);
	case CBOR_INT_64:
		return cbor_encode_negint64(cbor_get_uint64(item), buffer, buffer_size);
	}
}

size_t cbor_serialize_bytestring(const cbor_item_t * item, unsigned char * buffer, size_t buffer_size)
{
	assert(cbor_isa_bytestring(item));
	if (cbor_string_is_definite(item)) {
		size_t length = cbor_bytestring_length(item);
		size_t written = cbor_encode_bytestring_start(length, buffer, buffer_size);
		if (written && (buffer_size - written >= length)) {
			memcpy(buffer + written, cbor_bytestring_handle(item), length);
			return written + length;
		} else
			return 0;
	} else {
		size_t chunk_count = cbor_bytestring_chunk_count(item);
		size_t written = cbor_encode_indef_bytestring_start(buffer, buffer_size);
		// TODO stop this earlier if a chunk failed
		cbor_item_t ** chunks = cbor_bytestring_chunks_handle(item);
		for (size_t i = 0; i < chunk_count; i++) {
			written += cbor_serialize_bytestring(chunks[i], buffer, buffer_size - written);
		}
		written += cbor_encode_break(buffer, buffer_size - written);
		return written;
	}
}

size_t cbor_serialize_string(const cbor_item_t * item, unsigned char * buffer, size_t buffer_size)
{
	assert(cbor_isa_string(item));
}

size_t cbor_serialize_array(const cbor_item_t * item, unsigned char * buffer, size_t buffer_size)
{
	size_t size, written;
	assert(cbor_isa_array(item));
	if (cbor_array_is_definite(item)) {
		size = cbor_array_size(item);
		written = cbor_encode_array_start(size, buffer, buffer_size);
	} else {

	}
	return written;
}

size_t cbor_serialize_map(const cbor_item_t * item, unsigned char * buffer, size_t buffer_size)
{
	assert(cbor_isa_map(item));
}

size_t cbor_serialize_tag(const cbor_item_t * item, unsigned char * buffer, size_t buffer_size)
{
	assert(cbor_isa_tag(item));
}

size_t cbor_serialize_float_ctrl(const cbor_item_t * item, unsigned char * buffer, size_t buffer_size)
{
	assert(cbor_isa_float_ctrl(item));
}

