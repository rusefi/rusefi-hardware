
#include <string.h>
#include <stdint.h>
#include "efilib.h"

static char *ltoa_internal(char *p, uint32_t num, unsigned radix) {
	constexpr int bufferLength = 10;

	char buffer[bufferLength];

	size_t idx = bufferLength - 1;

	// First, we write from right-to-left so that we don't have to compute
	// log(num)/log(radix)
	do
	{
		auto digit = num % radix;

		// Digits 0-9 -> '0'-'9'
		// Digits 10-15 -> 'a'-'f'
		char c = digit < 10
			? digit + '0'
			: digit + 'a' - 10;

		// Write this digit in to the buffer
		buffer[idx] = c;
		idx--;
	} while ((num /= radix) != 0);

	idx++;

	// Now, we copy characters in to place in the final buffer
	while (idx < bufferLength)
	{
		*p++ = buffer[idx++];
	}

	return p;
}

/**
 * @return pointer at the end zero symbol after the digits
 */
static char* itoa_signed(char *p, int num, unsigned radix) {
	if (num < 0) {
		*p++ = '-';
		char *end = ltoa_internal(p, -num, radix);
		*end = 0;
		return end;
	}
	char *end = ltoa_internal(p, num, radix);
	*end = 0;
	return end;
}

/**
 * Integer to string
 *
 * @return pointer at the end zero symbol after the digits
 */
char* itoa10(char *p, int num) {
	return itoa_signed(p, num, 10);
}
