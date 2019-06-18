/*
 * Copyright (c) 2014-2019 Pavel Kalvoda <me@pavelkalvoda.com>
 *
 * libcbor is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "cbor/common.h"
#include "arrays.h"
#include "bytestrings.h"
#include "data.h"
#include "floats_ctrls.h"
#include "ints.h"
#include "maps.h"
#include "strings.h"
#include "tags.h"

bool cbor_isa_uint(const cbor_item_t *item) {
  return item->type == CBOR_TYPE_UINT;
}

bool cbor_isa_negint(const cbor_item_t *item) {
  return item->type == CBOR_TYPE_NEGINT;
}

bool cbor_isa_bytestring(const cbor_item_t *item) {
  return item->type == CBOR_TYPE_BYTESTRING;
}

bool cbor_isa_string(const cbor_item_t *item) {
  return item->type == CBOR_TYPE_STRING;
}

bool cbor_isa_array(const cbor_item_t *item) {
  return item->type == CBOR_TYPE_ARRAY;
}

bool cbor_isa_map(const cbor_item_t *item) {
  return item->type == CBOR_TYPE_MAP;
}

bool cbor_isa_tag(const cbor_item_t *item) {
  return item->type == CBOR_TYPE_TAG;
}

bool cbor_isa_float_ctrl(const cbor_item_t *item) {
  return item->type == CBOR_TYPE_FLOAT_CTRL;
}

cbor_type cbor_typeof(const cbor_item_t *item) { return item->type; }

bool cbor_is_int(const cbor_item_t *item) {
  return cbor_isa_uint(item) || cbor_isa_negint(item);
}

bool cbor_is_bool(const cbor_item_t *item) {
  return cbor_isa_float_ctrl(item) &&
         (cbor_ctrl_value(item) == CBOR_CTRL_FALSE ||
          cbor_ctrl_value(item) == CBOR_CTRL_TRUE);
}

bool cbor_is_null(const cbor_item_t *item) {
  return cbor_isa_float_ctrl(item) && cbor_ctrl_value(item) == CBOR_CTRL_NULL;
}

bool cbor_is_undef(const cbor_item_t *item) {
  return cbor_isa_float_ctrl(item) && cbor_ctrl_value(item) == CBOR_CTRL_UNDEF;
}

bool cbor_is_float(const cbor_item_t *item) {
  return cbor_isa_float_ctrl(item) && !cbor_float_ctrl_is_ctrl(item);
}

cbor_item_t *cbor_incref(cbor_item_t *item) {
  item->refcount++;
  return item;
}

void cbor_decref(cbor_item_t **item_ref) {
  cbor_item_t *item = *item_ref;
  assert(item->refcount > 0);
  if (--item->refcount == 0) {
    switch (item->type) {
      case CBOR_TYPE_UINT:
        /* Fallthrough */
      case CBOR_TYPE_NEGINT:
        /* Combined allocation, freeing the item suffices */
        { break; }
      case CBOR_TYPE_BYTESTRING: {
        if (cbor_bytestring_is_definite(item)) {
          _CBOR_FREE(item->data);
        } else {
          /* We need to decref all chunks */
          cbor_item_t **handle = cbor_bytestring_chunks_handle(item);
          for (size_t i = 0; i < cbor_bytestring_chunk_count(item); i++)
            cbor_decref(&handle[i]);
          _CBOR_FREE(
              ((struct cbor_indefinite_string_data *)item->data)->chunks);
          _CBOR_FREE(item->data);
        }
        break;
      }
      case CBOR_TYPE_STRING: {
        if (cbor_string_is_definite(item)) {
          _CBOR_FREE(item->data);
        } else {
          /* We need to decref all chunks */
          cbor_item_t **handle = cbor_string_chunks_handle(item);
          for (size_t i = 0; i < cbor_string_chunk_count(item); i++)
            cbor_decref(&handle[i]);
          _CBOR_FREE(
              ((struct cbor_indefinite_string_data *)item->data)->chunks);
          _CBOR_FREE(item->data);
        }
        break;
      }
      case CBOR_TYPE_ARRAY: {
        /* Get all items and decref them */
        cbor_item_t **handle = cbor_array_handle(item);
        size_t size = cbor_array_size(item);
        for (size_t i = 0; i < size; i++)
          if (handle[i] != NULL) cbor_decref(&handle[i]);
        _CBOR_FREE(item->data);
        break;
      }
      case CBOR_TYPE_MAP: {
        struct cbor_pair *handle = cbor_map_handle(item);
        for (size_t i = 0; i < item->metadata.map_metadata.end_ptr;
             i++, handle++) {
          cbor_decref(&handle->key);
          if (handle->value != NULL) cbor_decref(&handle->value);
        }
        _CBOR_FREE(item->data);
        break;
      };
      case CBOR_TYPE_TAG: {
        if (item->metadata.tag_metadata.tagged_item != NULL)
          cbor_decref(&item->metadata.tag_metadata.tagged_item);
        _CBOR_FREE(item->data);
        break;
      }
      case CBOR_TYPE_FLOAT_CTRL: {
        /* Floats have combined allocation */
        break;
      }
    }
    _CBOR_FREE(item);
    // TODO
    *item_ref = NULL;
  }
}

void cbor_intermediate_decref(cbor_item_t *item) { cbor_decref(&item); }

size_t cbor_refcount(const cbor_item_t *item) { return item->refcount; }

cbor_item_t *cbor_move(cbor_item_t *item) {
  item->refcount--;
  return item;
}

bool cbor_equal(cbor_item_t *item1, cbor_item_t *item2) {
  size_t i;
  
  if (item1 != NULL && item2 != NULL) {
    if (item1->type != item2->type) {
      return false;
    } else {
      switch (item1->type) {
        case CBOR_TYPE_UINT:
        case CBOR_TYPE_NEGINT:
          if (cbor_int_get_width(item1) != cbor_int_get_width(item2)) {
            return false;
          } else {
            if (cbor_int_get_width(item1) == CBOR_INT_8)
              return !memcmp(item1->data, item2->data, sizeof(uint8_t))?true:false;
            else if (cbor_int_get_width(item1) == CBOR_INT_16)
              return !memcmp(item1->data, item2->data, sizeof(uint16_t))?true:false;
            else if (cbor_int_get_width(item1) == CBOR_INT_32)
              return !memcmp(item1->data, item2->data, sizeof(uint32_t))?true:false;
            else if (cbor_int_get_width(item1) == CBOR_INT_64)
              return !memcmp(item1->data, item2->data, sizeof(uint64_t))?true:false;
            else
              return false;
          }
          break;
        case CBOR_TYPE_BYTESTRING:
          if (cbor_bytestring_length(item1) != cbor_bytestring_length(item2))
            return false;
          else
            return !memcmp(item1->data, item2->data, cbor_bytestring_length(item1))?true:false;
          break;
        case CBOR_TYPE_STRING:
          if (cbor_string_length(item1) != cbor_string_length(item2))
            return false;
          else
            return !memcmp(item1->data, item2->data, cbor_bytestring_length(item1))?true:false;
          break;
        case CBOR_TYPE_ARRAY:
          if (cbor_array_size(item1) != cbor_array_size(item2)) {
            return false;
          } else {
            for (i=0; i<cbor_array_size(item1); i++) {
              if (cbor_equal(cbor_array_get(item1, i), cbor_array_get(item2, i)) == false)
                return false;
            }
            return true;
          }
          break;
        case CBOR_TYPE_MAP:
          if (cbor_map_size(item1) != cbor_map_size(item2)) {
            return false;
          } else {
            for (i=0; i<cbor_map_size(item1); i++) {
              struct cbor_pair pair1 = cbor_map_handle(item1)[i];
              struct cbor_pair pair2 = cbor_map_handle(item2)[i];
              if (cbor_equal(pair1.key, pair2.key) == false || cbor_equal(pair1.value, pair2.value) == false)
                return false;
            }
            return true;
          }
          break;
        case CBOR_TYPE_TAG:
          if (cbor_tag_value(item1) != cbor_tag_value(item2))
            return false;
          else
            return cbor_equal(cbor_tag_item(item1), cbor_tag_item(item2));
          break;
        case CBOR_TYPE_FLOAT_CTRL:
          if (cbor_float_get_width(item1) != cbor_float_get_width(item2))
            return false;
          else
            return cbor_float_get_float(item1)==cbor_float_get_float(item2)?true:false;
          break;
      }
    }
  } else if (item1 == NULL && item2 == NULL) {
    return true;
  } else {
    return false;
  }
}
