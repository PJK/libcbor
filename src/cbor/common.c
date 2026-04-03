/*
 * Copyright (c) 2014-2020 Pavel Kalvoda <me@pavelkalvoda.com>
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

#ifdef DEBUG
bool _cbor_enable_assert = true;
#endif

cbor_type cbor_typeof(const cbor_item_t* item) {
  CBOR_ASSERT(item != NULL);
  CBOR_ASSERT_VALID_TYPE(item->type);
  return item->type;
}

bool cbor_isa_uint(const cbor_item_t* item) {
  return cbor_typeof(item) == CBOR_TYPE_UINT;
}

bool cbor_isa_negint(const cbor_item_t* item) {
  return cbor_typeof(item) == CBOR_TYPE_NEGINT;
}

bool cbor_isa_bytestring(const cbor_item_t* item) {
  return cbor_typeof(item) == CBOR_TYPE_BYTESTRING;
}

bool cbor_isa_string(const cbor_item_t* item) {
  return cbor_typeof(item) == CBOR_TYPE_STRING;
}

bool cbor_isa_array(const cbor_item_t* item) {
  return cbor_typeof(item) == CBOR_TYPE_ARRAY;
}

bool cbor_isa_map(const cbor_item_t* item) {
  return cbor_typeof(item) == CBOR_TYPE_MAP;
}

bool cbor_isa_tag(const cbor_item_t* item) {
  return cbor_typeof(item) == CBOR_TYPE_TAG;
}

bool cbor_isa_float_ctrl(const cbor_item_t* item) {
  return cbor_typeof(item) == CBOR_TYPE_FLOAT_CTRL;
}

bool cbor_is_int(const cbor_item_t* item) {
  return cbor_isa_uint(item) || cbor_isa_negint(item);
}

bool cbor_is_bool(const cbor_item_t* item) {
  return cbor_isa_float_ctrl(item) && cbor_float_ctrl_is_ctrl(item) &&
         (cbor_ctrl_value(item) == CBOR_CTRL_FALSE ||
          cbor_ctrl_value(item) == CBOR_CTRL_TRUE);
}

bool cbor_is_null(const cbor_item_t* item) {
  return cbor_isa_float_ctrl(item) && cbor_float_ctrl_is_ctrl(item) &&
         cbor_ctrl_value(item) == CBOR_CTRL_NULL;
}

bool cbor_is_undef(const cbor_item_t* item) {
  return cbor_isa_float_ctrl(item) && cbor_float_ctrl_is_ctrl(item) &&
         cbor_ctrl_value(item) == CBOR_CTRL_UNDEF;
}

bool cbor_is_float(const cbor_item_t* item) {
  return cbor_isa_float_ctrl(item) && !cbor_float_ctrl_is_ctrl(item);
}

cbor_item_t* cbor_incref(cbor_item_t* item) {
  item->refcount++;
  return item;
}

void cbor_decref(cbor_item_t** item_ref) {
  cbor_item_t* item = *item_ref;
  CBOR_ASSERT(item->refcount > 0);
  if (--item->refcount == 0) {
    switch (item->type) {
      case CBOR_TYPE_UINT:
        /* Fallthrough */
      case CBOR_TYPE_NEGINT:
        /* Combined allocation, freeing the item suffices */
        { break; }
      case CBOR_TYPE_BYTESTRING: {
        if (cbor_bytestring_is_definite(item)) {
          _cbor_free(item->data);
        } else {
          /* We need to decref all chunks */
          cbor_item_t** handle = cbor_bytestring_chunks_handle(item);
          for (size_t i = 0; i < cbor_bytestring_chunk_count(item); i++)
            cbor_decref(&handle[i]);
          _cbor_free(((struct cbor_indefinite_string_data*)item->data)->chunks);
          _cbor_free(item->data);
        }
        break;
      }
      case CBOR_TYPE_STRING: {
        if (cbor_string_is_definite(item)) {
          _cbor_free(item->data);
        } else {
          /* We need to decref all chunks */
          cbor_item_t** handle = cbor_string_chunks_handle(item);
          for (size_t i = 0; i < cbor_string_chunk_count(item); i++)
            cbor_decref(&handle[i]);
          _cbor_free(((struct cbor_indefinite_string_data*)item->data)->chunks);
          _cbor_free(item->data);
        }
        break;
      }
      case CBOR_TYPE_ARRAY: {
        /* Get all items and decref them */
        cbor_item_t** handle = cbor_array_handle(item);
        size_t size = cbor_array_size(item);
        for (size_t i = 0; i < size; i++)
          if (handle[i] != NULL) cbor_decref(&handle[i]);
        _cbor_free(item->data);
        break;
      }
      case CBOR_TYPE_MAP: {
        struct cbor_pair* handle = cbor_map_handle(item);
        for (size_t i = 0; i < item->metadata.map_metadata.end_ptr;
             i++, handle++) {
          cbor_decref(&handle->key);
          if (handle->value != NULL) cbor_decref(&handle->value);
        }
        _cbor_free(item->data);
        break;
      }
      case CBOR_TYPE_TAG: {
        if (item->metadata.tag_metadata.tagged_item != NULL)
          cbor_decref(&item->metadata.tag_metadata.tagged_item);
        _cbor_free(item->data);
        break;
      }
      case CBOR_TYPE_FLOAT_CTRL: {
        /* Floats have combined allocation */
        break;
      }
    }
    _cbor_free(item);
    *item_ref = NULL;
  }
}

void cbor_intermediate_decref(cbor_item_t* item) { cbor_decref(&item); }

size_t cbor_refcount(const cbor_item_t* item) { return item->refcount; }

cbor_item_t* cbor_move(cbor_item_t* item) {
  if (item == NULL) return NULL;
  item->refcount--;
  return item;
}

bool cbor_structurally_equal(const cbor_item_t* item1,
                             const cbor_item_t* item2) {
  CBOR_ASSERT(item1 != NULL);
  CBOR_ASSERT(item2 != NULL);
  if (item1->type != item2->type) return false;

  switch (item1->type) {
    case CBOR_TYPE_UINT:
    case CBOR_TYPE_NEGINT: {
      /* Encoding width (CBOR_INT_8/16/32/64) is part of structural identity */
      if (cbor_int_get_width(item1) != cbor_int_get_width(item2)) return false;
      /* Compare the raw data bytes that back the integer value */
      static const size_t int_widths[] = {1, 2, 4, 8};
      return memcmp(item1->data, item2->data,
                    int_widths[cbor_int_get_width(item1)]) == 0;
    }

    case CBOR_TYPE_BYTESTRING: {
      /* Definite vs. indefinite encoding is structurally distinct */
      if (cbor_bytestring_is_definite(item1) !=
          cbor_bytestring_is_definite(item2))
        return false;
      if (cbor_bytestring_is_definite(item1)) {
        if (cbor_bytestring_length(item1) != cbor_bytestring_length(item2))
          return false;
        return memcmp(item1->data, item2->data,
                      cbor_bytestring_length(item1)) == 0;
      } else {
        /* Indefinite: chunk count and each chunk must match structurally */
        size_t count = cbor_bytestring_chunk_count(item1);
        if (count != cbor_bytestring_chunk_count(item2)) return false;
        cbor_item_t** c1 = cbor_bytestring_chunks_handle(item1);
        cbor_item_t** c2 = cbor_bytestring_chunks_handle(item2);
        for (size_t i = 0; i < count; i++)
          if (!cbor_structurally_equal(c1[i], c2[i])) return false;
        return true;
      }
    }

    case CBOR_TYPE_STRING: {
      /* Definite vs. indefinite encoding is structurally distinct */
      if (cbor_string_is_definite(item1) != cbor_string_is_definite(item2))
        return false;
      if (cbor_string_is_definite(item1)) {
        if (cbor_string_length(item1) != cbor_string_length(item2))
          return false;
        return memcmp(item1->data, item2->data, cbor_string_length(item1)) == 0;
      } else {
        /* Indefinite: chunk boundaries are part of the structure */
        size_t count = cbor_string_chunk_count(item1);
        if (count != cbor_string_chunk_count(item2)) return false;
        cbor_item_t** c1 = cbor_string_chunks_handle(item1);
        cbor_item_t** c2 = cbor_string_chunks_handle(item2);
        for (size_t i = 0; i < count; i++)
          if (!cbor_structurally_equal(c1[i], c2[i])) return false;
        return true;
      }
    }

    case CBOR_TYPE_ARRAY: {
      /* Definite vs. indefinite encoding is structurally distinct */
      if (cbor_array_is_definite(item1) != cbor_array_is_definite(item2))
        return false;
      size_t size = cbor_array_size(item1);
      if (size != cbor_array_size(item2)) return false;
      cbor_item_t** h1 = cbor_array_handle(item1);
      cbor_item_t** h2 = cbor_array_handle(item2);
      for (size_t i = 0; i < size; i++)
        if (!cbor_structurally_equal(h1[i], h2[i])) return false;
      return true;
    }

    case CBOR_TYPE_MAP: {
      /* Definite vs. indefinite encoding is structurally distinct */
      if (cbor_map_is_definite(item1) != cbor_map_is_definite(item2))
        return false;
      size_t size = cbor_map_size(item1);
      if (size != cbor_map_size(item2)) return false;
      /*
       * Structural equality for maps is positional: pair at index i in item1
       * must match pair at index i in item2 (both key and value). This reflects
       * the encoded byte order. Use cbor_map_handle to avoid refcount churn.
       */
      struct cbor_pair* p1 = cbor_map_handle(item1);
      struct cbor_pair* p2 = cbor_map_handle(item2);
      for (size_t i = 0; i < size; i++)
        if (!cbor_structurally_equal(p1[i].key, p2[i].key) ||
            !cbor_structurally_equal(p1[i].value, p2[i].value))
          return false;
      return true;
    }

    case CBOR_TYPE_TAG: {
      if (cbor_tag_value(item1) != cbor_tag_value(item2)) return false;
      /* Access the tagged item directly to avoid refcount churn */
      cbor_item_t* t1 = item1->metadata.tag_metadata.tagged_item;
      cbor_item_t* t2 = item2->metadata.tag_metadata.tagged_item;
      if (t1 == NULL && t2 == NULL) return true;
      if (t1 == NULL || t2 == NULL) return false;
      return cbor_structurally_equal(t1, t2);
    }

    case CBOR_TYPE_FLOAT_CTRL: {
      /* Encoding width (CBOR_FLOAT_0/16/32/64) is part of structural identity
       */
      if (cbor_float_get_width(item1) != cbor_float_get_width(item2))
        return false;
      if (cbor_float_ctrl_is_ctrl(item1)) {
        /* Simple/ctrl values: compare control byte */
        return cbor_ctrl_value(item1) == cbor_ctrl_value(item2);
      } else {
        /*
         * Floats: bitwise comparison of the stored representation. This treats
         * distinct NaN payloads as distinct values, consistent with structural
         * equality. Widths CBOR_FLOAT_16 and CBOR_FLOAT_32 are both backed by
         * 4 bytes (C float); CBOR_FLOAT_64 is backed by 8 bytes (C double).
         */
        size_t sz = (cbor_float_get_width(item1) == CBOR_FLOAT_64) ? 8 : 4;
        return memcmp(item1->data, item2->data, sz) == 0;
      }
    }
  }
  /* Unreachable: all enum values are handled above */
  return false; /* LCOV_EXCL_LINE */
}
