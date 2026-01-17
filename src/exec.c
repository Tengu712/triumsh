#include "exec.h"

#include "strutil.h"

#include <stdio.h>
#include <string.h>

// Executes ECHO command.
// Returns 0 on success, 1 on failure, 2 on skipped.
int execute_ECHO(const uint8_t *const *cmdline, size_t count) {
	if (!cmpstr_early(cmdline[0], (uint8_t *)"ECHO")) return 2;
	for (size_t i = 1; i < count; ++i) {
		fwrite(cmdline[i], 1, strlen((char *)cmdline[i]), stdout);
		if (i + 1 < count) fwrite(" ", 1, 1, stdout);
	}
	fwrite("\n", 1, 1, stdout);
	return 0;
}

#define EXECUTE(n) \
	switch (execute_##n(cmdline, count)) { \
	case 0: return 0; \
	case 1: return -1; \
	default: break; \
	}

int execute_command(const uint8_t *const *cmdline, size_t count) {
	EXECUTE(ECHO);
	return execute_external_command(cmdline, count);
}
