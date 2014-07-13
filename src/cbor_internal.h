#ifndef CBOR_INTERNAL_
#define CBOR_INTERNAL_

#include "cbor.h"

// TODO hook these into the parser
typedef enum {
	_CBOR_METADATA_EMPTY	 = 0x00,
	_CBOR_METADATA_COMPLETE  = 0x01,
	_CBOR_METADATA_RESUMABLE = 0x02,	/* Parsing may be resumed */
	_CBOR_METADATA_MADE		 = 0x04 	/* Created by a cbor_make_<X> */
} _cbor_metadata;

typedef enum {
	_CBOR_BYTESTRING_METADATA_DEFINITE,
	_CBOR_BYTESTRING_METADATA_INDEFINITE
} _cbor_bytestring_type_metadata;

struct _cbor_bytestring_metadata {
	size_t length;
	_cbor_bytestring_type_metadata type;
};

struct _cbor_float_ctrl_metadata {
	cbor_float_width width;
	cbor_ctrl type;
};

#define _CBOR_METADATA_WIDTH sizeof(_cbor_metadata)

#define _CBOR_INT_METADATA_WIDTH sizeof(cbor_int_width)

#define _CBOR_BYTESTRING_METADATA_WIDTH sizeof(struct _cbor_bytestring_metadata)

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
