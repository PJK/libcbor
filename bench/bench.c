/*
 * SPDX-FileCopyrightText: 2014-2026 Pavel Kalvoda <me@pavelkalvoda.com>
 * SPDX-License-Identifier: MIT
 */

/*
 * Benchmark harness for performance regression testing. See bench/README.md.
 *
 * Every case is an operation applied to an input. Inputs are the files in
 * bench/data and synthetic shapes generated at startup. The harness only uses
 * the public API, so the same source can be built against any libcbor
 * version that installs a CMake package (0.12.0 and later).
 *
 * The measured code is always called through `bench_measured`, so tools can
 * restrict collection to it, e.g.
 *   valgrind --tool=callgrind --toggle-collect=bench_measured \
 *     libcbor_bench run load/gld_document
 */

#define _POSIX_C_SOURCE 200809L

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "cbor.h"

/* GCC may also clone the function (e.g. bench_measured.constprop.0), which
 * would hide it from --toggle-collect=bench_measured. */
#if defined(__clang__)
#define BENCH_NOINLINE __attribute__((noinline))
#elif defined(__GNUC__)
#define BENCH_NOINLINE __attribute__((noinline, noclone))
#else
#define BENCH_NOINLINE
#endif

#ifndef BENCH_DATA_DIR
#define BENCH_DATA_DIR "data"
#endif

static void die(const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);
  fprintf(stderr, "libcbor_bench: ");
  vfprintf(stderr, fmt, args);
  fprintf(stderr, "\n");
  va_end(args);
  exit(2);
}

#define CHECK(cond)                                                         \
  do {                                                                      \
    if (!(cond)) die("%s:%d: check failed: %s", __FILE__, __LINE__, #cond); \
  } while (0)

static void* xmalloc(size_t size) {
  void* ptr = malloc(size ? size : 1);
  CHECK(ptr != NULL);
  return ptr;
}

static void* xrealloc(void* ptr, size_t size) {
  void* res = realloc(ptr, size ? size : 1);
  CHECK(res != NULL);
  return res;
}

/*
 * ============================================================================
 * Inputs
 * ============================================================================
 */

struct buffer {
  unsigned char* data;
  size_t size;
  size_t capacity;
};

static void buffer_reserve(struct buffer* buf, size_t extra) {
  if (buf->size + extra <= buf->capacity) return;
  size_t capacity = buf->capacity ? buf->capacity : 64;
  while (capacity < buf->size + extra) capacity *= 2;
  buf->data = xrealloc(buf->data, capacity);
  buf->capacity = capacity;
}

static void buffer_append(struct buffer* buf, const void* data, size_t size) {
  buffer_reserve(buf, size);
  memcpy(buf->data + buf->size, data, size);
  buf->size += size;
}

/* Encoders take (value..., buffer, size); this calls one on the buffer tail. */
#define EMIT(buf, call_with_tail)                    \
  do {                                               \
    buffer_reserve((buf), 16);                       \
    unsigned char* tail = (buf)->data + (buf)->size; \
    size_t avail = (buf)->capacity - (buf)->size;    \
    size_t written = call_with_tail;                 \
    CHECK(written > 0);                              \
    (buf)->size += written;                          \
  } while (0)

enum input_flags {
  /* cbor_load fails on this input; only the load operation applies. */
  INPUT_INVALID = 1,
};

struct input {
  char name[64];
  struct buffer bytes;
  int flags;
};

#define MAX_INPUTS 64
static struct input inputs[MAX_INPUTS];
static size_t input_count = 0;

static struct input* new_input(const char* name, int flags) {
  CHECK(input_count < MAX_INPUTS);
  struct input* in = &inputs[input_count++];
  memset(in, 0, sizeof(*in));
  snprintf(in->name, sizeof(in->name), "%s", name);
  in->flags = flags;
  return in;
}

/* Payloads from GLD.SerializerBenchmark, see bench/data/README.md. */
static const char* const data_files[] = {
    "gld_message",   "gld_message_x100",   "gld_document", "gld_document_x100",
    "gld_telemetry", "gld_telemetry_x100", "gld_strings",  "gld_strings_x100",
    "gld_event",     "gld_event_x100",
};

static void load_data_files(const char* dir) {
  for (size_t i = 0; i < sizeof(data_files) / sizeof(data_files[0]); i++) {
    char path[1024];
    snprintf(path, sizeof(path), "%s/%s.cbor", dir, data_files[i]);
    FILE* f = fopen(path, "rb");
    if (!f) die("cannot open %s (use --data DIR)", path);
    struct input* in = new_input(data_files[i], 0);
    unsigned char chunk[4096];
    size_t n;
    while ((n = fread(chunk, 1, sizeof(chunk), f)) > 0)
      buffer_append(&in->bytes, chunk, n);
    CHECK(!ferror(f));
    fclose(f);
  }
}

/* Unsigned integers spread over all four widths. */
static void gen_uints(struct buffer* b) {
  EMIT(b, cbor_encode_array_start(1000, tail, avail));
  for (uint64_t i = 0; i < 1000; i++) {
    switch (i % 4) {
      case 0:
        EMIT(b, cbor_encode_uint8((uint8_t)(i % 200), tail, avail));
        break;
      case 1:
        EMIT(b, cbor_encode_uint16((uint16_t)(1000 + i), tail, avail));
        break;
      case 2:
        EMIT(b, cbor_encode_uint32((uint32_t)(100000 + i), tail, avail));
        break;
      default:
        EMIT(b, cbor_encode_uint64(5000000000ULL + i, tail, avail));
    }
  }
}

static void gen_negint_map(struct buffer* b) {
  EMIT(b, cbor_encode_map_start(500, tail, avail));
  for (uint32_t i = 0; i < 500; i++) {
    EMIT(b, cbor_encode_uint32(i, tail, avail));
    EMIT(b, cbor_encode_negint32(i * 7919, tail, avail));
  }
}

static void gen_strings(struct buffer* b) {
  EMIT(b, cbor_encode_array_start(500, tail, avail));
  for (size_t i = 0; i < 500; i++) {
    char s[32];
    size_t len = 4 + i % 17;
    for (size_t j = 0; j < len; j++) s[j] = (char)('a' + (i + j) % 26);
    EMIT(b, cbor_encode_string_start(len, tail, avail));
    buffer_append(b, s, len);
  }
}

static void gen_floats(struct buffer* b) {
  EMIT(b, cbor_encode_array_start(1000, tail, avail));
  for (size_t i = 0; i < 1000; i++)
    EMIT(b, cbor_encode_double((double)i * 1.25 + 0.1, tail, avail));
}

static void gen_bytes(struct buffer* b) {
  const size_t len = 64 * 1024;
  EMIT(b, cbor_encode_bytestring_start(len, tail, avail));
  buffer_reserve(b, len);
  for (size_t i = 0; i < len; i++) b->data[b->size + i] = (unsigned char)i;
  b->size += len;
}

/* Many tiny nested containers: [[[0], {0: 0}], ...] */
static void gen_tiny_nested(struct buffer* b) {
  EMIT(b, cbor_encode_array_start(500, tail, avail));
  for (size_t i = 0; i < 500; i++) {
    EMIT(b, cbor_encode_array_start(2, tail, avail));
    EMIT(b, cbor_encode_array_start(1, tail, avail));
    EMIT(b, cbor_encode_uint8(0, tail, avail));
    EMIT(b, cbor_encode_map_start(1, tail, avail));
    EMIT(b, cbor_encode_uint8(0, tail, avail));
    EMIT(b, cbor_encode_uint8(0, tail, avail));
  }
}

/* 2000 nested one-element arrays, just under CBOR_MAX_STACK_SIZE (2048). */
static void gen_deep(struct buffer* b) {
  for (size_t i = 0; i < 2000; i++)
    EMIT(b, cbor_encode_array_start(1, tail, avail));
  EMIT(b, cbor_encode_uint8(0, tail, avail));
}

/* Indefinite containers and chunked strings, plus tags and simple values. */
static void gen_indefinite(struct buffer* b) {
  EMIT(b, cbor_encode_indef_array_start(tail, avail));
  for (size_t i = 0; i < 100; i++) {
    EMIT(b, cbor_encode_indef_string_start(tail, avail));
    for (size_t j = 0; j < 2; j++) {
      EMIT(b, cbor_encode_string_start(5, tail, avail));
      buffer_append(b, "chunk", 5);
    }
    EMIT(b, cbor_encode_break(tail, avail));
    EMIT(b, cbor_encode_indef_map_start(tail, avail));
    EMIT(b, cbor_encode_uint8(1, tail, avail));
    EMIT(b, cbor_encode_tag(1, tail, avail));
    EMIT(b, cbor_encode_uint32((uint32_t)(1700000000 + i), tail, avail));
    EMIT(b, cbor_encode_uint8(2, tail, avail));
    EMIT(b, cbor_encode_bool(i % 2 == 0, tail, avail));
    EMIT(b, cbor_encode_uint8(3, tail, avail));
    EMIT(b, cbor_encode_null(tail, avail));
    EMIT(b, cbor_encode_break(tail, avail));
  }
  EMIT(b, cbor_encode_break(tail, avail));
}

/* A 5-byte header declaring 2^28 items (#418). */
static void gen_hostile_claim(struct buffer* b) {
  EMIT(b, cbor_encode_array_start(1u << 28, tail, avail));
}

/* 409 nested headers declaring 2^20 items each (#431). */
static void gen_hostile_nested_claims(struct buffer* b) {
  for (size_t i = 0; i < 409; i++)
    EMIT(b, cbor_encode_array_start(1u << 20, tail, avail));
}

static void generate_inputs(void) {
  static const struct {
    const char* name;
    void (*generate)(struct buffer*);
    int flags;
  } shapes[] = {
      {"uints", gen_uints, 0},
      {"negint_map", gen_negint_map, 0},
      {"strings", gen_strings, 0},
      {"floats", gen_floats, 0},
      {"bytes_64k", gen_bytes, 0},
      {"tiny_nested", gen_tiny_nested, 0},
      {"deep_2000", gen_deep, 0},
      {"indefinite", gen_indefinite, 0},
      {"hostile_claim", gen_hostile_claim, INPUT_INVALID},
      {"hostile_nested_claims", gen_hostile_nested_claims, INPUT_INVALID},
  };
  for (size_t i = 0; i < sizeof(shapes) / sizeof(shapes[0]); i++) {
    struct input* in = new_input(shapes[i].name, shapes[i].flags);
    shapes[i].generate(&in->bytes);
  }
}

/*
 * ============================================================================
 * Event tapes: a recording of the streaming decoder callbacks, replayed into
 * the DOM builders or the streaming encoder without measuring the decoder.
 * ============================================================================
 */

enum event_kind {
  EV_UINT8,
  EV_UINT16,
  EV_UINT32,
  EV_UINT64,
  EV_NEGINT8,
  EV_NEGINT16,
  EV_NEGINT32,
  EV_NEGINT64,
  EV_BYTES,
  EV_BYTES_INDEF,
  EV_STRING,
  EV_STRING_INDEF,
  EV_ARRAY,
  EV_ARRAY_INDEF,
  EV_MAP,
  EV_MAP_INDEF,
  EV_TAG,
  EV_FLOAT2,
  EV_FLOAT4,
  EV_FLOAT8,
  EV_NULL,
  EV_UNDEF,
  EV_BOOL,
  EV_BREAK,
};

struct event {
  enum event_kind kind;
  uint64_t value;
  double real;
  cbor_data data;
};

struct tape {
  struct event* events;
  size_t count;
  size_t capacity;
  /* Number of top-level items. */
  size_t items;
};

static void record(void* ctx, enum event_kind kind, uint64_t value, double real,
                   cbor_data data) {
  struct tape* t = ctx;
  if (t->count == t->capacity) {
    t->capacity = t->capacity ? 2 * t->capacity : 256;
    t->events = xrealloc(t->events, t->capacity * sizeof(struct event));
  }
  t->events[t->count++] = (struct event){kind, value, real, data};
}

static void on_uint8(void* c, uint8_t v) { record(c, EV_UINT8, v, 0, NULL); }
static void on_uint16(void* c, uint16_t v) { record(c, EV_UINT16, v, 0, NULL); }
static void on_uint32(void* c, uint32_t v) { record(c, EV_UINT32, v, 0, NULL); }
static void on_uint64(void* c, uint64_t v) { record(c, EV_UINT64, v, 0, NULL); }
static void on_negint8(void* c, uint8_t v) {
  record(c, EV_NEGINT8, v, 0, NULL);
}
static void on_negint16(void* c, uint16_t v) {
  record(c, EV_NEGINT16, v, 0, NULL);
}
static void on_negint32(void* c, uint32_t v) {
  record(c, EV_NEGINT32, v, 0, NULL);
}
static void on_negint64(void* c, uint64_t v) {
  record(c, EV_NEGINT64, v, 0, NULL);
}
static void on_bytes(void* c, cbor_data d, uint64_t len) {
  record(c, EV_BYTES, len, 0, d);
}
static void on_bytes_indef(void* c) { record(c, EV_BYTES_INDEF, 0, 0, NULL); }
static void on_string(void* c, cbor_data d, uint64_t len) {
  record(c, EV_STRING, len, 0, d);
}
static void on_string_indef(void* c) { record(c, EV_STRING_INDEF, 0, 0, NULL); }
static void on_array(void* c, uint64_t n) { record(c, EV_ARRAY, n, 0, NULL); }
static void on_array_indef(void* c) { record(c, EV_ARRAY_INDEF, 0, 0, NULL); }
static void on_map(void* c, uint64_t n) { record(c, EV_MAP, n, 0, NULL); }
static void on_map_indef(void* c) { record(c, EV_MAP_INDEF, 0, 0, NULL); }
static void on_tag(void* c, uint64_t v) { record(c, EV_TAG, v, 0, NULL); }
static void on_float2(void* c, float v) { record(c, EV_FLOAT2, 0, v, NULL); }
static void on_float4(void* c, float v) { record(c, EV_FLOAT4, 0, v, NULL); }
static void on_float8(void* c, double v) { record(c, EV_FLOAT8, 0, v, NULL); }
static void on_null(void* c) { record(c, EV_NULL, 0, 0, NULL); }
static void on_undef(void* c) { record(c, EV_UNDEF, 0, 0, NULL); }
static void on_bool(void* c, bool v) { record(c, EV_BOOL, v, 0, NULL); }
static void on_break(void* c) { record(c, EV_BREAK, 0, 0, NULL); }

/* Returns false if the input is not a well-formed CBOR sequence. */
static bool record_tape(const struct input* in, struct tape* t) {
  /* Start from the library's own table so this compiles against versions
   * with fewer callback members. Simple values other than false, true, null
   * and undefined are not recorded. */
  struct cbor_callbacks callbacks = cbor_empty_callbacks;
  callbacks.uint8 = on_uint8;
  callbacks.uint16 = on_uint16;
  callbacks.uint32 = on_uint32;
  callbacks.uint64 = on_uint64;
  callbacks.negint8 = on_negint8;
  callbacks.negint16 = on_negint16;
  callbacks.negint32 = on_negint32;
  callbacks.negint64 = on_negint64;
  callbacks.byte_string = on_bytes;
  callbacks.byte_string_start = on_bytes_indef;
  callbacks.string = on_string;
  callbacks.string_start = on_string_indef;
  callbacks.array_start = on_array;
  callbacks.indef_array_start = on_array_indef;
  callbacks.map_start = on_map;
  callbacks.indef_map_start = on_map_indef;
  callbacks.tag = on_tag;
  callbacks.float2 = on_float2;
  callbacks.float4 = on_float4;
  callbacks.float8 = on_float8;
  callbacks.null = on_null;
  callbacks.undefined = on_undef;
  callbacks.boolean = on_bool;
  callbacks.indef_break = on_break;

  memset(t, 0, sizeof(*t));
  size_t offset = 0;
  /* Count top-level items by tracking how many items open containers still
   * expect. Indefinite containers count as SIZE_MAX until their break. */
  size_t depth_pending[4096];
  size_t depth = 0;
  while (offset < in->bytes.size) {
    size_t before = t->count;
    struct cbor_decoder_result res = cbor_stream_decode(
        in->bytes.data + offset, in->bytes.size - offset, &callbacks, t);
    if (res.status != CBOR_DECODER_FINISHED || t->count != before + 1)
      return false;
    offset += res.read;

    const struct event* e = &t->events[before];
    size_t expects = 0;
    switch (e->kind) {
      case EV_ARRAY:
        expects = (size_t)e->value;
        break;
      case EV_MAP:
        expects = (size_t)e->value * 2;
        break;
      case EV_TAG:
        expects = 1;
        break;
      case EV_ARRAY_INDEF:
      case EV_MAP_INDEF:
      case EV_BYTES_INDEF:
      case EV_STRING_INDEF:
        expects = SIZE_MAX;
        break;
      case EV_BREAK:
        if (depth == 0 || depth_pending[depth - 1] != SIZE_MAX) return false;
        depth--;
        break;
      default:
        break;
    }
    if (expects > 0) {
      if (depth == sizeof(depth_pending) / sizeof(depth_pending[0]))
        return false;
      depth_pending[depth++] = expects;
      continue;
    }
    /* The item is complete. It fills a slot of its parent, which may in turn
     * complete the parent. */
    for (;;) {
      if (depth == 0) {
        t->items++;
        break;
      }
      if (depth_pending[depth - 1] == SIZE_MAX) break;
      if (--depth_pending[depth - 1] > 0) break;
      depth--;
    }
  }
  return depth == 0;
}

static cbor_item_t* build_item(const struct tape* t, size_t* pos);

static void build_chunks(const struct tape* t, size_t* pos, cbor_item_t* item,
                         bool (*add)(cbor_item_t*, cbor_item_t*)) {
  while (t->events[*pos].kind != EV_BREAK) {
    cbor_item_t* chunk = build_item(t, pos);
    CHECK(add(item, cbor_move(chunk)));
  }
  (*pos)++;
}

static cbor_item_t* build_item(const struct tape* t, size_t* pos) {
  CHECK(*pos < t->count);
  const struct event* e = &t->events[(*pos)++];
  cbor_item_t* item = NULL;
  switch (e->kind) {
    case EV_UINT8:
      item = cbor_build_uint8((uint8_t)e->value);
      break;
    case EV_UINT16:
      item = cbor_build_uint16((uint16_t)e->value);
      break;
    case EV_UINT32:
      item = cbor_build_uint32((uint32_t)e->value);
      break;
    case EV_UINT64:
      item = cbor_build_uint64(e->value);
      break;
    case EV_NEGINT8:
      item = cbor_build_negint8((uint8_t)e->value);
      break;
    case EV_NEGINT16:
      item = cbor_build_negint16((uint16_t)e->value);
      break;
    case EV_NEGINT32:
      item = cbor_build_negint32((uint32_t)e->value);
      break;
    case EV_NEGINT64:
      item = cbor_build_negint64(e->value);
      break;
    case EV_BYTES:
      item = cbor_build_bytestring(e->data, (size_t)e->value);
      break;
    case EV_BYTES_INDEF:
      item = cbor_new_indefinite_bytestring();
      CHECK(item != NULL);
      build_chunks(t, pos, item, cbor_bytestring_add_chunk);
      break;
    case EV_STRING:
      item = cbor_build_stringn((const char*)e->data, (size_t)e->value);
      break;
    case EV_STRING_INDEF:
      item = cbor_new_indefinite_string();
      CHECK(item != NULL);
      build_chunks(t, pos, item, cbor_string_add_chunk);
      break;
    case EV_ARRAY:
      item = cbor_new_definite_array((size_t)e->value);
      CHECK(item != NULL);
      for (uint64_t i = 0; i < e->value; i++) {
        cbor_item_t* child = build_item(t, pos);
        CHECK(cbor_array_push(item, cbor_move(child)));
      }
      break;
    case EV_ARRAY_INDEF:
      item = cbor_new_indefinite_array();
      CHECK(item != NULL);
      while (t->events[*pos].kind != EV_BREAK) {
        cbor_item_t* child = build_item(t, pos);
        CHECK(cbor_array_push(item, cbor_move(child)));
      }
      (*pos)++;
      break;
    case EV_MAP:
    case EV_MAP_INDEF:
      item = e->kind == EV_MAP ? cbor_new_definite_map((size_t)e->value)
                               : cbor_new_indefinite_map();
      CHECK(item != NULL);
      for (uint64_t i = 0;
           e->kind == EV_MAP ? i < e->value : t->events[*pos].kind != EV_BREAK;
           i++) {
        cbor_item_t* key = build_item(t, pos);
        cbor_item_t* value = build_item(t, pos);
        CHECK(
            cbor_map_add(item, (struct cbor_pair){.key = cbor_move(key),
                                                  .value = cbor_move(value)}));
      }
      if (e->kind == EV_MAP_INDEF) (*pos)++;
      break;
    case EV_TAG: {
      cbor_item_t* tagged = build_item(t, pos);
      item = cbor_build_tag(e->value, tagged);
      cbor_decref(&tagged);
      break;
    }
    case EV_FLOAT2:
      item = cbor_build_float2((float)e->real);
      break;
    case EV_FLOAT4:
      item = cbor_build_float4((float)e->real);
      break;
    case EV_FLOAT8:
      item = cbor_build_float8(e->real);
      break;
    case EV_NULL:
      item = cbor_new_null();
      break;
    case EV_UNDEF:
      item = cbor_new_undef();
      break;
    case EV_BOOL:
      item = cbor_build_bool(e->value != 0);
      break;
    case EV_BREAK:
      die("unexpected break in tape");
  }
  CHECK(item != NULL);
  return item;
}

static size_t encode_tape(const struct tape* t, unsigned char* out,
                          size_t size) {
  size_t offset = 0;
  for (size_t i = 0; i < t->count; i++) {
    const struct event* e = &t->events[i];
    unsigned char* tail = out + offset;
    size_t avail = size - offset;
    size_t written = 0;
    switch (e->kind) {
      case EV_UINT8:
        written = cbor_encode_uint8((uint8_t)e->value, tail, avail);
        break;
      case EV_UINT16:
        written = cbor_encode_uint16((uint16_t)e->value, tail, avail);
        break;
      case EV_UINT32:
        written = cbor_encode_uint32((uint32_t)e->value, tail, avail);
        break;
      case EV_UINT64:
        written = cbor_encode_uint64(e->value, tail, avail);
        break;
      case EV_NEGINT8:
        written = cbor_encode_negint8((uint8_t)e->value, tail, avail);
        break;
      case EV_NEGINT16:
        written = cbor_encode_negint16((uint16_t)e->value, tail, avail);
        break;
      case EV_NEGINT32:
        written = cbor_encode_negint32((uint32_t)e->value, tail, avail);
        break;
      case EV_NEGINT64:
        written = cbor_encode_negint64(e->value, tail, avail);
        break;
      case EV_BYTES:
      case EV_STRING:
        written =
            e->kind == EV_BYTES
                ? cbor_encode_bytestring_start((size_t)e->value, tail, avail)
                : cbor_encode_string_start((size_t)e->value, tail, avail);
        CHECK(written > 0 && avail - written >= e->value);
        memcpy(tail + written, e->data, (size_t)e->value);
        written += (size_t)e->value;
        break;
      case EV_BYTES_INDEF:
        written = cbor_encode_indef_bytestring_start(tail, avail);
        break;
      case EV_STRING_INDEF:
        written = cbor_encode_indef_string_start(tail, avail);
        break;
      case EV_ARRAY:
        written = cbor_encode_array_start((size_t)e->value, tail, avail);
        break;
      case EV_ARRAY_INDEF:
        written = cbor_encode_indef_array_start(tail, avail);
        break;
      case EV_MAP:
        written = cbor_encode_map_start((size_t)e->value, tail, avail);
        break;
      case EV_MAP_INDEF:
        written = cbor_encode_indef_map_start(tail, avail);
        break;
      case EV_TAG:
        written = cbor_encode_tag(e->value, tail, avail);
        break;
      case EV_FLOAT2:
        written = cbor_encode_half((float)e->real, tail, avail);
        break;
      case EV_FLOAT4:
        written = cbor_encode_single((float)e->real, tail, avail);
        break;
      case EV_FLOAT8:
        written = cbor_encode_double(e->real, tail, avail);
        break;
      case EV_NULL:
        written = cbor_encode_null(tail, avail);
        break;
      case EV_UNDEF:
        written = cbor_encode_undef(tail, avail);
        break;
      case EV_BOOL:
        written = cbor_encode_bool(e->value != 0, tail, avail);
        break;
      case EV_BREAK:
        written = cbor_encode_break(tail, avail);
        break;
    }
    CHECK(written > 0);
    offset += written;
  }
  return offset;
}

/*
 * ============================================================================
 * Operations
 * ============================================================================
 */

struct state {
  const struct input* input;
  struct tape tape;
  cbor_item_t** items;
  size_t item_count;
  unsigned char* out;
  size_t out_size;
};

struct operation {
  const char* name;
  const char* description;
  /* Returns false if the operation does not apply to the input. */
  bool (*setup)(struct state*);
  void (*run)(struct state*);
};

static bool setup_valid(struct state* s) {
  return !(s->input->flags & INPUT_INVALID);
}

static bool setup_any(struct state* s) {
  (void)s;
  return true;
}

static bool setup_tape(struct state* s) {
  if (!setup_valid(s) || !record_tape(s->input, &s->tape)) return false;
  s->out_size = 2 * s->input->bytes.size + 1024;
  s->out = xmalloc(s->out_size);
  return true;
}

static bool setup_items(struct state* s) {
  if (!setup_tape(s)) return false;
  s->items = xmalloc(s->tape.items * sizeof(cbor_item_t*));
  size_t pos = 0;
  for (size_t i = 0; i < s->tape.items; i++)
    s->items[s->item_count++] = build_item(&s->tape, &pos);
  return true;
}

static void run_load(struct state* s) {
  const struct buffer* in = &s->input->bytes;
  size_t offset = 0;
  while (offset < in->size) {
    struct cbor_load_result res;
    cbor_item_t* item = cbor_load(in->data + offset, in->size - offset, &res);
    if (item == NULL) {
      CHECK(s->input->flags & INPUT_INVALID);
      return;
    }
    offset += res.read;
    cbor_decref(&item);
  }
  CHECK(!(s->input->flags & INPUT_INVALID));
}

static void run_decode(struct state* s) {
  const struct buffer* in = &s->input->bytes;
  size_t offset = 0;
  while (offset < in->size) {
    struct cbor_decoder_result res = cbor_stream_decode(
        in->data + offset, in->size - offset, &cbor_empty_callbacks, NULL);
    CHECK(res.status == CBOR_DECODER_FINISHED);
    offset += res.read;
  }
}

static void run_serialize(struct state* s) {
  size_t offset = 0;
  for (size_t i = 0; i < s->item_count; i++) {
    size_t written =
        cbor_serialize(s->items[i], s->out + offset, s->out_size - offset);
    CHECK(written > 0);
    offset += written;
  }
}

static void run_build(struct state* s) {
  size_t pos = 0;
  for (size_t i = 0; i < s->tape.items; i++) {
    cbor_item_t* item = build_item(&s->tape, &pos);
    cbor_decref(&item);
  }
}

static void run_encode(struct state* s) {
  CHECK(encode_tape(&s->tape, s->out, s->out_size) > 0);
}

static const struct operation operations[] = {
    {"load", "cbor_load, then cbor_decref", setup_any, run_load},
    {"decode", "cbor_stream_decode with no-op callbacks", setup_valid,
     run_decode},
    {"serialize", "cbor_serialize of a built item", setup_items, run_serialize},
    {"build", "cbor_build_*/cbor_new_* and cbor_decref", setup_tape, run_build},
    {"encode", "cbor_encode_* streaming encoder", setup_tape, run_encode},
};

#define OPERATION_COUNT (sizeof(operations) / sizeof(operations[0]))

static void teardown(struct state* s) {
  for (size_t i = 0; i < s->item_count; i++) cbor_decref(&s->items[i]);
  free(s->items);
  free(s->tape.events);
  free(s->out);
  memset(s, 0, sizeof(*s));
}

/* The only function whose cost the harness reports. Keep it out of line so
 * callgrind can toggle collection on it. */
/* Written after each call so that neither call is a tail call. Callgrind does
 * not see a function entered by a jump. */
static volatile size_t bench_calls = 0;

BENCH_NOINLINE void bench_measured(const struct operation* op, struct state* s,
                                   size_t iterations) {
  for (size_t i = 0; i < iterations; i++) op->run(s);
  bench_calls++;
}

/*
 * ============================================================================
 * Cases
 * ============================================================================
 */

struct bench_case {
  const struct operation* op;
  const struct input* input;
};

static void case_name(const struct bench_case* c, char* out, size_t size) {
  snprintf(out, size, "%.31s/%.63s", c->op->name, c->input->name);
}

/* Calls f for every case that applies and whose name is in names (or every
 * case if names is empty). */
static size_t for_each_case(char** names, int name_count,
                            void (*f)(const struct bench_case*, struct state*,
                                      const char*)) {
  size_t matched = 0;
  for (size_t o = 0; o < OPERATION_COUNT; o++) {
    for (size_t i = 0; i < input_count; i++) {
      struct bench_case c = {&operations[o], &inputs[i]};
      char name[128];
      case_name(&c, name, sizeof(name));
      bool selected = name_count == 0;
      for (int n = 0; n < name_count && !selected; n++)
        selected = strcmp(names[n], name) == 0;
      if (!selected) continue;
      struct state s = {0};
      s.input = c.input;
      if (c.op->setup(&s)) {
        matched++;
        f(&c, &s, name);
      } else if (name_count > 0) {
        die("case %s does not apply", name);
      }
      teardown(&s);
    }
  }
  if (name_count > 0 && matched != (size_t)name_count)
    die("unknown case; see `libcbor_bench list`");
  return matched;
}

static void do_list(const struct bench_case* c, struct state* s,
                    const char* name) {
  (void)s;
  printf("%s\t%s\t%zu bytes\n", name, c->op->description, c->input->bytes.size);
}

static void do_run(const struct bench_case* c, struct state* s,
                   const char* name) {
  (void)name;
  /* Warm up caches, lazy binding and the allocator outside the measurement */
  c->op->run(s);
  bench_measured(c->op, s, 1);
  bench_calls++;
}

/* Counting allocator. Each block carries its size in a header so that live
 * and peak bytes can be tracked. */
static struct {
  size_t allocs;
  size_t reallocs;
  size_t bytes;
  size_t live;
  size_t peak;
} counters;

typedef union {
  size_t size;
  /* Keep the returned blocks aligned for any type. */
  long double align_real;
  uint64_t align_int;
  void* align_ptr;
} alloc_header;

static void* counting_malloc(size_t size) {
  alloc_header* h = malloc(sizeof(alloc_header) + size);
  if (h == NULL) return NULL;
  h->size = size;
  counters.allocs++;
  counters.bytes += size;
  counters.live += size;
  if (counters.live > counters.peak) counters.peak = counters.live;
  // The block is freed through counting_free.
  // cppcheck-suppress memleak
  return h + 1;
}

static void* counting_realloc(void* ptr, size_t size) {
  if (ptr == NULL) return counting_malloc(size);
  alloc_header* h = (alloc_header*)ptr - 1;
  size_t old = h->size;
  alloc_header* res = realloc(h, sizeof(alloc_header) + size);
  if (res == NULL) return NULL;
  res->size = size;
  counters.reallocs++;
  if (size > old) counters.bytes += size - old;
  counters.live = counters.live - old + size;
  if (counters.live > counters.peak) counters.peak = counters.live;
  // The block is freed through counting_free.
  // cppcheck-suppress memleak
  return res + 1;
}

static void counting_free(void* ptr) {
  if (ptr == NULL) return;
  alloc_header* h = (alloc_header*)ptr - 1;
  counters.live -= h->size;
  free(h);
}

static void do_alloc(const struct bench_case* c, struct state* s,
                     const char* name) {
  c->op->run(s);
  size_t live = counters.live;
  counters.allocs = counters.reallocs = counters.bytes = 0;
  counters.peak = live;
  bench_measured(c->op, s, 1);
  printf("%s\tallocs\t%zu\n", name, counters.allocs);
  printf("%s\treallocs\t%zu\n", name, counters.reallocs);
  printf("%s\talloc_bytes\t%zu\n", name, counters.bytes);
  printf("%s\tpeak_bytes\t%zu\n", name, counters.peak - live);
  if (counters.live != live)
    die("%s leaked %zu bytes", name, counters.live - live);
}

static double now_ns(void) {
  struct timespec ts;
  CHECK(clock_gettime(CLOCK_MONOTONIC, &ts) == 0);
  return (double)ts.tv_sec * 1e9 + (double)ts.tv_nsec;
}

static int compare_doubles(const void* a, const void* b) {
  double x = *(const double*)a, y = *(const double*)b;
  return (x > y) - (x < y);
}

#define TIME_SAMPLES 15
#define TIME_SAMPLE_NS 2e7

static void do_time(const struct bench_case* c, struct state* s,
                    const char* name) {
  c->op->run(s);
  /* Pick an iteration count so that one sample takes about 20 ms. */
  size_t iterations = 1;
  for (;;) {
    double start = now_ns();
    bench_measured(c->op, s, iterations);
    double elapsed = now_ns() - start;
    if (elapsed >= TIME_SAMPLE_NS / 4 || iterations >= ((size_t)1 << 26)) break;
    iterations *= 4;
  }
  iterations *= 4;
  double samples[TIME_SAMPLES];
  for (size_t i = 0; i < TIME_SAMPLES; i++) {
    double start = now_ns();
    bench_measured(c->op, s, iterations);
    samples[i] = (now_ns() - start) / (double)iterations;
  }
  qsort(samples, TIME_SAMPLES, sizeof(double), compare_doubles);
  printf("%s\tns_min\t%.1f\n", name, samples[0]);
  printf("%s\tns_median\t%.1f\n", name, samples[TIME_SAMPLES / 2]);
  fflush(stdout);
}

/* Checks the harness itself: the streaming encoder and cbor_serialize must
 * reproduce each valid input byte for byte. */
static bool check_failed = false;

static void do_check(const struct bench_case* c, struct state* s,
                     const char* name) {
  c->op->run(s);
  if (c->op->run != run_encode && c->op->run != run_serialize) return;
  size_t size = 0;
  if (c->op->run == run_encode) {
    size = encode_tape(&s->tape, s->out, s->out_size);
  } else {
    for (size_t i = 0; i < s->item_count; i++)
      size += cbor_serialize(s->items[i], s->out + size, s->out_size - size);
  }
  if (size != s->input->bytes.size ||
      memcmp(s->out, s->input->bytes.data, size) != 0) {
    fprintf(stderr, "%s: output differs from the input\n", name);
    check_failed = true;
  }
}

static void usage(void) {
  fprintf(
      stderr,
      "Usage: libcbor_bench [--data DIR] COMMAND [CASE...]\n"
      "\n"
      "Commands:\n"
      "  list    List cases (name, description, input size)\n"
      "  run     Run each case once after a warmup run, for use under\n"
      "          valgrind --tool=callgrind --toggle-collect=bench_measured\n"
      "  alloc   Print allocation counts, bytes and peak bytes\n"
      "  time    Print the minimum and median wall-clock ns per run\n"
      "  check   Check that encode and serialize reproduce the inputs\n"
      "\n"
      "Without CASE arguments, every case is used. Output is\n"
      "tab-separated: case, metric, value.\n");
  exit(2);
}

int main(int argc, char** argv) {
  const char* data_dir = BENCH_DATA_DIR;
  int arg = 1;
  if (arg + 1 < argc && strcmp(argv[arg], "--data") == 0) {
    data_dir = argv[arg + 1];
    arg += 2;
  }
  if (arg >= argc) usage();
  const char* command = argv[arg++];
  char** names = argv + arg;
  int name_count = argc - arg;

  void (*f)(const struct bench_case*, struct state*, const char*) = NULL;
  if (strcmp(command, "list") == 0) {
    f = do_list;
  } else if (strcmp(command, "run") == 0) {
    f = do_run;
  } else if (strcmp(command, "alloc") == 0) {
    cbor_set_allocs(counting_malloc, counting_realloc, counting_free);
    f = do_alloc;
  } else if (strcmp(command, "time") == 0) {
    f = do_time;
  } else if (strcmp(command, "check") == 0) {
    f = do_check;
  } else {
    usage();
  }

  load_data_files(data_dir);
  generate_inputs();
  for_each_case(names, name_count, f);
  return check_failed ? 1 : 0;
}
