#include "cbor.h"
#include <setjmp.h>
#include <stdarg.h>
#include <cmocka.h>

#ifndef ASSERTIONS_H_
#define ASSERTIONS_H_

void assert_uint8(cbor_item_t * item, uint8_t num);

void assert_decoder_result(size_t, enum cbor_decoder_status, struct cbor_decoder_result);

#endif
