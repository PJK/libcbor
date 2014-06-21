#include "cbor_internal.h"
#include <assert.h>

// TODO asserts

void assert_avail_bytes(size_t num,
						size_t source_size,
						struct cbor_load_result * result)
{
	if (source_size < num) {
		result->error = (struct cbor_error) { 0, CBOR_ERR_NOTENOUGHDATA };
	}
}

void handle_load_uint8(const unsigned char * source,
					   size_t source_size,
					   cbor_item_t * item,
					   struct cbor_load_result * result)
{
	assert_avail_bytes(1, source_size, result);
	result->read += 1;
	item->data = malloc(METADATA_WIDTH + INT_METADATA_WIDTH + 1);
	*(cbor_int_width *)&item->data[METADATA_WIDTH] = CBOR_INT_8;
	cbor_set_uint8(item, (uint8_t)*source);
}

void handle_load_uint16(const unsigned char * source,
						size_t source_size,
						cbor_item_t * item,
						struct cbor_load_result * result)
{
	assert_avail_bytes(2, source_size, result);
	result->read += 2;
	item->data = malloc(METADATA_WIDTH + INT_METADATA_WIDTH + 2);
	*(cbor_int_width *)&item->data[METADATA_WIDTH] = CBOR_INT_16;
	/* TODO endianness */
	uint16_t value = ((uint16_t)*(source + 0) << 8) +
					  (uint8_t )*(source + 1);
	cbor_set_uint16(item, value);
}

void handle_load_uint32(const unsigned char * source,
					    size_t source_size,
					    cbor_item_t * item,
					    struct cbor_load_result * result)
{
	assert_avail_bytes(4, source_size, result);
	result->read += 4;
	item->data = malloc(METADATA_WIDTH + INT_METADATA_WIDTH + 4);
	*(cbor_int_width *)&item->data[METADATA_WIDTH] = CBOR_INT_32;
	/* TODO endianness */
	uint32_t value = ((uint32_t)*(source + 0) << 0x18) +
					 ((uint32_t)*(source + 1) << 0x10) +
					 ((uint16_t)*(source + 2) << 0x08) +
					  (uint8_t )*(source + 3);
	cbor_set_uint32(item, value);
}


void handle_load_uint64(const unsigned char * source,
						size_t source_size,
						cbor_item_t * item,
						struct cbor_load_result * result)
{
	assert_avail_bytes(8, source_size, result);
	result->read += 8;
	item->data = malloc(METADATA_WIDTH + INT_METADATA_WIDTH + 8);
	*(cbor_int_width *)&item->data[METADATA_WIDTH] = CBOR_INT_64;
	/* TODO endianness */
	uint64_t value = ((uint64_t)*(source + 0) << 0x38) +
					 ((uint64_t)*(source + 1) << 0x30) +
					 ((uint64_t)*(source + 2) << 0x28) +
					 ((uint64_t)*(source + 3) << 0x20) +
					 ((uint32_t)*(source + 4) << 0x18) +
					 ((uint32_t)*(source + 5) << 0x10) +
					 ((uint16_t)*(source + 6) << 0x08) +
					  (uint8_t )*(source + 7);

	cbor_set_uint64(item, value);
}
