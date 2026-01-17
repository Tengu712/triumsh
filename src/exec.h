#pragma once

#include <stddef.h>
#include <stdint.h>

// Executes a command.
// Returns -1 on failure, otherwise returns the command's exit code.
int execute_command(const uint8_t *const *cmdline, size_t count);

// Executes an external command.
// Returns -1 on failure, otherwise returns the command's exit code.
//
// NOTE: Internal use only. Called exclusively by `execute_command()`.
int execute_external_command(const uint8_t *const *cmdline, size_t count);
