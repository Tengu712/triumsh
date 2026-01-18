#pragma once

#include "cursor.h"

typedef struct CommandLineBuffer_t {
	uint8_t        *start;
	uint8_t        *ptr;
	uint8_t **const cmdline;
	size_t          token_count;
} CommandLineBuffer;

static inline void write_cmdline_buf(CommandLineBuffer *clb, const uint8_t *src, size_t size) {
	memcpy(clb->ptr, src, size);
	clb->ptr += size;
}

Cursor pr_top_level(const char *file_name, Cursor cur, CommandLineBuffer *clb);
Cursor pr_cmdline(const char *file_name, Cursor cur, CommandLineBuffer *clb);
Cursor pr_token(const char *file_name, Cursor cur, CommandLineBuffer *clb);
Cursor pr_single_quoted(const char *file_name, Cursor cur, CommandLineBuffer *clb);
