#pragma once

#include <stdint.h>

// Executes a built-in command.
// Returns 1 if executed, 0 otherwise.
int exec_builtin_command(const uint8_t *const cmdline, size_t len);
