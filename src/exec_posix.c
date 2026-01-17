#include "exec.h"

#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

int execute_external_command(const uint8_t *const *cmdline, size_t count) {
	// VALIDATION:
	if (cmdline[count] != NULL) {
		fprintf(stderr, "Internal error: cmdline array not NULL terminated\n");
		exit(2);
	}

	pid_t pid = fork();
	if (pid < 0) return -1;

	if (pid == 0) {
		// NOTE: This block runs on child process.
		execvp((char *)cmdline[0], (char **)cmdline);
		_exit(127);
	}

	int status;
	if (waitpid(pid, &status, 0) < 0) return -1;
	if (WIFEXITED(status))            return WEXITSTATUS(status);
	return -1;
}
