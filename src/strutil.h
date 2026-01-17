#pragma once

#include <stddef.h>
#include <stdint.h>
#include <string.h>

// Returns UTF-8 character length in bytes from the first byte, or 0 if invalid.
static inline size_t get_utf8_char_length(uint8_t first_byte) {
	if ((first_byte & 0x80) == 0x00) return 1; // 0xxxxxxx
	if ((first_byte & 0xE0) == 0xC0) return 2; // 110xxxxx
	if ((first_byte & 0xF0) == 0xE0) return 3; // 1110xxxx
	if ((first_byte & 0xF8) == 0xF0) return 4; // 11110xxx
	return 0;
}

// Returns 1 if `s1` is the same as `s2`, 0 otherwise.
//
// NOTE: Both `s1` and `s2` must be null-terminated.
static inline int cmpstr_early(const uint8_t *s1, const uint8_t *s2) {
	while (*s1 && *s2) {
		if (*s1 != *s2) return 0;
		s1++;
		s2++;
	}
	return (*s1 == *s2);
}

// Returns the length of `s`.
// If `s` contains one or more spaces, sets `has_space` to 1.
//
// NOTE: `s` must be null-terminated.
static inline size_t strlen_cheking_space(const uint8_t *s, int *has_space) {
	const uint8_t *const start = s;
	while (*s) {
		if (*s == ' ' || *s == '\t') *has_space = 1;
		s += get_utf8_char_length(*s);
	}
	return s - start;
}

// Copies string from `src` to `dst`, escaping character `c` with backslash.
// Returns the number of bytes written to `dst`.
//
// NOTE: `src` must be null-terminated.
// NOTE: `dst` must have enough space for the escaped string.
static inline size_t cpystr_escaping_char(uint8_t *dst, const uint8_t *src, uint8_t c) {
	const uint8_t *const dst_start = dst;
	const uint8_t       *src_start = src;
	while (*src) {
		if (*src == c) {
			const size_t advance = src - src_start;
			memcpy(dst, src_start, advance);
			dst += advance;
			*dst++ = '\\';
			*dst++ = c;
			src++;
			src_start = src;
		} else {
			src += get_utf8_char_length(*src);
		}
	}
	const size_t remaining = src - src_start;
	memcpy(dst, src_start, remaining);
	dst += remaining;
	return dst - dst_start;
}
