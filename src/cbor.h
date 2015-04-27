#ifndef CBOR_H_
#define CBOR_H_


#include <stdio.h>

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

#endif
