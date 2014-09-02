#include "cbor.h"
#include <setjmp.h>
#include <stdarg.h>
#include <cmocka.h>

#ifndef ASSERTIONS_H_
#define ASSERTIONS_H_

void assert_uint8(cbor_item_t * item, uint8_t num);

#endif
