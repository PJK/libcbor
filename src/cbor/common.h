/*
 * Copyright (c) 2014-2015 Pavel Kalvoda <me@pavelkalvoda.com>
 *
 * libcbor is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef LIBCBOR_COMMON_H
#define LIBCBOR_COMMON_H

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

#endif //LIBCBOR_COMMON_H
