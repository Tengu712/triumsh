#include "command.h"

#include <stdio.h>
#include <string.h>

int builtin_echo(const uint8_t *const cmdline, size_t len) {
	if (len < 4) return 0;
	if (strncmp((char *)cmdline, "ECHO", 4) != 0) return 0;
	if (len == 4) {
		fwrite("\n", sizeof(char), 1, stdout);
		return 1;
	}
	if (cmdline[4] != ' ' && cmdline[4] != '\t') return 0;
	if (len == 5) {
		fwrite("\n", sizeof(char), 1, stdout);
		return 1;
	}

	size_t s = 5;
	size_t i = 5;
	uint8_t quote = '\0';

	while (i < len) {
		if ((cmdline[i] == '\'' || cmdline[i] == '"') && (quote == '\0' || quote == cmdline[i])) {
			if (i > s) fwrite((void *)&cmdline[s], sizeof(uint8_t), i - s, stdout);
			quote = cmdline[i];
			i++;
			s = i;
			continue;
		}
		i++;
	}

	if (i > s) fwrite((void *)&cmdline[s], sizeof(uint8_t), i - s, stdout);
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
