/*
 * Copyright (c) 2014-2015 Pavel Kalvoda <me@pavelkalvoda.com>
 *
 * libcbor is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef LIBCBOR_DATA_H
#define LIBCBOR_DATA_H

#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef const unsigned char * cbor_data;

typedef enum {             /* Corresponding Major Type */
	CBOR_TYPE_UINT,        /* 0 */
	CBOR_TYPE_NEGINT,      /* 1 */
	CBOR_TYPE_BYTESTRING,  /* 2 */
	CBOR_TYPE_STRING,      /* 3 */
	CBOR_TYPE_ARRAY,       /* 4 */
	CBOR_TYPE_MAP,         /* 5 */
	CBOR_TYPE_TAG,         /* 6 - additional semantics */
	CBOR_TYPE_FLOAT_CTRL   /* 7 - also bool, null, undefined, and others */
} cbor_type;

typedef enum {
	CBOR_ERR_NONE,
	CBOR_ERR_NOTENOUGHDATA,
	CBOR_ERR_NODATA,
	CBOR_ERR_MALFORMATED,
	CBOR_ERR_MEMERROR,		/* Memory error - item allocation failed. Is it too big for your allocator?? */
	CBOR_ERR_SYNTAXERROR	/* Stack parsing algorithm failed */
} cbor_error_code;

/* Possible widths of CBOR_TYPE_UINT */
typedef enum {
	CBOR_INT_8,
	CBOR_INT_16,
	CBOR_INT_32,
	CBOR_INT_64
} cbor_int_width;

typedef enum {
	CBOR_FLOAT_0, /* Registered for internal use - breaks and such */
	CBOR_FLOAT_16,
	CBOR_FLOAT_32,
	CBOR_FLOAT_64
} cbor_float_width;

typedef enum {
	_CBOR_METADATA_DEFINITE,
	_CBOR_METADATA_INDEFINITE
} _cbor_dst_metadata;

typedef enum {
	CBOR_CTRL_NONE = 0,
	CBOR_CTRL_FALSE = 20,
	CBOR_CTRL_TRUE = 21,
	CBOR_CTRL_NULL = 22,
	CBOR_CTRL_UNDEF = 23
} _cbor_ctrl;

struct _cbor_int_metadata {
	cbor_int_width width;
};

struct _cbor_bytestring_metadata {
	size_t length;
	_cbor_dst_metadata type;
};

struct _cbor_string_metadata {
	size_t             length;
	size_t             codepoint_count; /* Sum of chunks' codepoint_counts for indefinite strings */
	_cbor_dst_metadata type;
};

struct _cbor_array_metadata {
	size_t             allocated;
	size_t             end_ptr;
	_cbor_dst_metadata type;
};

struct _cbor_map_metadata {
	size_t             allocated;
	size_t             end_ptr;
	_cbor_dst_metadata type;
};

/* cbor_item_metadata is 2 * sizeof(size_t) + sizeof(_cbor_string_type_metadata), lets use the space */
struct _cbor_tag_metadata {
	struct cbor_item_t * tagged_item;
	uint64_t             value;
};

struct _cbor_float_ctrl_metadata {
	cbor_float_width width;
	uint8_t          ctrl;
};

/* Raw memory casts */
union _cbor_float_helper {
	float    as_float;
	uint32_t as_uint;
};

union _cbor_double_helper {
	double   as_double;
	uint64_t as_uint;
};

union cbor_item_metadata {
	struct _cbor_int_metadata        int_metadata;
	struct _cbor_bytestring_metadata bytestring_metadata;
	struct _cbor_string_metadata     string_metadata;
	struct _cbor_array_metadata      array_metadata;
	struct _cbor_map_metadata        map_metadata;
	struct _cbor_tag_metadata        tag_metadata;
	struct _cbor_float_ctrl_metadata float_ctrl_metadata;
};

typedef struct cbor_item_t {
	union cbor_item_metadata metadata;
	size_t                   refcount;
	cbor_type                type;
	unsigned char *          data;
} cbor_item_t;

struct cbor_indefinite_string_data {
	size_t          chunk_count;
	size_t          chunk_capacity;
	cbor_item_t * * chunks;
};

struct cbor_error {
	size_t          position;
	cbor_error_code code;
};

struct cbor_pair {
	cbor_item_t * key, * value;
};

struct cbor_load_result {
	struct cbor_error error;
	size_t            read;
};


enum cbor_decoder_status {
	CBOR_DECODER_FINISHED,		/* OK, finished */
	CBOR_DECODER_NEDATA,		/* Not enough data - mismatch with MTB */
	CBOR_DECODER_EBUFFER,		/* Buffer manipulation problem */
	CBOR_DECODER_ERROR,			/* Malformed or reserved MTB/value */
};

struct cbor_decoder_result {
	size_t                   read;
	enum cbor_decoder_status status;
};

#ifdef __cplusplus
}
#endif

#endif //LIBCBOR_DATA_H
