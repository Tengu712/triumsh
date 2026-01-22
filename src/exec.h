#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

typedef struct ExecParams_t {
	const uint8_t *const *const cmdline;
	const size_t                count;
	FILE *const                 destination;

	// If `output` is not NULL, stdout is captured into `output`
	// and `*output_len` is set to the length.
	// Then, `destination` is not used.
	uint8_t *const output;
	size_t  *const output_len;

	// If `input` is not NULL, it is used as stdin.
	const uint8_t *const input;
	const size_t         input_len;
} ExecParams;

// Executes a command.
// Returns -1 on failure, otherwise returns the command's exit code.
int execute_command(ExecParams params);
