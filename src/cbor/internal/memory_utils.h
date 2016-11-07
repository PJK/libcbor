/*
 * Copyright (c) 2014-2016 Pavel Kalvoda <me@pavelkalvoda.com>
 *
 * libcbor is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef LIBCBOR_MEMORY_UTILS_H
#define LIBCBOR_MEMORY_UTILS_H

#include <string.h>

/** Overflow-proof contiguous array allocation
 *
 * @param item_size
 * @param item_count
 * @return Region of item_size * item_count bytes, or NULL if the total size overflows size_t or the underlying allocator failed
 */
void * _cbor_alloc_multiple(size_t item_size, size_t item_count);

#endif //LIBCBOR_MEMORY_UTILS_H
