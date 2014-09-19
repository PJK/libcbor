/*
 * This file provides testing tools for the streaming decoder. The expected usage is as follows:
 * 	1) SE API wrapper is initialized
 * 	2) Client build (ordered) series of expectations
 * 	3) The decoder is executed
 * 	4) SE checks all assertions
 * 	5) Go to 2) if desired
 */

#ifndef STREAM_EXPECTATIONS_H_
#define STREAM_EXPECTATIONS_H_

#include <stdint.h>
#include <stddef.h>
#include "cbor.h"
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>


static const size_t max_queue_size = 30;

enum test_expectation {
	UINT8_EQ,
	UINT16_EQ,
	UINT32_EQ,
	UINT64_EQ,
	NEGINT8_EQ,
	NEGINT16_EQ,
	NEGINT32_EQ,
	NEGINT64_EQ
};

union test_expectation_data {
	uint8_t int8;
	uint16_t int16;
	uint32_t int32;
	uint64_t int64;
};

struct test_assertion {
	enum test_expectation       expectation;
	union test_expectation_data data;
};

/* Ordered from 0 to queue_size - 1 */
static struct test_assertion assertions_queue[max_queue_size];
static size_t queue_size = 0;
static size_t current_expectation = 0;

/* Tested function */
typedef struct cbor_decoder_result decoder_t(cbor_data, size_t, const struct cbor_callbacks *);
decoder_t * decoder;

void set_decoder(decoder_t *);
struct cbor_decoder_result decode(cbor_data, size_t, const struct cbor_callbacks *);

/* Assertions builders */

void assert_uint8_eq(uint8_t, union test_expectation_data);
void assert_uint16_eq(uint16_t, union test_expectation_data);
void assert_uint32_eq(uint32_t, union test_expectation_data);
void assert_uint64_eq(uint64_t, union test_expectation_data);

void assert_negint8_eq(uint8_t, union test_expectation_data);
void assert_negint16_eq(uint16_t, union test_expectation_data);
void assert_negint32_eq(uint32_t, union test_expectation_data);
void assert_negint64_eq(uint64_t, union test_expectation_data);

/* Assertions verifying callbacks */
enum cbor_callback_result uint8_callback(uint8_t);

#endif
