#ifndef CBOR_INTERNAL_
#define CBOR_INTERNAL_

#include "cbor.h"

typedef enum {
	METADATA_EMPTY	   = 0x00,
	METADATA_COMPLETE  = 0x01,
	METADATA_RESUMABLE = 0x02,	/* Parsing may be resumed */
	METADATA_MADE	   = 0x04	/* Created by a cbor_make_<X> */
} metadata;

#define METADATA_WIDTH sizeof(metadata)

#define INT_METADATA_WIDTH sizeof(cbor_int_width)

/* Read the give uint from the given location, no questions asked */
uint8_t load_uint8(const unsigned char * source);
uint16_t load_uint16(const unsigned char * source);
uint32_t load_uint32(const unsigned char * source);
uint64_t load_uint64(const unsigned char * source);

void handle_load_uint8(const unsigned char * source,
					   size_t source_size,
					   cbor_item_t * item,
					   struct cbor_load_result * result);

void handle_load_uint16(const unsigned char * source,
						size_t source_size,
						cbor_item_t * item,
						struct cbor_load_result * result);

void handle_load_uint32(const unsigned char * source,
						size_t source_size,
						cbor_item_t * item,
						struct cbor_load_result * result);

void handle_load_uint64(const unsigned char * source,
						size_t source_size,
						cbor_item_t * item,
						struct cbor_load_result * result);
#endif
