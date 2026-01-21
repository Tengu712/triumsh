#pragma once

#include "cursor.h"

typedef struct CommandLineBuffer_t {
	uint8_t        *start;
	uint8_t        *ptr;
	uint8_t **const cmdline;
	size_t          token_count;
} CommandLineBuffer;

Cursor pr_escape(const char *file_name, Cursor cur, CommandLineBuffer *clb);
Cursor pr_expansion_variable(const char *file_name, Cursor cur, CommandLineBuffer *clb, int simple);
Cursor pr_expansion(const char *file_name, Cursor cur, CommandLineBuffer *clb);
Cursor pr_single_quoted(const char *file_name, Cursor cur, CommandLineBuffer *clb);
Cursor pr_double_quoted(const char *file_name, Cursor cur, CommandLineBuffer *clb);
Cursor pr_token(const char *file_name, Cursor cur, CommandLineBuffer *clb);
Cursor pr_cmdline(const char *file_name, Cursor cur, CommandLineBuffer *clb, uint8_t end_char, int piped);
Cursor pr_top_level(const char *file_name, Cursor cur, CommandLineBuffer *clb);
