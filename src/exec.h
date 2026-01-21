#pragma once

#include <stddef.h>
#include <stdint.h>

// Executes a command.
// Returns -1 on failure, otherwise returns the command's exit code.
// If `output` is not NULL, stdout is captured into `output` and `*output_len` is set to the length.
int execute_command(const uint8_t *const *cmdline, size_t count, uint8_t *output, size_t *output_len);
