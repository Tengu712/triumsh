#pragma once

#include <stddef.h>
#include <stdint.h>

int execute_external_command(const uint8_t *const *cmdline, size_t count, uint8_t *output, size_t *output_len);
