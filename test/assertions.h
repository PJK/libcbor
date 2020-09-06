#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>

#include <cmocka.h>

#include "cbor.h"

#ifndef ASSERTIONS_H_
#define ASSERTIONS_H_

void assert_uint8(cbor_item_t* item, uint8_t num);
void assert_uint16(cbor_item_t* item, uint16_t num);
void assert_uint32(cbor_item_t* item, uint32_t num);
void assert_uint64(cbor_item_t* item, uint64_t num);

void assert_decoder_result(size_t, enum cbor_decoder_status,
                           struct cbor_decoder_result);

// TODO: Docs
void assert_decoder_result_nedata(size_t, struct cbor_decoder_result);

/**
 * Check that the streaming decoder will returns a correct CBOR_DECODER_NEDATA
 * result for all inputs from data[0..1] through data[0..(expected-1)].
 */
void assert_minimum_input_size(size_t expected, cbor_data data);

#endif
