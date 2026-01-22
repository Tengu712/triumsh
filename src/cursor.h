#pragma once

#include "strutil.h"

typedef struct Cursor_t {
	const uint8_t *ptr;
	size_t         line;
} Cursor;

// Advances cursor by one UTF-8 character.
// If the character is an invalid, sets 1 to `has_error`.
//
// NOTE: `cur` must not be ended.
static inline Cursor advance_cursor(Cursor cur, int *has_error) {
	if (*cur.ptr == '\n') return (Cursor){cur.ptr + 1, cur.line + 1};

	const size_t advance = get_utf8_char_length(*cur.ptr);
	if (!advance) {
		if (has_error) *has_error = 1;
		return (Cursor){cur.ptr + 1, cur.line};
	}
	for (size_t i = 0; i < advance; ++i) {
		if (!cur.ptr[i]) {
			if (has_error) *has_error = 1;
			return (Cursor){cur.ptr + 1, cur.line};
		}
	}
	return (Cursor){cur.ptr + advance, cur.line};
}

// Skips whitespaces.
//
// EXAMPLE: "  \t foo" -> "foo"
// EXAMPLE: "  \n foo" -> " foo"
//
// NOTE: `cur` must not be ended.
static inline Cursor skip_whitespaces(Cursor cur) {
	while (*cur.ptr && is_whitespace(*cur.ptr)) cur.ptr++;
	return (Cursor){cur.ptr, cur.line};
}

// Skips the line.
// If the line contains an invalid character, sets 1 to `has_error`.
//
// EXAMPLE: "foo\nbar" -> "bar"
// EXAMPLE: "foo\\\nbar" -> "bar"
//
// NOTE: `cur` must not be ended.
// NOTE: `has_error` must be initialized to 0.
static inline Cursor skip_line(Cursor cur, int *has_error) {
	while (*cur.ptr && !*has_error) {
		if (*cur.ptr == '\n') return advance_cursor(cur, has_error);
		cur = advance_cursor(cur, has_error);
	}
	return cur;
}

// Skips to after single quote.
// Escaped single quotes (\') are ignored.
//
// `has_error` will be sets to:
// - 1: if an invalid character found,
// - 2: no single quote found.
//
// EXAMPLE: "foo 'bar" -> "bar"
// EXAMPLE: "foo\\'bar\nbaz'fuga" -> "fuga"
//
// NOTE: `cur` must not be ended.
// NOTE: `has_error` must be initialized to 0.
static inline Cursor skip_to_after_single_quote(Cursor cur, int *has_error) {
	while (*cur.ptr && !*has_error) {
		switch (*cur.ptr) {
		case '\'':
			return advance_cursor(cur, has_error);
		case '\\':
			cur.ptr++;
			if (*cur.ptr) cur = advance_cursor(cur, has_error);
			break;
		default:
			cur = advance_cursor(cur, has_error);
			break;
		}
	}
	*has_error = 2;
	return cur;
}

// Skips until a special character.
// If an invalid character found, sets 1 to `has_error`.
//
// EXAMPLE: "foo bar" -> " bar"
//
// NOTE: `cur` must not be ended.
// NOTE: `has_error` must be initialized to 0.
static inline Cursor skip_until_special_char(Cursor cur, int *has_error) {
	while (*cur.ptr && !*has_error) {
		switch (*cur.ptr) {
		case ' ':
		case '\t':
		case '\n':
		case '\'':
		case '"':
		case '\\':
		case '$':
		case '{':
		case '}':
		case '(':
		case ')':
		case '>':
		case '|':
			return cur;
		default:
			cur = advance_cursor(cur, has_error);
			break;
		}
	}
	return cur;
}
