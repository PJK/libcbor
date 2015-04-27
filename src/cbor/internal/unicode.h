/*
 * Copyright (c) 2014-2015 Pavel Kalvoda <me@pavelkalvoda.com>
 *
 * libcbor is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef LIBCBOR_UNICODE_H
#define LIBCBOR_UNICODE_H

#include "../common.h"

enum _cbor_unicode_status_error {
	_CBOR_UNICODE_OK,
	_CBOR_UNICODE_BADCP
};

struct _cbor_unicode_status {
	enum _cbor_unicode_status_error status;
	size_t                          location;
};

size_t _cbor_unicode_codepoint_count(cbor_data source, size_t source_length, struct _cbor_unicode_status * status);


#endif //LIBCBOR_UNICODE_H
