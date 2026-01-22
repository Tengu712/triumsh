#include "exec.h"
#include "exec_internal.h"

#include "strutil.h"

#include <stdio.h>
#include <string.h>

void write_output(const void *src, size_t len, uint8_t **output, FILE *redirect_file) {
	if (*output) {
		memcpy(*output, src, len);
		*output += len;
	}
	else if (redirect_file) fwrite(src, sizeof(uint8_t), len, redirect_file);
	else fwrite(src, sizeof(uint8_t), len, stdout);
}

// Executes ECHO command.
// Returns 0 on success, 1 on failure, 2 on skipped.
int execute_ECHO(const uint8_t *const *cmdline, size_t count, uint8_t *output, size_t *output_len, FILE *redirect_file) {
	if (!cmpstr_early(cmdline[0], (uint8_t *)"ECHO")) return 2;
	uint8_t *const start = output;
	for (size_t i = 1; i < count; ++i) {
		size_t len = strlen((char *)cmdline[i]);
		write_output(cmdline[i], len, &output, redirect_file);
		if (i + 1 < count) write_output(" ", 1, &output, redirect_file);
	}
	write_output("\n", 1, &output, redirect_file);
	if (output_len) *output_len = output - start;
	return 0;
}

#define EXECUTE(n) \
	switch (execute_##n(cmdline, count, output, output_len, redirect_file)) { \
	case 0: result = 0; goto cleanup; \
	case 1: result = -1; goto cleanup; \
	default: break; \
	}

int execute_command(const uint8_t *const *cmdline, size_t count, uint8_t *output, size_t *output_len, const uint8_t *redirect_path) {
	int result;
	FILE *redirect_file = NULL;
	if (redirect_path) {
		redirect_file = fopen((const char *)redirect_path, "wb");
		if (!redirect_file) return -1;
	}
	EXECUTE(ECHO);
	result = execute_external_command(cmdline, count, output, output_len, redirect_path);
cleanup:
	if (redirect_file) fclose(redirect_file);
	return result;
}
