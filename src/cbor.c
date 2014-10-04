#include "cbor.h"
#include "cbor_internal.h"
#include <assert.h>

void cbor_incref(cbor_item_t * item)
{
	item->refcount++;
}

void cbor_decref(cbor_item_t ** item)
{
	if (--(*item)->refcount == 0) {
		switch((*item)->type) {
		case CBOR_TYPE_UINT:
			/* Fallthrough */
		case CBOR_TYPE_NEGINT:
			/* Combined allocation, freeing the item suffices */
			{
				break;
			}
		case CBOR_TYPE_BYTESTRING:
			{
				if (cbor_bytestring_is_definite(*item)) {
					free((*item)->data);
				} else {
					/* We need to decref all chunks */
					cbor_item_t ** handle = cbor_bytestring_chunks_handle(*item);
					for (size_t i = 0; i < cbor_bytestring_chunk_count(*item); i++)
						cbor_decref(&handle[i]);
					free(((struct cbor_indefinite_string_data *)(*item)->data)->chunks);
					free((*item)->data);
				}
				break;
			}
		case CBOR_TYPE_STRING:
		case CBOR_TYPE_ARRAY:
			{
				/* Get all items and decref them */
				cbor_item_t ** handle = cbor_array_handle(*item);
				for (size_t i = 0; i < cbor_array_size(*item); i++)
					cbor_decref(&handle[i]);
				/* Fallthrough */
			}
		case CBOR_TYPE_MAP:
		case CBOR_TYPE_TAG:
			{
				free((*item)->data);
				break;
			}
		case CBOR_TYPE_FLOAT_CTRL:
			{
				// unless it's a control symbol
				if (cbor_float_ctrl_get_width(*item) != CBOR_FLOAT_0)
					free((*item)->data);
				break;
			}
		}
		free(*item);
		//TODO
		*item = NULL;
	}
}

cbor_item_t * cbor_load(cbor_data source,
						size_t source_size,
						cbor_flags_t flags,
						struct cbor_load_result * result)
{
	/* Context stack */
	static struct cbor_callbacks callbacks = {
		.uint8 = &cbor_builder_uint8_callback,
		.uint16 = &cbor_builder_uint16_callback,
		.uint32 = &cbor_builder_uint32_callback,
		.uint64 = &cbor_builder_uint64_callback,

		.negint8 = &cbor_builder_negint8_callback,
		.negint16 = &cbor_builder_negint16_callback,
		.negint32 = &cbor_builder_negint32_callback,
		.negint64 = &cbor_builder_negint64_callback,

		.byte_string = &cbor_builder_byte_string_callback,
		.byte_string_start = &cbor_builder_byte_string_start_callback,

		.array_start = &cbor_builder_array_start_callback,
		.indef_array_start = &cbor_builder_indef_array_start_callback,

		.indef_break = &cbor_builder_indef_break_callback
	};

	if (source_size == 0) {
		result->error.code = CBOR_ERR_NODATA;
		return NULL;
	}
	/* Target for callbacks */
	struct _cbor_decoder_context * context = malloc(sizeof(struct _cbor_decoder_context));
	struct _cbor_stack stack = _cbor_stack_init();
	context->stack = &stack;
	struct cbor_decoder_result decode_result;
	*result = (struct cbor_load_result){ .read = 0, .error = { .code = CBOR_ERR_NONE } };

	// TODO unify errors
	// TODO unify returns
	do {
		if (source_size > result->read) { /* Check for overflows */
			decode_result = cbor_stream_decode(source + result->read, source_size - result->read, &callbacks, context);
		} else
			goto error;

		if (decode_result.status == CBOR_DECODER_FINISHED) {
			result->read += decode_result.read;
		} else
			goto error;

	} while (stack.size > 0);

	/* Move the result before free */
	cbor_item_t * result_item = context->root;
	free(context);
	return result_item;

error:
	/* Free the stack */
	while (stack.size > 0) {
		cbor_decref(&stack.top->item);
		_cbor_stack_pop(&stack);
	}
	free(context);
	result->error.code = CBOR_ERR_NOTENOUGHDATA;
	return NULL;
}

bool _cbor_claim_bytes(size_t required, size_t provided, struct cbor_decoder_result * result)
{
	if (required > (provided - result->read)) {
		/* We need to keep all the metadata if parsing is to be resumed */
		result->read = 0;
		result->status = CBOR_DECODER_NEDATA;
		return false;
	} else {
		result->read += required;
		return true;
	}
}

struct cbor_decoder_result cbor_stream_decode(cbor_data source, size_t source_size, const struct cbor_callbacks * callbacks, void * context)
{
	/* If we have no data, we cannot read even the MTB */
	if (source_size < 1) {
		return (struct cbor_decoder_result){ 0, CBOR_DECODER_EBUFFER };
	}

	/* If we have a byte, assume it's the MTB */
	struct cbor_decoder_result result = { 1, CBOR_DECODER_FINISHED };

	switch(*source) {
	case 0x00: /* Fallthrough */
	case 0x01: /* Fallthrough */
	case 0x02: /* Fallthrough */
	case 0x03: /* Fallthrough */
	case 0x04: /* Fallthrough */
	case 0x05: /* Fallthrough */
	case 0x06: /* Fallthrough */
	case 0x07: /* Fallthrough */
	case 0x08: /* Fallthrough */
	case 0x09: /* Fallthrough */
	case 0x0A: /* Fallthrough */
	case 0x0B: /* Fallthrough */
	case 0x0C: /* Fallthrough */
	case 0x0D: /* Fallthrough */
	case 0x0E: /* Fallthrough */
	case 0x0F: /* Fallthrough */
	case 0x10: /* Fallthrough */
	case 0x11: /* Fallthrough */
	case 0x12: /* Fallthrough */
	case 0x13: /* Fallthrough */
	case 0x14: /* Fallthrough */
	case 0x15: /* Fallthrough */
	case 0x16: /* Fallthrough */
	case 0x17:
		/* Embedded one byte unsigned integer */
		{
			callbacks->uint8(context, _cbor_load_uint8(source));
			return result;
		}
	case 0x18:
		/* One byte unsigned integer */
		{
			if (_cbor_claim_bytes(1, source_size, &result)) {
				callbacks->uint8(context, _cbor_load_uint8(source + 1));
			}
			return result;
		}
	case 0x19:
		/* Two bytes unsigned integer */
		{
			if (_cbor_claim_bytes(2, source_size, &result)) {
				callbacks->uint16(context, _cbor_load_uint16(source + 1));
			}
			return result;
		}
	case 0x1A:
		/* Four bytes unsigned integer */
		{
			if (_cbor_claim_bytes(4, source_size, &result)) {
				callbacks->uint32(context, _cbor_load_uint32(source + 1));
			}
			return result;
		}
	case 0x1B:
		/* Eight bytes unsigned integer */
		{
			if (_cbor_claim_bytes(8, source_size, &result)) {
				callbacks->uint64(context, _cbor_load_uint64(source + 1));
			}
			return result;
		}
	case 0x1C: /* Fallthrough */
	case 0x1D: /* Fallthrough */
	case 0x1E: /* Fallthrough */
	case 0x1F:
		/* Reserved */
		{
			return (struct cbor_decoder_result){ 0, CBOR_DECODER_ERROR };
		}
	case 0x20: /* Fallthrough */
	case 0x21: /* Fallthrough */
	case 0x22: /* Fallthrough */
	case 0x23: /* Fallthrough */
	case 0x24: /* Fallthrough */
	case 0x25: /* Fallthrough */
	case 0x26: /* Fallthrough */
	case 0x27: /* Fallthrough */
	case 0x28: /* Fallthrough */
	case 0x29: /* Fallthrough */
	case 0x2A: /* Fallthrough */
	case 0x2B: /* Fallthrough */
	case 0x2C: /* Fallthrough */
	case 0x2D: /* Fallthrough */
	case 0x2E: /* Fallthrough */
	case 0x2F: /* Fallthrough */
	case 0x30: /* Fallthrough */
	case 0x31: /* Fallthrough */
	case 0x32: /* Fallthrough */
	case 0x33: /* Fallthrough */
	case 0x34: /* Fallthrough */
	case 0x35: /* Fallthrough */
	case 0x36: /* Fallthrough */
	case 0x37:
		/* Embedded one byte negative integer */
		{
			callbacks->negint8(context, _cbor_load_uint8(source) - 0x20); /* 0x20 offset */
			return result;
		}
	case 0x38:
		/* One byte negative integer */
	{
		if (_cbor_claim_bytes(1, source_size, &result)) {
			callbacks->negint8(context, _cbor_load_uint8(source + 1));
		}
		return result;
	}
	case 0x39:
		/* Two bytes negative integer */
	{
		if (_cbor_claim_bytes(2, source_size, &result)) {
			callbacks->negint16(context, _cbor_load_uint16(source + 1));
		}
		return result;
	}
	case 0x3A:
		/* Four bytes negative integer */
	{
		if (_cbor_claim_bytes(4, source_size, &result)) {
			callbacks->negint32(context, _cbor_load_uint32(source + 1));
		}
		return result;
	}
	case 0x3B:
		/* Eight bytes negative integer */
	{
		if (_cbor_claim_bytes(8, source_size, &result)) {
			callbacks->negint64(context, _cbor_load_uint64(source + 1));
		}
		return result;
	}
	case 0x3C: /* Fallthrough */
	case 0x3D: /* Fallthrough */
	case 0x3E: /* Fallthrough */
	case 0x3F:
		/* Reserved */
		{
			return (struct cbor_decoder_result){ 0, CBOR_DECODER_ERROR };
		}
	case 0x40: /* Fallthrough */
	case 0x41: /* Fallthrough */
	case 0x42: /* Fallthrough */
	case 0x43: /* Fallthrough */
	case 0x44: /* Fallthrough */
	case 0x45: /* Fallthrough */
	case 0x46: /* Fallthrough */
	case 0x47: /* Fallthrough */
	case 0x48: /* Fallthrough */
	case 0x49: /* Fallthrough */
	case 0x4A: /* Fallthrough */
	case 0x4B: /* Fallthrough */
	case 0x4C: /* Fallthrough */
	case 0x4D: /* Fallthrough */
	case 0x4E: /* Fallthrough */
	case 0x4F: /* Fallthrough */
	case 0x50: /* Fallthrough */
	case 0x51: /* Fallthrough */
	case 0x52: /* Fallthrough */
	case 0x53: /* Fallthrough */
	case 0x54: /* Fallthrough */
	case 0x55: /* Fallthrough */
	case 0x56: /* Fallthrough */
	case 0x57:
		/* Embedded length byte string */
		{
			size_t length = (size_t)_cbor_load_uint8(source) - 0x40; /* 0x40 offset */
			if (_cbor_claim_bytes(length, source_size, &result)) {
				callbacks->byte_string(context, source + 1, length);
			}
			return result;
		}
	case 0x58:
		/* One byte length byte string */
		// TODO template this?
		{
			if (_cbor_claim_bytes(1, source_size, &result)) {
				size_t length = (size_t) _cbor_load_uint8(source + 1);
				if (_cbor_claim_bytes(length, source_size, &result)) {
					callbacks->byte_string(context, source + 1 + 1, length);
				}
			}
			return result;
		}
	case 0x59:
		/* Two bytes length byte string */
		{
			if (_cbor_claim_bytes(2, source_size, &result)) {
				size_t length = (size_t) _cbor_load_uint16(source + 1);
				if (_cbor_claim_bytes(length, source_size, &result)) {
					callbacks->byte_string(context, source + 1 + 2, length);
				}
			}
			return result;
		}
	case 0x5A:
		/* Four bytes length byte string */
		{
			if (_cbor_claim_bytes(4, source_size, &result)) {
				size_t length = (size_t) _cbor_load_uint32(source + 1);
				if (_cbor_claim_bytes(length, source_size, &result)) {
					callbacks->byte_string(context, source + 1 + 4, length);
				}
			}
			return result;
		}
	case 0x5B:
		/* Eight bytes length byte string */
		{
			if (_cbor_claim_bytes(8, source_size, &result)) {
				size_t length = (size_t) _cbor_load_uint64(source + 1);
				if (_cbor_claim_bytes(length, source_size, &result)) {
					callbacks->byte_string(context, source + 1 + 8, length);
				}
			}
			return result;
		}
	case 0x5C: /* Fallthrough */
	case 0x5D: /* Fallthrough */
	case 0x5E:
		/* Reserved */
		{
			return (struct cbor_decoder_result){ 0, CBOR_DECODER_ERROR };
		}
	case 0x5F:
		/* Indefinite byte string */
		{
			callbacks->byte_string_start(context);
			return result;
		}
	case 0x60: /* Fallthrough */
	case 0x61: /* Fallthrough */
	case 0x62: /* Fallthrough */
	case 0x63: /* Fallthrough */
	case 0x64: /* Fallthrough */
	case 0x65: /* Fallthrough */
	case 0x66: /* Fallthrough */
	case 0x67: /* Fallthrough */
	case 0x68: /* Fallthrough */
	case 0x69: /* Fallthrough */
	case 0x6A: /* Fallthrough */
	case 0x6B: /* Fallthrough */
	case 0x6C: /* Fallthrough */
	case 0x6D: /* Fallthrough */
	case 0x6E: /* Fallthrough */
	case 0x6F: /* Fallthrough */
	case 0x70: /* Fallthrough */
	case 0x71: /* Fallthrough */
	case 0x72: /* Fallthrough */
	case 0x73: /* Fallthrough */
	case 0x74: /* Fallthrough */
	case 0x75: /* Fallthrough */
	case 0x76: /* Fallthrough */
	case 0x77:
		/* Embedded one byte length string */
		{
			// TODO
			return result;
		}
	case 0x78:
		/* One byte length string */
	case 0x79:
		/* Two bytes length string */
	case 0x7A:
		/* Four bytes length string */
	case 0x7B:
		/* Eight bytes length string */
	case 0x7C: /* Fallthrough */
	case 0x7D: /* Fallthrough */
	case 0x7E:
		/* Reserved */
		{
			return (struct cbor_decoder_result){ 0, CBOR_DECODER_ERROR };
		}
	case 0x7F:
		/* Indefinite length string */
	case 0x80: /* Fallthrough */
	case 0x81: /* Fallthrough */
	case 0x82: /* Fallthrough */
	case 0x83: /* Fallthrough */
	case 0x84: /* Fallthrough */
	case 0x85: /* Fallthrough */
	case 0x86: /* Fallthrough */
	case 0x87: /* Fallthrough */
	case 0x88: /* Fallthrough */
	case 0x89: /* Fallthrough */
	case 0x8A: /* Fallthrough */
	case 0x8B: /* Fallthrough */
	case 0x8C: /* Fallthrough */
	case 0x8D: /* Fallthrough */
	case 0x8E: /* Fallthrough */
	case 0x8F: /* Fallthrough */
	case 0x90: /* Fallthrough */
	case 0x91: /* Fallthrough */
	case 0x92: /* Fallthrough */
	case 0x93: /* Fallthrough */
	case 0x94: /* Fallthrough */
	case 0x95: /* Fallthrough */
	case 0x96: /* Fallthrough */
	case 0x97:
		/* Embedded one byte length array */
		{
			callbacks->array_start(context, (size_t)_cbor_load_uint8(source) - 0x80); /* 0x40 offset */
			return result;
		}
	case 0x98:
		/* One byte length array */
		{
			if (_cbor_claim_bytes(1, source_size, &result)) {
				callbacks->array_start(context, (size_t)_cbor_load_uint8(source + 1));
			}
			return result;
		}
	case 0x99:
		/* Two bytes length string */
		{
			if (_cbor_claim_bytes(2, source_size, &result)) {
				callbacks->array_start(context, (size_t)_cbor_load_uint16(source + 1));
			}
			return result;
		}
	case 0x9A:
		/* Four bytes length string */
		{
			if (_cbor_claim_bytes(4, source_size, &result)) {
				callbacks->array_start(context, (size_t)_cbor_load_uint32(source + 1));
			}
			return result;
		}
	case 0x9B:
		/* Eight bytes length string */
		{
			if (_cbor_claim_bytes(8, source_size, &result)) {
				callbacks->array_start(context, (size_t)_cbor_load_uint64(source + 1));
			}
			return result;
		}
	case 0x9C: /* Fallthrough */
	case 0x9D: /* Fallthrough */
	case 0x9E:
		/* Reserved */
		{
			return (struct cbor_decoder_result){ 0, CBOR_DECODER_ERROR };
		}
	case 0x9F:
		/* Indefinite length array */
		{
			callbacks->indef_array_start(context);
			return result;
		}
	case 0xA0: /* Fallthrough */
	case 0xA1: /* Fallthrough */
	case 0xA2: /* Fallthrough */
	case 0xA3: /* Fallthrough */
	case 0xA4: /* Fallthrough */
	case 0xA5: /* Fallthrough */
	case 0xA6: /* Fallthrough */
	case 0xA7: /* Fallthrough */
	case 0xA8: /* Fallthrough */
	case 0xA9: /* Fallthrough */
	case 0xAA: /* Fallthrough */
	case 0xAB: /* Fallthrough */
	case 0xAC: /* Fallthrough */
	case 0xAD: /* Fallthrough */
	case 0xAE: /* Fallthrough */
	case 0xAF: /* Fallthrough */
	case 0xB0: /* Fallthrough */
	case 0xB1: /* Fallthrough */
	case 0xB2: /* Fallthrough */
	case 0xB3: /* Fallthrough */
	case 0xB4: /* Fallthrough */
	case 0xB5: /* Fallthrough */
	case 0xB6: /* Fallthrough */
	case 0xB7:
		/* Embedded one byte length map */
		{
			callbacks->map_start(context, (size_t)_cbor_load_uint8(source) - 0xA0); /* 0xA0 offset */
			return result;
		}
	case 0xB8:
		/* One byte length map */
		{
			if (_cbor_claim_bytes(1, source_size, &result)) {
				callbacks->map_start(context, (size_t)_cbor_load_uint8(source + 1));
			}
			return result;
		}
	case 0xB9:
		/* Two bytes length map */
		{
			if (_cbor_claim_bytes(2, source_size, &result)) {
				callbacks->map_start(context, (size_t)_cbor_load_uint16(source + 1));
			}
			return result;
		}
	case 0xBA:
		/* Four bytes length map */
		{
			if (_cbor_claim_bytes(4, source_size, &result)) {
				callbacks->map_start(context, (size_t)_cbor_load_uint32(source + 1));
			}
			return result;
		}
	case 0xBB:
		/* Eight bytes length map */
		{
			if (_cbor_claim_bytes(8, source_size, &result)) {
				callbacks->map_start(context, (size_t)_cbor_load_uint64(source + 1));
			}
			return result;
		}
	case 0xBC: /* Fallthrough */
	case 0xBD: /* Fallthrough */
	case 0xBE:
		/* Reserved */
		{
			return (struct cbor_decoder_result){ 0, CBOR_DECODER_ERROR };
		}
	case 0xBF:
		/* Indefinite length map */
		{
			callbacks->indef_map_start(context);
			return result;
		}
	case 0xC0:
		/* Text date/time - RFC 3339 tag, fallthrough */
	case 0xC1:
		/* Epoch date tag, fallthrough */
	case 0xC2:
		/* Positive bignum tag, fallthrough */
	case 0xC3:
		/* Negative bignum tag, fallthrough */
	case 0xC4:
		/* Fraction, fallthrough */
	case 0xC5:
		/* Big float */
		{
			callbacks->tag(context, _cbor_load_uint8(source) - 0xC0); /* 0xC0 offset */
			return result;
		}
	case 0xC6: /* Fallthrough */
	case 0xC7: /* Fallthrough */
	case 0xC8: /* Fallthrough */
	case 0xC9: /* Fallthrough */
	case 0xCA: /* Fallthrough */
	case 0xCB: /* Fallthrough */
	case 0xCC: /* Fallthrough */
	case 0xCD: /* Fallthrough */
	case 0xCE: /* Fallthrough */
	case 0xCF: /* Fallthrough */
	case 0xD0: /* Fallthrough */
	case 0xD1: /* Fallthrough */
	case 0xD2: /* Fallthrough */
	case 0xD3: /* Fallthrough */
	case 0xD4: /* Unassigned tag value */
		{
			return (struct cbor_decoder_result){ 0, CBOR_DECODER_ERROR };
		}
	case 0xD5: /* Expected b64url conversion tag - fallthrough */
	case 0xD6: /* Expected b64 conversion tag - fallthrough */
	case 0xD7: /* Expected b16 conversion tag */
		{
			callbacks->tag(context, _cbor_load_uint8(source) - 0xC0); /* 0xC0 offset */
			return result;
		}
	case 0xD8: /* 1B tag */
		{
			if (_cbor_claim_bytes(1, source_size, &result)) {
				callbacks->tag(context, _cbor_load_uint8(source + 1));
			}
			return result;
		}
	case 0xD9: /* 2B tag */
		{
			if (_cbor_claim_bytes(2, source_size, &result)) {
				callbacks->tag(context, _cbor_load_uint16(source + 1));
			}
			return result;
		}
	case 0xDA: /* 4B tag */
		{
			if (_cbor_claim_bytes(4, source_size, &result)) {
				callbacks->tag(context, _cbor_load_uint32(source + 1));
			}
			return result;
		}
	case 0xDB: /* 8B tag */
		{
			if (_cbor_claim_bytes(8, source_size, &result)) {
				callbacks->tag(context, _cbor_load_uint64(source + 1));
			}
			return result;
		}
	case 0xDC: /* Fallthrough */
	case 0xDD: /* Fallthrough */
	case 0xDE: /* Fallthrough */
	case 0xDF: /* Reserved */
		{
			return (struct cbor_decoder_result){ 0, CBOR_DECODER_ERROR };
		}
	case 0xE0: /* Fallthrough */
	case 0xE1: /* Fallthrough */
	case 0xE2: /* Fallthrough */
	case 0xE3: /* Fallthrough */
	case 0xE4: /* Fallthrough */
	case 0xE5: /* Fallthrough */
	case 0xE6: /* Fallthrough */
	case 0xE7: /* Fallthrough */
	case 0xE8: /* Fallthrough */
	case 0xE9: /* Fallthrough */
	case 0xEA: /* Fallthrough */
	case 0xEB: /* Fallthrough */
	case 0xEC: /* Fallthrough */
	case 0xED: /* Fallthrough */
	case 0xEE: /* Fallthrough */
	case 0xEF: /* Fallthrough */
	case 0xF0: /* Fallthrough */
	case 0xF1: /* Fallthrough */
	case 0xF2: /* Fallthrough */
	case 0xF3: /* Simple value - unassigned */
		// TODO pass these as valid?
		{
			return (struct cbor_decoder_result){ 0, CBOR_DECODER_ERROR };
		}
	case 0xF4:
		/* False */
		{
			callbacks->boolean(context, false);
			return result;
		}
	case 0xF5:
		/* True */
		{
			callbacks->boolean(context, true);
			return result;
		}
	case 0xF6:
		/* Null */
		{
			callbacks->null(context);
			return result;
		}
	case 0xF7:
		/* Undefined */
		{
			callbacks->undefined(context);
			return result;
		}
	case 0xF8:
		/* 1B simple value, unassigned */
		{
			return (struct cbor_decoder_result){ 0, CBOR_DECODER_ERROR };
		}
	case 0xF9:
		/* 2B float */
		{
			if (_cbor_claim_bytes(2, source_size, &result)) {
				callbacks->float2(context, _cbor_load_half(source + 1));
			}
			return  result;
		}
	case 0xFA:
		/* 4B float */
		{
			if (_cbor_claim_bytes(4, source_size, &result)) {
				callbacks->float4(context, _cbor_load_float(source + 1));
			}
			return  result;
		}
	case 0xFB:
		/* 8B float */
		{
			if (_cbor_claim_bytes(8, source_size, &result)) {
				callbacks->float8(context, _cbor_load_double(source + 1));
			}
			return  result;
		}
	case 0xFC: /* Fallthrough */
	case 0xFD: /* Fallthrough */
	case 0xFE:
		/* Reserved */
		{
			return (struct cbor_decoder_result){ 0, CBOR_DECODER_ERROR };
		}
	case 0xFF:
		/* Break */
		{
			callbacks->indef_break(context);
			return result;
		}
	default: /* Never happens - this shuts up the compiler */
		{
			return result;
		}
	}
}


inline cbor_type cbor_typeof(const cbor_item_t * item)
{
	return item->type;
}


cbor_int_width cbor_int_get_width(const cbor_item_t * item)
{
	assert(cbor_is_int(item));
	return item->metadata.int_metadata.width;
}

uint8_t cbor_get_uint8(const cbor_item_t * item)
{
	return *(uint8_t *)item->data;
}

uint16_t cbor_get_uint16(const cbor_item_t * item)
{
	return *(uint16_t *)item->data;
}

uint32_t cbor_get_uint32(const cbor_item_t * item)
{
	return *(uint32_t *)item->data;
}

uint64_t cbor_get_uint64(const cbor_item_t * item)
{
	return *(uint64_t *)item->data;
}

void cbor_set_uint8(cbor_item_t * item, uint8_t value)
{
	assert(cbor_is_int(item));
	assert(cbor_int_get_width(item) == CBOR_INT_8);
	*(uint8_t *)item->data = value;
}

void cbor_set_uint16(cbor_item_t * item, uint16_t value)
{
	assert(cbor_is_int(item));
	assert(cbor_int_get_width(item) == CBOR_INT_16);
	*(uint16_t *)item->data = value;
}


void cbor_set_uint32(cbor_item_t * item, uint32_t value)
{
	assert(cbor_is_int(item));
	assert(cbor_int_get_width(item) == CBOR_INT_32);
	*(uint32_t *)item->data = value;
}

void cbor_set_uint64(cbor_item_t * item, uint64_t value)
{
	assert(cbor_is_int(item));
	assert(cbor_int_get_width(item) == CBOR_INT_64);
	*(uint64_t *)item->data = value;
}

void cbor_mark_uint(cbor_item_t * item)
{
	assert(cbor_is_int(item));
	item->type = CBOR_TYPE_UINT;
}

void cbor_mark_negint(cbor_item_t * item)
{
	assert(cbor_is_int(item));
	item->type = CBOR_TYPE_NEGINT;
}

cbor_item_t * cbor_new_int8()
{
	cbor_item_t * item = malloc(sizeof(cbor_item_t) + 1);
	*item = (cbor_item_t){
		.data = (unsigned char *)item + sizeof(cbor_item_t),
		.refcount = 1,
		.metadata = { .int_metadata = { .width = CBOR_INT_8 } }
	};
	return item;
}

cbor_item_t * cbor_new_int16()
{
	cbor_item_t * item = malloc(sizeof(cbor_item_t) + 2);
	*item = (cbor_item_t){
		.data = (unsigned char *)item + sizeof(cbor_item_t),
		.refcount = 1,
		.metadata = { .int_metadata = { .width = CBOR_INT_16 } }
	};
	return item;
}

cbor_item_t * cbor_new_int32()
{
	cbor_item_t * item = malloc(sizeof(cbor_item_t) + 4);
	*item = (cbor_item_t){
		.data = (unsigned char *)item + sizeof(cbor_item_t),
		.refcount = 1,
		.metadata = { .int_metadata = { .width = CBOR_INT_32 } }
	};
	return item;
}

cbor_item_t * cbor_new_int64()
{
	cbor_item_t * item = malloc(sizeof(cbor_item_t) + 8);
	*item = (cbor_item_t){
		.data = (unsigned char *)item + sizeof(cbor_item_t),
		.refcount = 1,
		.metadata = { .int_metadata = { .width = CBOR_INT_64 } }
	};
	return item;
}


cbor_item_t * cbor_new_definite_string()
{
	cbor_item_t * item = malloc(sizeof(cbor_item_t));
	*item = (cbor_item_t){
		.refcount = 1,
		.type = CBOR_TYPE_STRING,
		.metadata = { .string_metadata = { _CBOR_STRING_METADATA_DEFINITE, 0 } }
	};
	return item;
}

cbor_item_t * cbor_new_indefinite_string()
{
	cbor_item_t * item = malloc(sizeof(cbor_item_t));
	*item = (cbor_item_t){
		.refcount = 1,
		.type = CBOR_TYPE_STRING,
		.metadata = { .string_metadata = { .type = _CBOR_STRING_METADATA_INDEFINITE, .length = 0 } },
		.data = malloc(sizeof(struct cbor_indefinite_string_data))
	};
	*((struct cbor_indefinite_string_data *)item->data) = (struct cbor_indefinite_string_data){
		.chunk_count = 0,
		.chunk_capacity = 0,
		.chunks = NULL,
	};
	return item;
}


void cbor_string_set_handle(cbor_item_t * item, unsigned char * data, size_t length)
{
	assert(cbor_isa_string(item));
	assert(cbor_string_is_definite(item));
	item->data = data;
	item->metadata.string_metadata.length = length;
}

cbor_item_t * * cbor_string_chunks_handle(const cbor_item_t * item)
{
	assert(cbor_isa_string(item));
	assert(cbor_string_is_indefinite(item));
	return ((struct cbor_indefinite_string_data *)item->data)->chunks;
}

size_t cbor_string_chunk_count(const cbor_item_t * item)
{
	assert(cbor_isa_string(item));
	assert(cbor_string_is_indefinite(item));
	return ((struct cbor_indefinite_string_data *)item->data)->chunk_count;

}

cbor_item_t * cbor_string_add_chunk(cbor_item_t * item, cbor_item_t * chunk)
{
	assert(cbor_isa_string(item));
	assert(cbor_string_is_indefinite(item));
	struct cbor_indefinite_string_data * data = (struct cbor_indefinite_string_data *)item->data;
	// TODO optimize this with exponential growth
	if (data->chunk_count == data->chunk_capacity) {
		/* We need more space */
		data->chunk_capacity = data->chunk_capacity == 0 ? 1 : data->chunk_capacity * 2;
		cbor_item_t ** new_chunks_data =
			realloc(data->chunks, data->chunk_capacity * sizeof(cbor_item_t *));
		// TODO handle failures
		data->chunks = new_chunks_data;
	}
	data->chunks[data->chunk_count++] = chunk;
	return item;
}

size_t cbor_string_length(const cbor_item_t * item) {
	assert(cbor_isa_string(item));
	return item->metadata.string_metadata.length;
}

unsigned char * cbor_string_handle(const cbor_item_t * item) {
	assert(cbor_isa_string(item));
	return item->data;
}

bool cbor_string_is_definite(const cbor_item_t * item)
{
	assert(cbor_isa_string(item));
	return item->metadata.string_metadata.type == _CBOR_STRING_METADATA_DEFINITE;
}

bool cbor_string_is_indefinite(const cbor_item_t * item)
{
	return !cbor_string_is_definite(item);
}


cbor_item_t * cbor_new_definite_bytestring()
{
	cbor_item_t * item = malloc(sizeof(cbor_item_t));
	*item = (cbor_item_t){
		.refcount = 1,
		.type = CBOR_TYPE_BYTESTRING,
		.metadata = { .bytestring_metadata = { _CBOR_STRING_METADATA_DEFINITE, 0 } }
	};
	return item;
}

cbor_item_t * cbor_new_indefinite_bytestring()
{
	cbor_item_t * item = malloc(sizeof(cbor_item_t));
	*item = (cbor_item_t){
		.refcount = 1,
		.type = CBOR_TYPE_BYTESTRING,
		.metadata = { .bytestring_metadata = { .type = _CBOR_STRING_METADATA_INDEFINITE, .length = 0 } },
		.data = malloc(sizeof(struct cbor_indefinite_string_data))
	};
	*((struct cbor_indefinite_string_data *)item->data) = (struct cbor_indefinite_string_data){
		.chunk_count = 0,
		.chunk_capacity = 0,
		.chunks = NULL,
	};
	return item;
}

void cbor_bytestring_set_handle(cbor_item_t * item, unsigned char * data, size_t length)
{
	assert(cbor_isa_bytestring(item));
	assert(cbor_bytestring_is_definite(item));
	item->data = data;
	item->metadata.bytestring_metadata.length = length;
}

cbor_item_t * * cbor_bytestring_chunks_handle(const cbor_item_t * item)
{
	assert(cbor_isa_bytestring(item));
	assert(cbor_bytestring_is_indefinite(item));
	return ((struct cbor_indefinite_string_data *)item->data)->chunks;
}

size_t cbor_bytestring_chunk_count(const cbor_item_t * item)
{
	assert(cbor_isa_bytestring(item));
	assert(cbor_bytestring_is_indefinite(item));
	return ((struct cbor_indefinite_string_data *)item->data)->chunk_count;

}

cbor_item_t * cbor_bytestring_add_chunk(cbor_item_t * item, cbor_item_t * chunk)
{
	assert(cbor_isa_bytestring(item));
	assert(cbor_bytestring_is_indefinite(item));
	struct cbor_indefinite_string_data * data = (struct cbor_indefinite_string_data *)item->data;
	// TODO optimize this with exponential growth
	if (data->chunk_count == data->chunk_capacity) {
		/* We need more space */
		data->chunk_capacity = data->chunk_capacity == 0 ? 1 : data->chunk_capacity * 2;
		cbor_item_t ** new_chunks_data =
			realloc(data->chunks, data->chunk_capacity * sizeof(cbor_item_t *));
		// TODO handle failures
		data->chunks = new_chunks_data;
	}
	data->chunks[data->chunk_count++] = chunk;
	return item;
}


cbor_item_t * cbor_new_definite_array(size_t size)
{
	cbor_item_t * item = malloc(sizeof(cbor_item_t));
	*item = (cbor_item_t){
		.refcount = 1,
		.type = CBOR_TYPE_ARRAY,
		.metadata = { .array_metadata = { .type = _CBOR_ARRAY_METADATA_DEFINITE, .size = 0 } },
		.data = malloc(sizeof(cbor_item_t *) * size)
	};
	return item;
}

cbor_item_t * cbor_new_indefinite_array()
{
	cbor_item_t * item = malloc(sizeof(cbor_item_t));
	*item = (cbor_item_t){
		.refcount = 1,
		.type = CBOR_TYPE_ARRAY,
		.metadata = { .array_metadata = { .type = _CBOR_ARRAY_METADATA_INDEFINITE, .size = 0 } },
		.data = NULL /* Can be safely realloc-ed */
	};
	return item;
}

cbor_item_t * cbor_array_push(cbor_item_t * array, cbor_item_t * pushee)
{
	assert(cbor_isa_array(array));
	struct _cbor_array_metadata * metadata = (struct _cbor_array_metadata *)&array->metadata;
	cbor_item_t * * data = (cbor_item_t * *)array->data;
	if (cbor_array_is_definite(array)) {
		// TODO check size - throw
		data[metadata->size++] = pushee;
	} else {
		// TODO exponential reallocs?
		data = realloc(data, (metadata->size + 1) * sizeof(cbor_item_t *));
		data[metadata->size++] = pushee;
		array->data = (unsigned char *)data;
	}
	return array;
}

/** ========================================================== */

inline bool cbor_isa_uint(const cbor_item_t * item)
{
	return item->type == CBOR_TYPE_UINT;
}

inline bool cbor_isa_negint(const cbor_item_t * item)
{
	return item->type == CBOR_TYPE_NEGINT;
}

inline bool cbor_isa_bytestring(const cbor_item_t * item)
{
	return item->type == CBOR_TYPE_BYTESTRING;
}

inline bool cbor_isa_string(const cbor_item_t * item)
{
	return item->type == CBOR_TYPE_STRING;
}

inline bool cbor_isa_array(const cbor_item_t * item)
{
	return item->type == CBOR_TYPE_ARRAY;
}

inline bool cbor_isa_map(const cbor_item_t * item)
{
	return item->type == CBOR_TYPE_MAP;
}

inline bool cbor_isa_tag(const cbor_item_t * item)
{
	return item->type == CBOR_TYPE_TAG;
}

inline bool cbor_isa_float_ctrl(const cbor_item_t * item)
{
	return item->type == CBOR_TYPE_FLOAT_CTRL;
}


/** ========================================================== */


inline bool cbor_is_int(const cbor_item_t * item)
{
	return cbor_isa_uint(item) || cbor_isa_negint(item);
}

inline bool cbor_is_uint(const cbor_item_t * item)
{
	/* Negative 'signed' ints are negints */
	return cbor_isa_uint(item);
}


inline bool cbor_is_bool(const cbor_item_t * item)
{
	return cbor_isa_float_ctrl(item) && (cbor_float_ctrl_get_ctrl(item) == CBOR_CTRL_FALSE || cbor_float_ctrl_get_ctrl(item) == CBOR_CTRL_TRUE);
}

inline bool cbor_is_null(const cbor_item_t * item)
{
	return cbor_isa_float_ctrl(item) && cbor_float_ctrl_get_ctrl(item) == CBOR_CTRL_NULL;
}

inline bool cbor_is_undef(const cbor_item_t * item)
{
	return cbor_isa_float_ctrl(item) && cbor_float_ctrl_get_ctrl(item) == CBOR_CTRL_UNDEF;
}

inline bool cbor_is_break(const cbor_item_t * item)
{
	return cbor_isa_float_ctrl(item) && cbor_float_ctrl_get_ctrl(item) == CBOR_CTRL_BREAK;
}

size_t cbor_bytestring_length(const cbor_item_t * item) {
	assert(cbor_isa_bytestring(item));
	return item->metadata.bytestring_metadata.length;
}

unsigned char * cbor_bytestring_handle(const cbor_item_t * item) {
	assert(cbor_isa_bytestring(item));
	return item->data;
}

bool cbor_bytestring_is_definite(const cbor_item_t * item)
{
	assert(cbor_isa_bytestring(item));
	return item->metadata.bytestring_metadata.type == _CBOR_STRING_METADATA_DEFINITE;
}

bool cbor_bytestring_is_indefinite(const cbor_item_t * item)
{
	return !cbor_bytestring_is_definite(item);
}

size_t cbor_array_size(cbor_item_t * item)
{
	assert(cbor_isa_array(item));
	return item->metadata.array_metadata.size;
}

bool cbor_array_is_definite(cbor_item_t * item)
{
	assert(cbor_isa_array(item));
	return item->metadata.array_metadata.type == _CBOR_ARRAY_METADATA_DEFINITE;
}

bool cbor_array_is_indefinite(cbor_item_t * item)
{
	assert(cbor_isa_array(item));
	return item->metadata.array_metadata.type == _CBOR_ARRAY_METADATA_INDEFINITE;
}

cbor_item_t ** cbor_array_handle(cbor_item_t * item)
{
	assert(cbor_isa_array(item));
	return (cbor_item_t **)item->data;
}

cbor_float_width cbor_float_ctrl_get_width(const cbor_item_t * item)
{
	assert(cbor_isa_float_ctrl(item));
	return item->metadata.float_ctrl_metadata.width;
}

cbor_ctrl cbor_float_ctrl_get_ctrl(const cbor_item_t * item)
{
	assert(cbor_isa_float_ctrl(item));
	assert(cbor_float_ctrl_get_width(item) == CBOR_FLOAT_0);
	return item->metadata.float_ctrl_metadata.type;
}

#ifdef DEBUG
void cbor_describe(cbor_item_t * item) {
	printf("Address: %p\t\t Type: ", (void *)item);
	switch(cbor_typeof(item)) {
	case CBOR_TYPE_UINT:
		{
			printf("UInt\n");
			break;
		}
	case CBOR_TYPE_NEGINT:
		{
			printf("NegInt\n");
			// TODO
			break;
		}
	case CBOR_TYPE_BYTESTRING:
		{
			printf("Bytestring\n");
			// TODO
			break;
		}
	case CBOR_TYPE_STRING:
		{
			printf("String\n");
			// TODO
			break;
		}
	case CBOR_TYPE_ARRAY:
		{
			printf("Array\n");
			// TODO
			break;
		}
	case CBOR_TYPE_MAP:
		{
			printf("Map\n");
			// TODO
			break;
		}
	case CBOR_TYPE_TAG:
		{
			printf("Tag\n");
			// TODO
			break;
		}
	case CBOR_TYPE_FLOAT_CTRL:
		{
			printf("Float/CTRL\n");
			// TODO
			break;
		}
	default:
		{
			printf("Invalid type! (%u)\n", cbor_typeof(item));
		}
	}
}

#endif
