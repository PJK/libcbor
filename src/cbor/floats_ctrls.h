/*
 * Copyright (c) 2014-2015 Pavel Kalvoda <me@pavelkalvoda.com>
 *
 * libcbor is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef LIBCBOR_FLOATS_CTRLS_H
#define LIBCBOR_FLOATS_CTRLS_H

#include "common.h"

/*
* ============================================================================
* Float manipulation
* ============================================================================
*/

bool cbor_float_ctrl_is_ctrl(const cbor_item_t * item);
cbor_float_width cbor_float_get_width(const cbor_item_t * item);

float cbor_float_get_float2(const cbor_item_t * item);
float cbor_float_get_float4(const cbor_item_t * item);
double cbor_float_get_float8(const cbor_item_t * item);

double cbor_float_get_float(const cbor_item_t * item);

cbor_item_t * cbor_new_ctrl(); /* float0 */
cbor_item_t * cbor_new_float2();
cbor_item_t * cbor_new_float4();
cbor_item_t * cbor_new_float8();

cbor_item_t * cbor_new_null();
cbor_item_t * cbor_new_undef();
cbor_item_t * cbor_new_bool(bool value);

void cbor_set_ctrl(cbor_item_t * item, uint8_t value);
void cbor_set_float2(cbor_item_t * item, float value);
void cbor_set_float4(cbor_item_t * item, float value);
void cbor_set_float8(cbor_item_t * item, double value);

uint8_t cbor_ctrl_value(const cbor_item_t * item);
bool cbor_ctrl_bool(const cbor_item_t * item);

#endif //LIBCBOR_FLOATS_CTRLS_H
