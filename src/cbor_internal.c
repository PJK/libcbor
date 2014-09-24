#include "cbor_internal.h"
#include <assert.h>
#include <string.h>
#include <math.h>

// TODO asserts
// TODO check mallocs

// TODO endianess detection
uint8_t _cbor_load_uint8(cbor_data source)
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

/* As per http://tools.ietf.org/html/rfc7049#appendix-D */
double _cbor_decode_half(unsigned char *halfp) {
	int half = (halfp[0] << 8) + halfp[1];
	int exp = (half >> 10) & 0x1f;
	int mant = half & 0x3ff;
	double val;
	if (exp == 0) val = ldexp(mant, -24);
	else if (exp != 31) val = ldexp(mant + 1024, exp - 25);
	else val = mant == 0 ? INFINITY : NAN;
	return half & 0x8000 ? -val : val;
}

double _cbor_load_half(cbor_data source)
{
	/* Discard const */
	return _cbor_decode_half((unsigned char *)source);
}

/* Raw memory casts */
union _cbor_float_helper {
	float    as_float;
	uint32_t as_uint;
};

union _cbor_double_helper {
	double   as_double;
	uint64_t as_uint;
};

float _cbor_load_float(cbor_data source)
{
	union _cbor_float_helper helper = { .as_uint = _cbor_load_uint32(source) };
	return helper.as_float;
}

double _cbor_load_double(cbor_data source)
{
	union _cbor_double_helper helper = { .as_uint = _cbor_load_uint64(source) };
	return helper.as_double;
}

enum cbor_callback_result cbor_builder_uint8_callback(void * context, uint8_t value)
{
	cbor_item_t * res = cbor_new_int8();
	cbor_mark_uint(res);
	cbor_set_uint8(res, value);
	((struct _cbor_decoder_context *)context)->root = res;
	return CBOR_CALLBACK_OK;
}

enum cbor_callback_result cbor_builder_byte_string_callback(void * context, cbor_data data, size_t length)
{
	struct _cbor_decoder_context * ctx = context;
	unsigned char * new_handle = malloc(length);
	memcpy(new_handle, data, length);
	cbor_item_t * res = cbor_new_definite_bytestring();
	cbor_bytestring_set_handle(res, new_handle, length);
	if (ctx->stack->size > 0) {
		if (ctx->stack->top->type == CBOR_TYPE_BYTESTRING) {
			// TODO check success
			cbor_bytestring_add_chunk(ctx->current, res);
		} else {
			// TODO complain loudly
		}
	} else {
		ctx->root = res;
	}
	return CBOR_CALLBACK_OK;
}

enum cbor_callback_result cbor_builder_byte_string_start_callback(void * context)
{
	struct _cbor_decoder_context * ctx = context;
	_cbor_stack_push(ctx->stack, CBOR_TYPE_BYTESTRING);
	ctx->current = cbor_new_indefinite_bytestring();
	return CBOR_CALLBACK_OK;
}

enum cbor_callback_result cbor_builder_indef_break_callback(void * context)
{
	struct _cbor_decoder_context * ctx = context;
	if (ctx->stack->size == 0) {
		// TODO complain
	} else {
		_cbor_stack_pop(ctx->stack);
		if (ctx->stack->size == 0) {
			ctx->root = ctx->current;
		} else {
			//TODO assign this to the containing structure
		}
	}
	return CBOR_CALLBACK_OK;
}
