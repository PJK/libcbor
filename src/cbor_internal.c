#include "cbor_internal.h"
#include <assert.h>
#include <string.h>

// TODO asserts
// TODO check mallocs

bool _cbor_assert_avail_bytes(size_t num,
							  size_t source_size,
							  struct cbor_load_result * result)
{
	if (source_size < num) {
		result->error = (struct cbor_error) { 0, CBOR_ERR_NOTENOUGHDATA };
		return false;
	}
	return true;
}

uint8_t _cbor_load_uint8(const unsigned char * source)
{
	return (uint8_t)*source;
}

uint16_t _cbor_load_uint16(const unsigned char * source)
{
	return ((uint16_t)*(source + 0) << 8) +
			(uint8_t )*(source + 1);
}

uint32_t _cbor_load_uint32(const unsigned char * source)
{
	return ((uint32_t)*(source + 0) << 0x18) +
		   ((uint32_t)*(source + 1) << 0x10) +
		   ((uint16_t)*(source + 2) << 0x08) +
			(uint8_t )*(source + 3);
}

uint64_t _cbor_load_uint64(const unsigned char * source)
{
	return ((uint64_t)*(source + 0) << 0x38) +
		   ((uint64_t)*(source + 1) << 0x30) +
		   ((uint64_t)*(source + 2) << 0x28) +
		   ((uint64_t)*(source + 3) << 0x20) +
		   ((uint32_t)*(source + 4) << 0x18) +
		   ((uint32_t)*(source + 5) << 0x10) +
		   ((uint16_t)*(source + 6) << 0x08) +
		    (uint8_t )*(source + 7);
}

unsigned char * _cbor_new_int8_data()
{
	unsigned char * data = malloc(1);
	return data;
}

unsigned char * _cbor_new_int16_data()
{
	unsigned char * data = malloc(2);
	return data;
}

unsigned char * _cbor_new_int32_data()
{
	unsigned char * data = malloc(4);
	return data;
}

unsigned char * _cbor_new_int64_data()
{
	unsigned char * data = malloc(8);
	return data;
}


void _cbor_handle_load_uint8(const unsigned char * source,
							 size_t source_size,
							 cbor_item_t * item,
							 struct cbor_load_result * result)
{
	if (!_cbor_assert_avail_bytes(1, source_size, result))
		return;
	result->read += 1;
	item->metadata.int_metadata = (struct _cbor_int_metadata) { CBOR_INT_8 };
	item->data = malloc(1);
	cbor_set_uint8(item, _cbor_load_uint8(source));
}


void _cbor_handle_load_uint16(const unsigned char * source,
						size_t source_size,
						cbor_item_t * item,
						struct cbor_load_result * result)
{
	if (!_cbor_assert_avail_bytes(2, source_size, result))
		return;
	result->read += 2;
	item->metadata.int_metadata = (struct _cbor_int_metadata) { CBOR_INT_16 };
	item->data = malloc(2);
	cbor_set_uint16(item, _cbor_load_uint16(source));
}

void _cbor_handle_load_uint32(const unsigned char * source,
							  size_t source_size,
							  cbor_item_t * item,
							  struct cbor_load_result * result)
{
	if (!_cbor_assert_avail_bytes(4, source_size, result))
		return;
	result->read += 4;
	item->metadata.int_metadata = (struct _cbor_int_metadata) { CBOR_INT_32 };
	item->data = malloc(4);
	cbor_set_uint32(item, _cbor_load_uint32(source));
}


void _cbor_handle_load_uint64(const unsigned char * source,
							  size_t source_size,
							  cbor_item_t * item,
							  struct cbor_load_result * result)
{
	if (!_cbor_assert_avail_bytes(8, source_size, result))
		return;
	result->read += 8;
	item->metadata.int_metadata = (struct _cbor_int_metadata) { CBOR_INT_64 };
	item->data = malloc(8);
	cbor_set_uint64(item, _cbor_load_uint64(source));
}

// TODO what if size_t < uint64_t ?
// TODO chunked decoding
// TODO what
void _cbor_handle_load_bytestring(const unsigned char * source,
								  size_t source_size,
								  size_t length,
								  cbor_item_t * item,
								  struct cbor_load_result * result)
{
	if (!_cbor_assert_avail_bytes(length, source_size, result))
		return;
	result->read += length;
	item->data = malloc(length);
	item->metadata.bytestring_metadata = (struct _cbor_bytestring_metadata) { length, _CBOR_BYTESTRING_METADATA_DEFINITE };
	memcpy(item->data, source, length);
}
