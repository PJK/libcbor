/*
 * Copyright (c) 2014-2015 Pavel Kalvoda <me@pavelkalvoda.com>
 *
 * libcbor is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef LIBCBOR_COMMON_H
#define LIBCBOR_COMMON_H

#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include "data.h"

#define CBOR_MAJOR_VERSION 0
#define CBOR_MINOR_VERSION 0
#define CBOR_PATCH_VERSION 1

#define CBOR_VERSION TO_STR(CBOR_MAJOR_VERSION) "." TO_STR(CBOR_MINOR_VERSION) "." TO_STR(CBOR_PATCH_VERSION)

_Static_assert(sizeof(size_t) >= 8, "size_t must be at least 64 bits"); /* Otherwise we cannot support reasonably sized chunks */

/* http://stackoverflow.com/questions/1644868/c-define-macro-for-debug-printing */
#ifdef DEBUG
#include <stdio.h>
#define debug_print(fmt, ...) do { \
	if (DEBUG) \
		fprintf(stderr, "%s:%d:%s(): " fmt, __FILE__, __LINE__, __func__, __VA_ARGS__); \
	} while (0)
#else
#define debug_print(fmt, ...) do {} while (0)
#endif

#define TO_STR_(x) #x
#define TO_STR(x) TO_STR_(x) /* enables proper double expansion */

#ifdef CBOR_CUSTOM_ALLOC
	typedef void * (* _cbor_malloc_t)(size_t);
	typedef void * (* _cbor_realloc_t)(void *, size_t);
	typedef void (* _cbor_free_t)(void *);

	extern _cbor_malloc_t _cbor_malloc;
	extern _cbor_realloc_t _cbor_realloc;
	extern _cbor_free_t _cbor_free;

	void cbor_set_allocs(_cbor_malloc_t custom_malloc, _cbor_realloc_t custom_realloc, _cbor_free_t custom_free);

	#define _CBOR_MALLOC _cbor_malloc
	#define _CBOR_REALLOC _cbor_realloc
	#define _CBOR_FREE _cbor_free
#else
	#define _CBOR_MALLOC malloc
	#define _CBOR_REALLOC realloc
	#define _CBOR_FREE free
#endif

/*
* ============================================================================
* Type manipulation
* ============================================================================
*/

cbor_type cbor_typeof(const cbor_item_t * item); /* Will be inlined iff link-time opt is enabled */

/* Standard item types as described by the RFC */
bool cbor_isa_uint(const cbor_item_t * item);
bool cbor_isa_negint(const cbor_item_t * item);
bool cbor_isa_bytestring(const cbor_item_t * item);
bool cbor_isa_string(const cbor_item_t * item);
bool cbor_isa_array(const cbor_item_t * item);
bool cbor_isa_map(const cbor_item_t * item);
bool cbor_isa_tag(const cbor_item_t * item);
bool cbor_isa_float_ctrl(const cbor_item_t * item);

/* Practical types with respect to their semantics (but not tag values) */
bool cbor_is_int(const cbor_item_t * item);
bool cbor_is_uint(const cbor_item_t * item);
bool cbor_is_float(const cbor_item_t * item);
bool cbor_is_bool(const cbor_item_t * item);
bool cbor_is_null(const cbor_item_t * item);
bool cbor_is_undef(const cbor_item_t * item);

/*
* ============================================================================
* Memory management
* ============================================================================
*/

cbor_item_t * cbor_incref(cbor_item_t * item);
void cbor_decref(cbor_item_t ** item);
void cbor_intermediate_decref(cbor_item_t * item);
size_t cbor_refcount(const cbor_item_t * item);


#endif //LIBCBOR_COMMON_H
