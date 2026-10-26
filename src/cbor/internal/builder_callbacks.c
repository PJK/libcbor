/*
 * SPDX-FileCopyrightText: 2014-2026 Pavel Kalvoda <me@pavelkalvoda.com>
 * SPDX-License-Identifier: MIT
 */

#include "builder_callbacks.h"

#include <string.h>

#include "../arrays.h"
#include "../bytestrings.h"
#include "../common.h"
#include "../floats_ctrls.h"
#include "../ints.h"
#include "../maps.h"
#include "../strings.h"
#include "../tags.h"
#include "memory_utils.h"
#include "unicode.h"

/**
 * Every subitem of a definite array or map takes at least one byte, so an
 * open definite container with `subitems` items left needs `subitems - 1`
 * bytes beyond the item in progress in it (that item is either the one being
 * decoded or the container above it, and its bytes are counted there).
 * Indefinite containers and tags need nothing. Clamping to the input size
 * keeps the running total from overflowing without changing any comparison
 * against `remaining`.
 */
static uint64_t _cbor_owed(const struct _cbor_decoder_context* ctx,
                           size_t subitems) {
  if (subitems == 0) return 0;
  return subitems - 1 < ctx->source_size ? subitems - 1 : ctx->source_size;
}

/** Record that the top container received one of its subitems. */
static void _cbor_owed_decrement(struct _cbor_decoder_context* ctx) {
  size_t subitems = ctx->stack->top->subitems;
  ctx->owed -= _cbor_owed(ctx, subitems) - _cbor_owed(ctx, subitems - 1);
  ctx->stack->top->subitems--;
}

/**
 * How many subitems the input can still hold for a container whose header is
 * being decoded: the bytes left after its header, minus what the enclosing
 * containers still need.
 */
static size_t _cbor_claim_budget(const struct _cbor_decoder_context* ctx) {
  uint64_t after_header = ctx->remaining - 1;
  return ctx->owed < after_header ? (size_t)(after_header - ctx->owed) : 0;
}

/**
 * Make room for one more item in a definite array or map whose header
 * claimed more items than the input could hold (see
 * `cbor_builder_array_start_callback`). Grows geometrically, but never past
 * the claimed size, so storage stays proportional to the items that actually
 * arrive. `left` is the number of items (pairs, for maps) still expected,
 * including the one being added.
 */
static bool _cbor_builder_grow(cbor_item_t* item, size_t item_size,
                               size_t* allocated, size_t end_ptr,
                               size_t left) {
  size_t wanted = *allocated == 0 ? 1 : *allocated;
  if (*allocated > 0 && _cbor_safe_to_multiply(CBOR_BUFFER_GROWTH, wanted))
    wanted *= CBOR_BUFFER_GROWTH;
  if (wanted - end_ptr > left) wanted = end_ptr + left;
  unsigned char* data = _cbor_realloc_multiple(item->data, item_size, wanted);
  if (data == NULL) return false;
  memset(data + item_size * *allocated, 0,
         item_size * (wanted - *allocated));
  item->data = data;
  *allocated = wanted;
  return true;
}

// `_cbor_builder_append` takes ownership of `item`. If adding the item to
// parent container fails, `item` will be deallocated to prevent memory.
void _cbor_builder_append(cbor_item_t* item,
                          struct _cbor_decoder_context* ctx) {
  if (ctx->stack->size == 0) {
    /* Top level item */
    ctx->root = item;
    return;
  }
  /* Part of a bigger structure */
  switch (ctx->stack->top->item->type) {
    // Handle Arrays and Maps since they can contain subitems of any type.
    // Byte/string construction from chunks is handled in the respective chunk
    // handlers.
    case CBOR_TYPE_ARRAY: {
      if (cbor_array_is_definite(ctx->stack->top->item)) {
        // We don't need an explicit check for whether the item still belongs
        // into this array because if there are extra items, they will cause a
        // syntax error when decoded.
        CBOR_ASSERT(ctx->stack->top->subitems > 0);
        cbor_item_t* array = ctx->stack->top->item;
        struct _cbor_array_metadata* metadata = &array->metadata.array_metadata;
        if (metadata->end_ptr == metadata->allocated &&
            !_cbor_builder_grow(array, sizeof(cbor_item_t*),
                                &metadata->allocated, metadata->end_ptr,
                                ctx->stack->top->subitems)) {
          ctx->creation_failed = true;
          cbor_decref(&item);
          break;
        }
        if (!cbor_array_push(array, item)) {
          ctx->creation_failed = true;
          cbor_decref(&item);
          break;
        }
        cbor_decref(&item);
        _cbor_owed_decrement(ctx);
        if (ctx->stack->top->subitems == 0) {
          cbor_item_t* stack_item = ctx->stack->top->item;
          _cbor_stack_pop(ctx->stack);
          _cbor_builder_append(stack_item, ctx);
        }
      } else {
        /* Indefinite array, don't bother with subitems */
        if (!cbor_array_push(ctx->stack->top->item, item)) {
          ctx->creation_failed = true;
        }
        cbor_decref(&item);
      }
      break;
    }
    case CBOR_TYPE_MAP: {
      // Handle both definite and indefinite maps the same initially.
      // Note: We use 0 and 1 subitems to distinguish between keys and values in
      // indefinite items
      if (ctx->stack->top->subitems % 2) {
        // Odd record, this is a value.
        ctx->creation_failed =
            !_cbor_map_add_value(ctx->stack->top->item, item);
        // Adding a value never fails since the memory is allocated when the
        // key is added
        CBOR_ASSERT(!ctx->creation_failed);
      } else {
        // Even record, this is a key.
        cbor_item_t* map = ctx->stack->top->item;
        struct _cbor_map_metadata* metadata = &map->metadata.map_metadata;
        if (cbor_map_is_definite(map) &&
            metadata->end_ptr == metadata->allocated &&
            !_cbor_builder_grow(map, sizeof(struct cbor_pair),
                                &metadata->allocated, metadata->end_ptr,
                                ctx->stack->top->subitems / 2)) {
          ctx->creation_failed = true;
          cbor_decref(&item);
          break;
        }
        if (!_cbor_map_add_key(map, item)) {
          ctx->creation_failed = true;
          cbor_decref(&item);
          break;
        }
      }
      cbor_decref(&item);
      if (cbor_map_is_definite(ctx->stack->top->item)) {
        CBOR_ASSERT(ctx->stack->top->subitems > 0);
        _cbor_owed_decrement(ctx);
        if (ctx->stack->top->subitems == 0) {
          cbor_item_t* map_entry = ctx->stack->top->item;
          _cbor_stack_pop(ctx->stack);
          _cbor_builder_append(map_entry, ctx);
        }
      } else {
        ctx->stack->top->subitems ^=
            1; /* Flip the indicator for indefinite items */
      }
      break;
    }
    case CBOR_TYPE_TAG: {
      CBOR_ASSERT(ctx->stack->top->subitems == 1);
      cbor_tag_set_item(ctx->stack->top->item, item);
      cbor_decref(&item); /* Give up on our reference */
      cbor_item_t* tagged_item = ctx->stack->top->item;
      _cbor_stack_pop(ctx->stack);
      _cbor_builder_append(tagged_item, ctx);
      break;
    }
    // We have an item to append but nothing to append it to.
    default: {
      cbor_decref(&item);
      ctx->syntax_error = true;
    }
  }
}

#define CHECK_RES(ctx, res)        \
  do {                             \
    if (res == NULL) {             \
      ctx->creation_failed = true; \
      return;                      \
    }                              \
  } while (0)

// Check that the length fits into size_t. If not, we cannot possibly allocate
// the required memory and should fail fast. On 64-bit platforms size_t and
// uint64_t are the same width so this check is always false; it guards against
// oversized values on 32-bit platforms where SIZE_MAX < UINT64_MAX.
#define CHECK_LENGTH(ctx, length)  \
  do {                             \
    if (length > SIZE_MAX) {       \
      ctx->creation_failed = true; \
      return;                      \
    }                              \
  } while (0)

#define PUSH_CTX_STACK(ctx, res, subitems)                     \
  do {                                                         \
    if (_cbor_stack_push(ctx->stack, res, subitems) == NULL) { \
      cbor_decref(&res);                                       \
      ctx->creation_failed = true;                             \
    }                                                          \
  } while (0)

void cbor_builder_uint8_callback(void* context, uint8_t value) {
  struct _cbor_decoder_context* ctx = context;
  cbor_item_t* res = cbor_new_int8();
  CHECK_RES(ctx, res);
  cbor_mark_uint(res);
  cbor_set_uint8(res, value);
  _cbor_builder_append(res, ctx);
}

void cbor_builder_uint16_callback(void* context, uint16_t value) {
  struct _cbor_decoder_context* ctx = context;
  cbor_item_t* res = cbor_new_int16();
  CHECK_RES(ctx, res);
  cbor_mark_uint(res);
  cbor_set_uint16(res, value);
  _cbor_builder_append(res, ctx);
}

void cbor_builder_uint32_callback(void* context, uint32_t value) {
  struct _cbor_decoder_context* ctx = context;
  cbor_item_t* res = cbor_new_int32();
  CHECK_RES(ctx, res);
  cbor_mark_uint(res);
  cbor_set_uint32(res, value);
  _cbor_builder_append(res, ctx);
}

void cbor_builder_uint64_callback(void* context, uint64_t value) {
  struct _cbor_decoder_context* ctx = context;
  cbor_item_t* res = cbor_new_int64();
  CHECK_RES(ctx, res);
  cbor_mark_uint(res);
  cbor_set_uint64(res, value);
  _cbor_builder_append(res, ctx);
}

void cbor_builder_negint8_callback(void* context, uint8_t value) {
  struct _cbor_decoder_context* ctx = context;
  cbor_item_t* res = cbor_new_int8();
  CHECK_RES(ctx, res);
  cbor_mark_negint(res);
  cbor_set_uint8(res, value);
  _cbor_builder_append(res, ctx);
}

void cbor_builder_negint16_callback(void* context, uint16_t value) {
  struct _cbor_decoder_context* ctx = context;
  cbor_item_t* res = cbor_new_int16();
  CHECK_RES(ctx, res);
  cbor_mark_negint(res);
  cbor_set_uint16(res, value);
  _cbor_builder_append(res, ctx);
}

void cbor_builder_negint32_callback(void* context, uint32_t value) {
  struct _cbor_decoder_context* ctx = context;
  cbor_item_t* res = cbor_new_int32();
  CHECK_RES(ctx, res);
  cbor_mark_negint(res);
  cbor_set_uint32(res, value);
  _cbor_builder_append(res, ctx);
}

void cbor_builder_negint64_callback(void* context, uint64_t value) {
  struct _cbor_decoder_context* ctx = context;
  cbor_item_t* res = cbor_new_int64();
  CHECK_RES(ctx, res);
  cbor_mark_negint(res);
  cbor_set_uint64(res, value);
  _cbor_builder_append(res, ctx);
}

void cbor_builder_byte_string_callback(void* context, cbor_data data,
                                       uint64_t length) {
  struct _cbor_decoder_context* ctx = context;
  CHECK_LENGTH(ctx, length);
  // malloc(0) may legitimately return NULL, so do not allocate for an empty
  // chunk; the handle will be NULL and never dereferenced.
  unsigned char* new_handle = NULL;
  if (length > 0) {
    new_handle = _cbor_malloc(length);
    if (new_handle == NULL) {
      ctx->creation_failed = true;
      return;
    }
    memcpy(new_handle, data, length);
  }
  cbor_item_t* new_chunk = cbor_new_definite_bytestring();

  if (new_chunk == NULL) {
    _cbor_free(new_handle);
    ctx->creation_failed = true;
    return;
  }

  cbor_bytestring_set_handle(new_chunk, new_handle, length);

  // If an indef bytestring is on the stack, extend it (if it were closed, it
  // would have been popped). Handle any syntax errors upstream.
  if (ctx->stack->size > 0 && cbor_isa_bytestring(ctx->stack->top->item) &&
      cbor_bytestring_is_indefinite(ctx->stack->top->item)) {
    if (!cbor_bytestring_add_chunk(ctx->stack->top->item, new_chunk)) {
      ctx->creation_failed = true;
    }
    cbor_decref(&new_chunk);
  } else {
    _cbor_builder_append(new_chunk, ctx);
  }
}

void cbor_builder_byte_string_start_callback(void* context) {
  struct _cbor_decoder_context* ctx = context;
  cbor_item_t* res = cbor_new_indefinite_bytestring();
  CHECK_RES(ctx, res);
  PUSH_CTX_STACK(ctx, res, 0);
}

void cbor_builder_string_callback(void* context, cbor_data data,
                                  uint64_t length) {
  struct _cbor_decoder_context* ctx = context;
  CHECK_LENGTH(ctx, length);
  // malloc(0) may legitimately return NULL, so do not allocate for an empty
  // chunk; the handle will be NULL and never dereferenced.
  unsigned char* new_handle = NULL;
  if (length > 0) {
    new_handle = _cbor_malloc(length);
    if (new_handle == NULL) {
      ctx->creation_failed = true;
      return;
    }
    memcpy(new_handle, data, length);
  }
  cbor_item_t* new_chunk = cbor_new_definite_string();
  if (new_chunk == NULL) {
    _cbor_free(new_handle);
    ctx->creation_failed = true;
    return;
  }
  cbor_string_set_handle(new_chunk, new_handle, length);

  // If an indef string is on the stack, extend it (if it were closed, it would
  // have been popped). Handle any syntax errors upstream.
  if (ctx->stack->size > 0 && cbor_isa_string(ctx->stack->top->item) &&
      cbor_string_is_indefinite(ctx->stack->top->item)) {
    if (!cbor_string_add_chunk(ctx->stack->top->item, new_chunk)) {
      ctx->creation_failed = true;
    }
    cbor_decref(&new_chunk);
  } else {
    _cbor_builder_append(new_chunk, ctx);
  }
}

void cbor_builder_string_start_callback(void* context) {
  struct _cbor_decoder_context* ctx = context;
  cbor_item_t* res = cbor_new_indefinite_string();
  CHECK_RES(ctx, res);
  PUSH_CTX_STACK(ctx, res, 0);
}

void cbor_builder_array_start_callback(void* context, uint64_t size) {
  struct _cbor_decoder_context* ctx = context;
  CHECK_LENGTH(ctx, size);
  // Preallocate only what the input can hold. A header claiming more items
  // than that describes input that will fail to decode; the array grows on
  // demand until it does, so the error is the same as without the cap.
  size_t budget = _cbor_claim_budget(ctx);
  cbor_item_t* res = cbor_new_definite_array(size < budget ? size : budget);
  CHECK_RES(ctx, res);
  if (size > 0) {
    PUSH_CTX_STACK(ctx, res, size);
    ctx->owed += _cbor_owed(ctx, size);
  } else {
    _cbor_builder_append(res, ctx);
  }
}

void cbor_builder_indef_array_start_callback(void* context) {
  struct _cbor_decoder_context* ctx = context;
  cbor_item_t* res = cbor_new_indefinite_array();
  CHECK_RES(ctx, res);
  PUSH_CTX_STACK(ctx, res, 0);
}

void cbor_builder_indef_map_start_callback(void* context) {
  struct _cbor_decoder_context* ctx = context;
  cbor_item_t* res = cbor_new_indefinite_map();
  CHECK_RES(ctx, res);
  PUSH_CTX_STACK(ctx, res, 0);
}

void cbor_builder_map_start_callback(void* context, uint64_t size) {
  struct _cbor_decoder_context* ctx = context;
  CHECK_LENGTH(ctx, size);
  // Each map entry requires two subitems (key + value). Guard against
  // overflow in size * 2 before it is used as the subitems count.
  if (size > SIZE_MAX / 2) {
    ctx->creation_failed = true;
    return;
  }
  // As for arrays; each pair takes at least two bytes.
  size_t budget = _cbor_claim_budget(ctx) / 2;
  cbor_item_t* res = cbor_new_definite_map(size < budget ? size : budget);
  CHECK_RES(ctx, res);
  if (size > 0) {
    PUSH_CTX_STACK(ctx, res, size * 2);
    ctx->owed += _cbor_owed(ctx, size * 2);
  } else {
    _cbor_builder_append(res, ctx);
  }
}

/**
 * Is the (partially constructed) item indefinite?
 */
bool _cbor_is_indefinite(cbor_item_t* item) {
  switch (item->type) {
    case CBOR_TYPE_BYTESTRING:
      return cbor_bytestring_is_indefinite(item);
    case CBOR_TYPE_STRING:
      return cbor_string_is_indefinite(item);
    case CBOR_TYPE_ARRAY:
      return cbor_array_is_indefinite(item);
    case CBOR_TYPE_MAP:
      return cbor_map_is_indefinite(item);
    default:
      // Should never happen since a non-nested item cannot be on top of the
      // stack.
      return false;
  }
}

void cbor_builder_indef_break_callback(void* context) {
  struct _cbor_decoder_context* ctx = context;
  /* There must be an item to break out of*/
  if (ctx->stack->size > 0) {
    cbor_item_t* item = ctx->stack->top->item;
    if (_cbor_is_indefinite(
            item) && /* Only indefinite items can be terminated by 0xFF */
        /* Special case: we cannot append up if an indefinite map is incomplete
           (we are expecting a value). */
        (item->type != CBOR_TYPE_MAP || ctx->stack->top->subitems % 2 == 0)) {
      _cbor_stack_pop(ctx->stack);
      _cbor_builder_append(item, ctx);
      return;
    }
  }

  ctx->syntax_error = true;
}

void cbor_builder_float2_callback(void* context, float value) {
  struct _cbor_decoder_context* ctx = context;
  cbor_item_t* res = cbor_new_float2();
  CHECK_RES(ctx, res);
  cbor_set_float2(res, value);
  _cbor_builder_append(res, ctx);
}

void cbor_builder_float4_callback(void* context, float value) {
  struct _cbor_decoder_context* ctx = context;
  cbor_item_t* res = cbor_new_float4();
  CHECK_RES(ctx, res);
  cbor_set_float4(res, value);
  _cbor_builder_append(res, ctx);
}

void cbor_builder_float8_callback(void* context, double value) {
  struct _cbor_decoder_context* ctx = context;
  cbor_item_t* res = cbor_new_float8();
  CHECK_RES(ctx, res);
  cbor_set_float8(res, value);
  _cbor_builder_append(res, ctx);
}

void cbor_builder_null_callback(void* context) {
  struct _cbor_decoder_context* ctx = context;
  cbor_item_t* res = cbor_new_null();
  CHECK_RES(ctx, res);
  _cbor_builder_append(res, ctx);
}

void cbor_builder_undefined_callback(void* context) {
  struct _cbor_decoder_context* ctx = context;
  cbor_item_t* res = cbor_new_undef();
  CHECK_RES(ctx, res);
  _cbor_builder_append(res, ctx);
}

void cbor_builder_simple_value_callback(void* context, uint8_t value) {
  struct _cbor_decoder_context* ctx = context;
  cbor_item_t* res = cbor_build_ctrl(value);
  CHECK_RES(ctx, res);
  _cbor_builder_append(res, ctx);
}

void cbor_builder_boolean_callback(void* context, bool value) {
  struct _cbor_decoder_context* ctx = context;
  cbor_item_t* res = cbor_build_bool(value);
  CHECK_RES(ctx, res);
  _cbor_builder_append(res, ctx);
}

void cbor_builder_tag_callback(void* context, uint64_t value) {
  struct _cbor_decoder_context* ctx = context;
  cbor_item_t* res = cbor_new_tag(value);
  CHECK_RES(ctx, res);
  PUSH_CTX_STACK(ctx, res, 1);
}
