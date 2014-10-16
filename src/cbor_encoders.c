#include "cbor.h"
#include "cbor_internal.h"
#include "magic.h"

size_t cbor_encode_uint8(uint8_t value, unsigned char * buffer, size_t buffer_size)
{
	if (value <= 23) {
		if (buffer_size >= 1) {
			buffer[0] = value;
			return 1;
		}
	} else {
		if (buffer_size >= 2) {
			buffer[0] = 0x18;
			buffer[1] = value;
			return 2;
		}
	}
	return 0;
}

size_t cbor_encode_uint16(uint16_t value, unsigned char * buffer, size_t buffer_size)
{
	if (buffer_size >= 3) {
		buffer[0] = 0x19;

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

size_t cbor_encode_uint32(uint32_t value, unsigned char * buffer, size_t buffer_size)
{
	if (buffer_size >= 5) {
		buffer[0] = 0x1A;

	#if defined(__clang__) || defined(__GNUC__)
		*(uint32_t *)&buffer[1] = __builtin_bswap32(value);
	#else
		buffer[1] = value >> 24;
		buffer[2] = value >> 16;
		buffer[3] = value >> 8;
		buffer[4] = value;
	#endif

		return 3;
	} else
		return 0;
}

size_t cbor_encode_uint64(uint64_t value, unsigned char * buffer, size_t buffer_size)
{

}

size_t cbor_encode_uint(uint64_t value, unsigned char * buffer, size_t buffer_size)
{

}
