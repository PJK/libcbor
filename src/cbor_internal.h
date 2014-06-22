#ifndef CBOR_INTERNAL_
#define CBOR_INTERNAL_

#include "cbor.h"

typedef enum {
	_CBOR_METADATA_EMPTY	 = 0x00,
	_CBOR_METADATA_COMPLETE	 = 0x01,
	_CBOR_METADATA_RESUMABLE = 0x02,	/* Parsing may be resumed */
	_CBOR_METADATA_MADE		 = 0x04		/* Created by a cbor_make_<X> */
} metadata;

struct _cbor_bytesting_metadata {
	size_t length;
};

#define _CBOR_METADATA_WIDTH sizeof(metadata)

#define _CBOR_INT_METADATA_WIDTH sizeof(cbor_int_width)

#define _CBOR_BYTESTRING_METADATA_WIDTH sizeof(struct _cbor_bytesting_metadata)

bool _cbor_assert_avail_bytes(size_t num,
							  size_t source_size,
							  struct cbor_load_result * result);

/* Read the give uint from the given location, no questions asked */
uint8_t _cbor_load_uint8(const unsigned char * source);
uint16_t _cbor_load_uint16(const unsigned char * source);
uint32_t _cbor_load_uint32(const unsigned char * source);
uint64_t _cbor_load_uint64(const unsigned char * source);

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
								  uint64_t length,
								  cbor_item_t * item,
								  struct cbor_load_result * result);
#endif
