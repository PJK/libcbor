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
#include "cbor/streaming.h"


/*
* ============================================================================
* High level decoding
* ============================================================================
*/

cbor_item_t * cbor_load(cbor_data source,
						size_t source_size,
						struct cbor_load_result * result);

#ifdef PRETTY_PRINTER
#include <stdio.h>

void cbor_describe(cbor_item_t * item, FILE * out);

#endif

#endif //LIBCBOR_H_
