#pragma once

#include <stdint.h>

// Returns UTF-8 character length in bytes from the first byte, or 0 if invalid.
inline unsigned int get_utf8_char_length(uint8_t first_byte) {
	if ((first_byte & 0x80) == 0x00) return 1; // 0xxxxxxx
	if ((first_byte & 0xE0) == 0xC0) return 2; // 110xxxxx
	if ((first_byte & 0xF0) == 0xE0) return 3; // 1110xxxx
	if ((first_byte & 0xF8) == 0xF0) return 4; // 11110xxx
	return 0;
}
