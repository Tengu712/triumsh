#include "exec_internal.h"

#include "strutil.h"

#include <stdio.h>
#include <string.h>

void write_output(const void *src, size_t len, uint8_t **output, FILE *destination) {
	if (*output) {
		memcpy(*output, src, len);
		*output += len;
	}
	else fwrite(src, sizeof(uint8_t), len, destination);
}

// Executes ECHO command.
// Returns 0 on success, 1 on failure, 2 on skipped.
int execute_ECHO(ExecParams params) {
	if (!cmpstr_early(params.cmdline[0], (uint8_t *)"ECHO")) return 2;
	uint8_t *output = params.output;
	for (size_t i = 1; i < params.count; ++i) {
		size_t len = strlen((char *)params.cmdline[i]);
		write_output(params.cmdline[i], len, &output, params.destination);
		if (i + 1 < params.count) write_output(" ", 1, &output, params.destination);
	}
	write_output("\n", 1, &output, params.destination);
	if (params.output_len) *params.output_len = output - params.output;
	return 0;
}

#define EXECUTE(n) \
	switch (execute_##n(params)) { \
	case 0: return 0; \
	case 1: return -1; \
	default: break; \
	}

int execute_command(ExecParams params) {
	EXECUTE(ECHO);
	return execute_external_command(params);
}
