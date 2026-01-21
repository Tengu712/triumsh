#include "exec.h"
#include "exec_internal.h"

#include "strutil.h"

#include <stdio.h>
#include <string.h>

void write_to_stdout_or_output(const void *src, size_t len, uint8_t **output) {
	if (*output) {
		memcpy(*output, src, len);
		*output += len;
	}
	else fwrite(src, sizeof(uint8_t), len, stdout);
}

// Executes ECHO command.
// Returns 0 on success, 1 on failure, 2 on skipped.
int execute_ECHO(const uint8_t *const *cmdline, size_t count, uint8_t *output, size_t *output_len) {
	if (!cmpstr_early(cmdline[0], (uint8_t *)"ECHO")) return 2;
	uint8_t *const start = output;
	for (size_t i = 1; i < count; ++i) {
		size_t len = strlen((char *)cmdline[i]);
		write_to_stdout_or_output(cmdline[i], len, &output);
		if (i + 1 < count) write_to_stdout_or_output(" ", 1, &output);
	}
	write_to_stdout_or_output("\n", 1, &output);
	if (output_len) *output_len = output - start;
	return 0;
}

#define EXECUTE(n) \
	switch (execute_##n(cmdline, count, output, output_len)) { \
	case 0: return 0; \
	case 1: return -1; \
	default: break; \
	}

int execute_command(const uint8_t *const *cmdline, size_t count, uint8_t *output, size_t *output_len) {
	EXECUTE(ECHO);
	return execute_external_command(cmdline, count, output, output_len);
}
