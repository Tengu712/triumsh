#include "command.h"

#include <stdio.h>
#include <string.h>

int builtin_echo(const uint8_t *const cmdline, size_t len) {
	if (len < 4) return 0;
	if (strncmp((char *)cmdline, "ECHO", 4) != 0) return 0;
	if (len == 4) return 1;
	if (cmdline[4] != ' ' && cmdline[4] != '\t') return 0;
	if (len == 5) return 1;
	fwrite((void *)&cmdline[5], sizeof(uint8_t), len - 5, stdout);
	fwrite("\n", sizeof(char), 1, stdout);
	return 1;
}

int exec_builtin_command(const uint8_t *const cmdline, size_t len) {
	if (len == 0) return 0;

	switch (cmdline[0]) {
	case 'E': return builtin_echo(cmdline, len);
	}

	return 0;
}
