#ifndef CBOR_UNICODE_H_
#define CBOR_UNICODE_H_

#include "cbor.h"

enum _cbor_unicode_status_error {
	_CBOR_UNICODE_OK,
	_CBOR_UNICODE_BADCP
};

struct _cbor_unicode_status {
	enum _cbor_unicode_status_error status;
	size_t                          location;
};

size_t _cbor_unicode_codepoint_count(cbor_data source, size_t source_length, struct _cbor_unicode_status * status);

#endif
