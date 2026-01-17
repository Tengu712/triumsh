#pragma once

#include "strutil.h"

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

typedef struct Context_t {
	const char *const file_name;
	uint8_t *const    data;
	size_t const      file_size;
	size_t            cursor;
	size_t            line_number;
} Context;

// Returns 1 if still within the file, 0 otherwise.
static inline int is_continue(const Context *ctx) {
	return ctx->cursor < ctx->file_size;
}

// Returns 1 if the current character is whitespace, 0 otherwise.
static inline int is_whitespace(const Context *ctx) {
	switch (ctx->data[ctx->cursor]) {
	case ' ':
	case '\t':
		return 1;
	default:
		return 0;
	}
}

// Advances cursor by one UTF-8 character.
static inline void advance_cursor(Context *ctx) {
	if (ctx->data[ctx->cursor] == '\n') {
		ctx->cursor++;
		ctx->line_number++;
		return;
	}
	const size_t advance = get_utf8_char_length(ctx->data[ctx->cursor]);
	if (!advance) {
		fprintf(stderr, "Invalid character found: %s (%zu)\n", ctx->file_name, ctx->line_number);
		exit(1);
	}
	ctx->cursor += advance;
}

static inline void skip_whitespaces(Context *ctx) {
	while (is_continue(ctx)) {
		if (is_whitespace(ctx)) ctx->cursor++;
		else                    return;
	}
}

static inline int skip_to_next_newline(Context *ctx) {
	while (is_continue(ctx)) {
		if (ctx->data[ctx->cursor] == '\n') {
			advance_cursor(ctx);
			return 1;
		}
		advance_cursor(ctx);
	}
	return 0;
}

static inline int skip_to_next_single_quote(Context *ctx) {
	while (is_continue(ctx)) {
		// TODO: check if escaped.
		if (ctx->data[ctx->cursor] == '\'') {
			ctx->cursor++;
			return 1;
		}
		advance_cursor(ctx);
	}
	return 0;
}

static inline void skip_until_special_char(Context *ctx) {
	while (is_continue(ctx)) {
		// TODO: add \ $.
		switch (ctx->data[ctx->cursor]) {
		case ' ':
		case '\t':
		case '\n':
		case '\'':
		case '"':
			return;
		}
		advance_cursor(ctx);
	}
}
