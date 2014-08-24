#ifndef CBOR_INTERNAL_
#define CBOR_INTERNAL_

#include "cbor.h"


#define _CBOR_METADATA_WIDTH sizeof(_cbor_metadata)

#define _CBOR_INT_METADATA_WIDTH sizeof(cbor_int_width)

#define _CBOR_BYTESTRING_METADATA_WIDTH sizeof(struct _cbor_bytestring_metadata)

#define _CBOR_ARRAY_METADATA_WIDTH sizeof(struct _cbor_array_metadata)

#define _CBOR_FLOAT_CTRL_METADATA_WIDTH sizeof(struct _cbor_float_ctrl_metadata)


bool _cbor_assert_avail_bytes(size_t num,
							  size_t source_size,
							  struct cbor_load_result * result);

/* Read the give uint from the given location, no questions asked */
uint8_t _cbor_load_uint8(const unsigned char * source);
uint16_t _cbor_load_uint16(const unsigned char * source);
uint32_t _cbor_load_uint32(const unsigned char * source);
uint64_t _cbor_load_uint64(const unsigned char * source);

unsigned char * _cbor_new_int8_data();
unsigned char * _cbor_new_int16_data();
unsigned char * _cbor_new_int32_data();
unsigned char * _cbor_new_int64_data();

void _cbor_handle_load_uint8(const unsigned char * source,
							 size_t source_size,
							 cbor_item_t * item,
							 struct cbor_load_result * result);

void _cbor_handle_load_uint16(const unsigned char * source,
							  size_t source_size,
							  cbor_item_t * item,
							  struct cbor_load_result * result);

void _cbor_handle_load_uint32(const unsigned char * source,
							  size_t source_size,
							  cbor_item_t * item,
							  struct cbor_load_result * result);

void _cbor_handle_load_uint64(const unsigned char * source,
							  size_t source_size,
							  cbor_item_t * item,
							  struct cbor_load_result * result);

void _cbor_handle_load_bytestring(const unsigned char * source,
								  size_t source_size,
								  size_t length,
								  cbor_item_t * item,
								  struct cbor_load_result * result);
#endif
