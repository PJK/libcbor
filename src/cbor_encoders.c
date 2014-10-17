#include "cbor.h"
#include "cbor_internal.h"
#include "magic.h"

size_t _cbor_encode_uint8(uint8_t value, unsigned char * buffer, size_t buffer_size, uint8_t offset)
{
	if (value <= 23) {
		if (buffer_size >= 1) {
			buffer[0] = value + offset;
			return 1;
		}
	} else {
		if (buffer_size >= 2) {
			buffer[0] = 0x18 + offset;
			buffer[1] = value;
			return 2;
		}
	}
	return 0;
}

size_t _cbor_encode_uint16(uint16_t value, unsigned char * buffer, size_t buffer_size, uint8_t offset)
{
	if (buffer_size >= 3) {
		buffer[0] = 0x19 + offset;

		#if defined(__clang__) || defined(__GNUC__)
			*(uint16_t *)&buffer[1] = __builtin_bswap16(value);
		#else
			buffer[1] = value >> 8;
			buffer[2] = value;
		#endif

		return 3;
	} else
		return 0;
}

size_t _cbor_encode_uint32(uint32_t value, unsigned char * buffer, size_t buffer_size, uint8_t offset)
{
	if (buffer_size >= 5) {
		buffer[0] = 0x1A + offset;

	#if defined(__clang__) || defined(__GNUC__)
		*(uint32_t *)&buffer[1] = __builtin_bswap32(value);
	#else
		buffer[1] = value >> 24;
		buffer[2] = value >> 16;
		buffer[3] = value >> 8;
		buffer[4] = value;
	#endif

		return 5;
	} else
		return 0;
}

size_t _cbor_encode_uint64(uint64_t value, unsigned char * buffer, size_t buffer_size, uint8_t offset)
{
	if (buffer_size >= 9) {
		buffer[0] = 0x1B + offset;

	#if defined(__clang__) || defined(__GNUC__)
		*(uint64_t *)&buffer[1] = __builtin_bswap64(value);
	#else
		buffer[1] = value >> 56;
		buffer[2] = value >> 48;
		buffer[3] = value >> 40;
		buffer[4] = value >> 32;
		buffer[5] = value >> 24;
		buffer[6] = value >> 16;
		buffer[7] = value >> 8;
		buffer[8] = value;
	#endif

		return 9;
	} else
		return 0;
}

size_t _cbor_encode_uint(uint64_t value, unsigned char * buffer, size_t buffer_size, size_t offset)
{
	if (value <= UINT16_MAX)
		if (value <= UINT8_MAX)
			return _cbor_encode_uint8((uint8_t)value, buffer, buffer_size, offset);
		else
			return _cbor_encode_uint16((uint16_t)value, buffer, buffer_size, offset);
	else
		if (value <= UINT32_MAX)
			return _cbor_encode_uint32((uint32_t)value, buffer, buffer_size, offset);
		else
			return _cbor_encode_uint64((uint64_t)value, buffer, buffer_size, offset);
}

size_t cbor_encode_uint8(uint8_t value, unsigned char * buffer, size_t buffer_size)
{
	return _cbor_encode_uint8(value, buffer, buffer_size, 0x00);
}

size_t cbor_encode_uint16(uint16_t value, unsigned char * buffer, size_t buffer_size)
{
	return _cbor_encode_uint16(value, buffer, buffer_size, 0x00);
}

size_t cbor_encode_uint32(uint32_t value, unsigned char * buffer, size_t buffer_size)
{
	return _cbor_encode_uint32(value, buffer, buffer_size, 0x00);
}

size_t cbor_encode_uint64(uint64_t value, unsigned char * buffer, size_t buffer_size)
{
	return _cbor_encode_uint64(value, buffer, buffer_size, 0x00);
}

size_t cbor_encode_uint(uint64_t value, unsigned char * buffer, size_t buffer_size)
{
	return _cbor_encode_uint(value, buffer, buffer_size, 0x00);
}


size_t cbor_encode_negint8(uint8_t value, unsigned char * buffer, size_t buffer_size)
{
	return _cbor_encode_uint8(value, buffer, buffer_size, 0x20);
}

size_t cbor_encode_negint16(uint16_t value, unsigned char * buffer, size_t buffer_size)
{
	return _cbor_encode_uint16(value, buffer, buffer_size, 0x20);
}

size_t cbor_encode_negint32(uint32_t value, unsigned char * buffer, size_t buffer_size)
{
	return _cbor_encode_uint32(value, buffer, buffer_size, 0x20);
}

size_t cbor_encode_negint64(uint64_t value, unsigned char * buffer, size_t buffer_size)
{
	return _cbor_encode_uint64(value, buffer, buffer_size, 0x20);
}

size_t cbor_encode_negint(uint64_t value, unsigned char * buffer, size_t buffer_size)
{
	return _cbor_encode_uint(value, buffer, buffer_size, 0x20);
}
