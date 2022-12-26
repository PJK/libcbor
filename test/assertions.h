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

/** Verify the `actual_result.status` and `actual_result.status`. */
void assert_decoder_result(size_t expected_bytes_read,
                           enum cbor_decoder_status expected_status,
                           struct cbor_decoder_result actual_result);

/**
 * Assert that the result is set to CBOR_DECODER_NEDATA with the given
 * `cbor_decoder_result.required` value.
 */
void assert_decoder_result_nedata(size_t expected_bytes_required,
                                  struct cbor_decoder_result actual_result);

/**
 * Check that the streaming decoder returns a correct CBOR_DECODER_NEDATA
 * result for all inputs from data[0..1] through data[0..(expected-1)].
 */
void assert_minimum_input_size(size_t expected, cbor_data data);

#endif
