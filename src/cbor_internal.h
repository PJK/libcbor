#ifndef CBOR_INTERNAL_
#define CBOR_INTERNAL_

#include "cbor.h"
#include "cbor_stack.h"


#define _CBOR_METADATA_WIDTH sizeof(_cbor_metadata)

#define _CBOR_INT_METADATA_WIDTH sizeof(cbor_int_width)

#define _CBOR_BYTESTRING_METADATA_WIDTH sizeof(struct _cbor_bytestring_metadata)

#define _CBOR_ARRAY_METADATA_WIDTH sizeof(struct _cbor_array_metadata)

#define _CBOR_FLOAT_CTRL_METADATA_WIDTH sizeof(struct _cbor_float_ctrl_metadata)


bool _cbor_assert_avail_bytes(size_t num,
							  size_t source_size,
							  struct cbor_load_result * result);

/* Read the given uint from the given location, no questions asked */
uint8_t _cbor_load_uint8(const unsigned char * source);
uint16_t _cbor_load_uint16(const unsigned char * source);
uint32_t _cbor_load_uint32(const unsigned char * source);
uint64_t _cbor_load_uint64(const unsigned char * source);

double _cbor_load_half(cbor_data source);
float _cbor_load_float(cbor_data source);
double _cbor_load_double(cbor_data source);

enum cbor_callback_result cbor_builder_uint8_callback(void *, uint8_t);

enum cbor_callback_result cbor_builder_byte_string_callback(void *, cbor_data, size_t);
enum cbor_callback_result cbor_builder_byte_string_start_callback(void *);

struct _cbor_decoder_context {
	cbor_item_t *        result;
	struct _cbor_stack * stack;
};

#endif
