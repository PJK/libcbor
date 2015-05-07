/*
 * Copyright (c) 2014-2015 Pavel Kalvoda <me@pavelkalvoda.com>
 *
 * libcbor is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef LIBCBOR_STREAMING_H
#define LIBCBOR_STREAMING_H

#include "common.h"
#include "callbacks.h"

#ifdef __cplusplus
extern "C" {
#endif


struct cbor_decoder_result cbor_stream_decode(cbor_data,
											  size_t,
											  const struct cbor_callbacks *,
											  void *);


#ifdef __cplusplus
}
#endif

#endif //LIBCBOR_STREAMING_H
