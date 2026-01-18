#pragma once

#include "cursor.h"
#include "file.h"

#include <string.h>

typedef enum TokenizingState_t {
	TS_NEUTORAL = 0,
	TS_IN_TOKEN,
	TS_DOUBLE_QUOTED,
} TokenizingState;

typedef struct CommandLineBuffer_t {
	uint8_t *const  buf;
	size_t          start;
	size_t          cursor;
	uint8_t **const cmdline;
	size_t          token_count;
	TokenizingState state;
} CommandLineBuffer;

Cursor eval_cmdline(FileInfo *fi, Cursor cur, CommandLineBuffer *clb, int *exit_code);
