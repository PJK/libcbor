/*
 * SPDX-FileCopyrightText: libcbor contributors
 * SPDX-License-Identifier: MIT
 */

#include <string.h>

#include "assertions.h"
#include "cbor.h"

static size_t generate_overflow_data(unsigned char** overflow_data) {
  int i;
  *overflow_data = (unsigned char*)malloc(CBOR_MAX_STACK_SIZE + 3);
  assert_non_null(*overflow_data);
  for (i = 0; i < CBOR_MAX_STACK_SIZE + 1; i++) {
    (*overflow_data)[i] = 0xC2;  // tag of positive bignum
  }
  (*overflow_data)[CBOR_MAX_STACK_SIZE + 1] = 0x41;  // bytestring of length 1
  (*overflow_data)[CBOR_MAX_STACK_SIZE + 2] = 0x01;  // a bignum of value 1
  return CBOR_MAX_STACK_SIZE + 3;
}

static void test_stack_over_limit(void** _state _CBOR_UNUSED) {
  unsigned char* overflow_data;
  size_t overflow_data_len;
  struct cbor_load_result res;
  overflow_data_len = generate_overflow_data(&overflow_data);
  assert_null(cbor_load(overflow_data, overflow_data_len, &res));
  free(overflow_data);
  assert_size_equal(res.error.code, CBOR_ERR_MEMERROR);
}

// Regression test for https://github.com/PJK/libcbor/issues/464:
// Exact CBOR_MAX_STACK_SIZE nested definite arrays of size 1 unwind all levels
// at once when the leaf item is appended.
static void test_definite_arrays_at_stack_limit(void** _state _CBOR_UNUSED) {
  const size_t depth = CBOR_MAX_STACK_SIZE;
  const size_t buf_len = depth + 2;
  unsigned char* buf = (unsigned char*)malloc(buf_len);
  assert_non_null(buf);
  memset(buf, 0x81, depth);  // array(1)
  buf[depth] = 0x18;         // uint8(42)
  buf[depth + 1] = 0x2A;

  struct cbor_load_result res;
  cbor_item_t* root = cbor_load(buf, buf_len, &res);
  free(buf);
  assert_non_null(root);
  assert_size_equal(res.error.code, CBOR_ERR_NONE);
  assert_size_equal(res.read, buf_len);

  cbor_item_t* cur = root;
  for (size_t i = 0; i < depth; i++) {
    assert_true(cbor_isa_array(cur));
    assert_true(cbor_array_is_definite(cur));
    assert_size_equal(cbor_array_size(cur), 1);
    cur = cbor_array_handle(cur)[0];
  }
  assert_uint8(cur, 42);

  cbor_decref(&root);
  assert_null(root);

  // One level deeper must be rejected with CBOR_ERR_MEMERROR.
  const size_t over_len = depth + 3;
  unsigned char* over_buf = (unsigned char*)malloc(over_len);
  assert_non_null(over_buf);
  memset(over_buf, 0x81, depth + 1);
  over_buf[depth + 1] = 0x18;
  over_buf[depth + 2] = 0x2A;
  assert_null(cbor_load(over_buf, over_len, &res));
  free(over_buf);
  assert_size_equal(res.error.code, CBOR_ERR_MEMERROR);
}

// Nested definite size-1 maps in value position: {0: {0: ... {0: 1}...}}.
// Exercises full-depth iterative unwinding in _cbor_builder_append and
// value-branch parent stashing in _cbor_decref_free.
static void test_definite_maps_value_at_stack_limit(
    void** _state _CBOR_UNUSED) {
  const size_t depth = CBOR_MAX_STACK_SIZE;
  const size_t buf_len = depth * 2 + 1;
  unsigned char* buf = (unsigned char*)malloc(buf_len);
  assert_non_null(buf);
  for (size_t i = 0; i < depth; i++) {
    buf[i * 2] = 0xA1;      // map(1)
    buf[i * 2 + 1] = 0x00;  // key: 0
  }
  buf[depth * 2] = 0x01;  // innermost value: 1

  struct cbor_load_result res;
  cbor_item_t* root = cbor_load(buf, buf_len, &res);
  free(buf);
  assert_non_null(root);
  assert_size_equal(res.error.code, CBOR_ERR_NONE);
  assert_size_equal(res.read, buf_len);

  cbor_item_t* cur = root;
  for (size_t i = 0; i < depth; i++) {
    assert_true(cbor_isa_map(cur));
    assert_true(cbor_map_is_definite(cur));
    assert_size_equal(cbor_map_size(cur), 1);
    struct cbor_pair* pairs = cbor_map_handle(cur);
    assert_uint8(pairs[0].key, 0);
    cur = pairs[0].value;
  }
  assert_uint8(cur, 1);

  cbor_decref(&root);
  assert_null(root);

  // One level deeper must be rejected with CBOR_ERR_MEMERROR.
  const size_t over_len = (depth + 1) * 2 + 1;
  unsigned char* over_buf = (unsigned char*)malloc(over_len);
  assert_non_null(over_buf);
  for (size_t i = 0; i < depth + 1; i++) {
    over_buf[i * 2] = 0xA1;
    over_buf[i * 2 + 1] = 0x00;
  }
  over_buf[(depth + 1) * 2] = 0x01;
  assert_null(cbor_load(over_buf, over_len, &res));
  free(over_buf);
  assert_size_equal(res.error.code, CBOR_ERR_MEMERROR);
}

// Nested definite size-1 maps in key position: {{{{1: 2}: 2}: ...}: 2}.
// Exercises partial unwinding (stopping when completed child becomes the outer
// map's key with subitems == 1) and key-branch parent stashing in
// _cbor_decref_free.
static void test_definite_maps_key_at_stack_limit(void** _state _CBOR_UNUSED) {
  const size_t depth = CBOR_MAX_STACK_SIZE;
  const size_t buf_len = depth * 2 + 1;
  unsigned char* buf = (unsigned char*)malloc(buf_len);
  assert_non_null(buf);
  memset(buf, 0xA1, depth);              // depth nested map(1) headers
  buf[depth] = 0x01;                     // innermost key: 1
  memset(buf + depth + 1, 0x02, depth);  // value 2 for each map

  struct cbor_load_result res;
  cbor_item_t* root = cbor_load(buf, buf_len, &res);
  free(buf);
  assert_non_null(root);
  assert_size_equal(res.error.code, CBOR_ERR_NONE);
  assert_size_equal(res.read, buf_len);

  cbor_item_t* cur = root;
  for (size_t i = 0; i < depth; i++) {
    assert_true(cbor_isa_map(cur));
    assert_true(cbor_map_is_definite(cur));
    assert_size_equal(cbor_map_size(cur), 1);
    struct cbor_pair* pairs = cbor_map_handle(cur);
    assert_uint8(pairs[0].value, 2);
    cur = pairs[0].key;
  }
  assert_uint8(cur, 1);

  cbor_decref(&root);
  assert_null(root);
}

// Exact CBOR_MAX_STACK_SIZE nested tags wrapping a 1-byte bytestring.
static void test_tags_at_stack_limit(void** _state _CBOR_UNUSED) {
  const size_t depth = CBOR_MAX_STACK_SIZE;
  const size_t buf_len = depth + 2;
  unsigned char* buf = (unsigned char*)malloc(buf_len);
  assert_non_null(buf);
  memset(buf, 0xC2, depth);
  buf[depth] = 0x41;
  buf[depth + 1] = 0x01;

  struct cbor_load_result res;
  cbor_item_t* root = cbor_load(buf, buf_len, &res);
  free(buf);
  assert_non_null(root);
  assert_size_equal(res.error.code, CBOR_ERR_NONE);
  assert_size_equal(res.read, buf_len);

  cbor_item_t* cur = root;
  for (size_t i = 0; i < depth; i++) {
    assert_true(cbor_isa_tag(cur));
    assert_true(cbor_tag_value(cur) == 2);
    cbor_item_t* next = cbor_tag_item(cur);
    cbor_decref(&next);  // Balance cbor_tag_item's incref; parent owns it
    cur = next;
  }
  assert_true(cbor_isa_bytestring(cur));
  assert_size_equal(cbor_bytestring_length(cur), 1);
  assert_int_equal(cbor_bytestring_handle(cur)[0], 0x01);

  cbor_decref(&root);
  assert_null(root);
}

// Exercises cbor_builder_indef_break_callback unwinding CBOR_MAX_STACK_SIZE - 1
// enclosing definite arrays, plus empty definite array/map leaves at max depth.
static void test_indef_break_and_empty_containers_at_stack_limit(
    void** _state _CBOR_UNUSED) {
  const size_t depth = CBOR_MAX_STACK_SIZE;

  // (depth - 1) definite array(1)s wrapping [_ 7]
  const size_t indef_len = depth + 2;
  unsigned char* indef_buf = (unsigned char*)malloc(indef_len);
  assert_non_null(indef_buf);
  memset(indef_buf, 0x81, depth - 1);
  indef_buf[depth - 1] = 0x9F;  // indef array start (stack reaches depth)
  indef_buf[depth] = 0x07;      // element: 7
  indef_buf[depth + 1] = 0xFF;  // break -> unwinds all enclosing arrays

  struct cbor_load_result res;
  cbor_item_t* root = cbor_load(indef_buf, indef_len, &res);
  free(indef_buf);
  assert_non_null(root);
  assert_size_equal(res.error.code, CBOR_ERR_NONE);

  cbor_item_t* cur = root;
  for (size_t i = 0; i < depth - 1; i++) {
    assert_true(cbor_array_is_definite(cur));
    cur = cbor_array_handle(cur)[0];
  }
  assert_true(cbor_array_is_indefinite(cur));
  assert_size_equal(cbor_array_size(cur), 1);
  assert_uint8(cbor_array_handle(cur)[0], 7);
  cbor_decref(&root);

  // Empty definite array (0x80) and empty definite map (0xA0) do not push onto
  // the builder stack, so they are valid leaves at stack depth == limit.
  const unsigned char leaves[2] = {0x80, 0xA0};
  for (size_t k = 0; k < 2; k++) {
    unsigned char* buf = (unsigned char*)malloc(depth + 1);
    assert_non_null(buf);
    memset(buf, 0x81, depth);
    buf[depth] = leaves[k];
    root = cbor_load(buf, depth + 1, &res);
    free(buf);
    assert_non_null(root);
    assert_size_equal(res.error.code, CBOR_ERR_NONE);
    cbor_decref(&root);
  }
}

// Exercises cbor_load's error-cleanup path on deep partially-populated trees:
// 1) Outer array(2) whose first element is a (CBOR_MAX_STACK_SIZE - 1)-deep
//    completed array chain and whose second element is missing.
// 2) Deep map chain truncated right after a key is added (pair.value == NULL).
static void test_truncated_deep_containers_cleanup(void** _state _CBOR_UNUSED) {
  const size_t depth = CBOR_MAX_STACK_SIZE;
  struct cbor_load_result res;

  unsigned char* arr_buf = (unsigned char*)malloc(depth + 1);
  assert_non_null(arr_buf);
  arr_buf[0] = 0x82;  // array(2)
  memset(arr_buf + 1, 0x81, depth - 1);
  arr_buf[depth] = 0x00;  // completes element 0; element 1 is missing
  assert_null(cbor_load(arr_buf, depth + 1, &res));
  free(arr_buf);
  assert_size_equal(res.error.code, CBOR_ERR_NOTENOUGHDATA);

  unsigned char* map_buf = (unsigned char*)malloc(depth + 1);
  assert_non_null(map_buf);
  memset(map_buf, 0xA1, depth);
  map_buf[depth] = 0x01;  // innermost key added; value missing
  assert_null(cbor_load(map_buf, depth + 1, &res));
  free(map_buf);
  assert_size_equal(res.error.code, CBOR_ERR_NOTENOUGHDATA);
}

static size_t live_allocations = 0;

static void* counting_malloc(size_t size) {
  if (size == 0) return NULL;
  void* ptr = malloc(size);
  if (ptr != NULL) {
    live_allocations++;
  }
  return ptr;
}

static void* counting_realloc(void* ptr, size_t size) {
  if (size == 0) {
    if (ptr != NULL) {
      assert_true(live_allocations > 0);
      live_allocations--;
      free(ptr);
    }
    return NULL;
  }
  void* new_ptr = realloc(ptr, size);
  if (ptr == NULL && new_ptr != NULL) {
    live_allocations++;
  }
  return new_ptr;
}

static void counting_free(void* ptr) {
  if (ptr != NULL) {
    assert_true(live_allocations > 0);
    live_allocations--;
    free(ptr);
  }
}

// Programmatically builds a 65,536-deep structure (32x CBOR_MAX_STACK_SIZE)
// mixing definite/indefinite arrays, definite/indefinite maps (both key and
// value positions), tags, indefinite strings/bytestrings with multiple chunks,
// and shared-refcount items, then verifies cbor_decref frees every allocation
// in O(1) stack space.
static void test_deep_decref_mixed_containers_and_shared_refs(
    void** _state _CBOR_UNUSED) {
  const size_t depth = 65536;
  live_allocations = 0;
  cbor_set_allocs(counting_malloc, counting_realloc, counting_free);

  // Check empty tag and empty definite/indefinite items first.
  cbor_item_t* empty_tag = cbor_new_tag(0);
  assert_non_null(empty_tag);
  cbor_decref(&empty_tag);
  cbor_item_t* empty_indef_str = cbor_new_indefinite_string();
  assert_non_null(empty_indef_str);
  cbor_decref(&empty_indef_str);
  cbor_item_t* empty_indef_bstr = cbor_new_indefinite_bytestring();
  assert_non_null(empty_indef_bstr);
  cbor_decref(&empty_indef_bstr);
  assert_size_equal(live_allocations, 0);

  // Shared items referenced repeatedly throughout the deep tree; their
  // refcounts must drop back to 1 when the tree is freed, not 0.
  cbor_item_t* shared_int = cbor_build_uint32(0xDEADBEEF);
  cbor_item_t* shared_str_chunk = cbor_build_string("shared");
  const unsigned char raw_bytes[] = {0xAA, 0xBB};
  cbor_item_t* shared_bstr_chunk =
      cbor_build_bytestring(raw_bytes, sizeof(raw_bytes));
  assert_non_null(shared_int);
  assert_non_null(shared_str_chunk);
  assert_non_null(shared_bstr_chunk);

  cbor_item_t* cur = cbor_build_uint8(99);
  assert_non_null(cur);

  for (size_t i = 0; i < depth; i++) {
    cbor_item_t* next = NULL;
    switch (i % 6) {
      case 0: {
        next = cbor_new_definite_array(2);
        assert_non_null(next);
        assert_true(cbor_array_push(next, shared_int));
        assert_true(cbor_array_push(next, cbor_move(cur)));
        break;
      }
      case 1: {
        next = cbor_new_indefinite_array();
        assert_non_null(next);
        assert_true(cbor_array_push(next, cbor_move(cur)));
        break;
      }
      case 2: {
        next = cbor_new_definite_map(1);
        assert_non_null(next);
        cbor_item_t* k = cbor_build_uint8((uint8_t)(i & 0xFF));
        assert_non_null(k);
        assert_true(cbor_map_add(
            next,
            (struct cbor_pair){.key = cbor_move(k), .value = cbor_move(cur)}));
        break;
      }
      case 3: {
        next = cbor_new_indefinite_map();
        assert_non_null(next);
        assert_true(cbor_map_add(
            next,
            (struct cbor_pair){.key = cbor_move(cur), .value = shared_int}));
        break;
      }
      case 4: {
        next = cbor_build_tag(i, cbor_move(cur));
        assert_non_null(next);
        break;
      }
      case 5: {
        if (i % 512 == 5) {
          next = cbor_new_definite_array(3);
          assert_non_null(next);

          cbor_item_t* istr = cbor_new_indefinite_string();
          cbor_item_t* owned_str = cbor_build_string("owned");
          assert_non_null(istr);
          assert_non_null(owned_str);
          assert_true(cbor_string_add_chunk(istr, shared_str_chunk));
          assert_true(cbor_string_add_chunk(istr, cbor_move(owned_str)));

          cbor_item_t* ibstr = cbor_new_indefinite_bytestring();
          cbor_item_t* owned_bstr =
              cbor_build_bytestring(raw_bytes, sizeof(raw_bytes));
          assert_non_null(ibstr);
          assert_non_null(owned_bstr);
          assert_true(cbor_bytestring_add_chunk(ibstr, cbor_move(owned_bstr)));
          assert_true(cbor_bytestring_add_chunk(ibstr, shared_bstr_chunk));

          assert_true(cbor_array_push(next, cbor_move(istr)));
          assert_true(cbor_array_push(next, cbor_move(ibstr)));
          assert_true(cbor_array_push(next, cbor_move(cur)));
        } else {
          next = cbor_new_definite_array(1);
          assert_non_null(next);
          assert_true(cbor_array_push(next, cbor_move(cur)));
        }
        break;
      }
    }
    cur = next;
  }

  assert_true(cbor_refcount(shared_int) > 1);
  assert_true(cbor_refcount(shared_str_chunk) > 1);
  assert_true(cbor_refcount(shared_bstr_chunk) > 1);

  cbor_decref(&cur);
  assert_null(cur);

  // Every reference held inside the 65,536-level tree must have been released,
  // leaving only our local handles.
  assert_size_equal(cbor_refcount(shared_int), 1);
  assert_size_equal(cbor_refcount(shared_str_chunk), 1);
  assert_size_equal(cbor_refcount(shared_bstr_chunk), 1);

  cbor_decref(&shared_int);
  cbor_decref(&shared_str_chunk);
  cbor_decref(&shared_bstr_chunk);
  assert_size_equal(live_allocations, 0);

  cbor_set_allocs(malloc, realloc, free);
}

int main(void) {
  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test_stack_over_limit),
      cmocka_unit_test(test_definite_arrays_at_stack_limit),
      cmocka_unit_test(test_definite_maps_value_at_stack_limit),
      cmocka_unit_test(test_definite_maps_key_at_stack_limit),
      cmocka_unit_test(test_tags_at_stack_limit),
      cmocka_unit_test(test_indef_break_and_empty_containers_at_stack_limit),
      cmocka_unit_test(test_truncated_deep_containers_cleanup),
      cmocka_unit_test(test_deep_decref_mixed_containers_and_shared_refs),
  };
  return cmocka_run_group_tests(tests, NULL, NULL);
}
