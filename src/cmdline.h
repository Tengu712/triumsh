#pragma once

#include "context.h"

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

int eval_cmdline(Context *ctx, CommandLineBuffer *clb);
