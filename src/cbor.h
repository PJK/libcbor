/*
 * Copyright (c) 2014-2015 Pavel Kalvoda <me@pavelkalvoda.com>
 *
 * libcbor is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */


#ifndef LIBCBOR_H_
#define LIBCBOR_H_

#include "cbor/data.h"
#include "cbor/common.h"

#include "cbor/arrays.h"
#include "cbor/bytestrings.h"
#include "cbor/floats_ctrls.h"
#include "cbor/ints.h"
#include "cbor/maps.h"
#include "cbor/strings.h"
#include "cbor/tags.h"

#include "cbor/encoding.h"
#include "cbor/serialization.h"
#include "cbor/callbacks.h"


/*
* ============================================================================
* High level decoding
* ============================================================================
*/

struct cbor_decoder_result cbor_stream_decode(cbor_data, size_t, const struct cbor_callbacks *, void *);
cbor_item_t * cbor_load(cbor_data source, size_t source_size, struct cbor_load_result * result);


#ifdef DEBUG
#include <stdio.h>
/* Pretty-print item description to STDOUT */
void cbor_describe(cbor_item_t * item);
#endif

#endif //LIBCBOR_H_
