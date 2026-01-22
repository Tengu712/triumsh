#include "exec_internal.h"

#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

int execute_external_command(const uint8_t *const *cmdline, size_t count, uint8_t *output, size_t *output_len, const uint8_t *redirect_path) {
	// VALIDATION:
	if (cmdline[count] != NULL) {
		fprintf(stderr, "Internal error: cmdline array not NULL terminated\n");
		exit(2);
	}

	int pipefd[2];
	if (output && pipe(pipefd) < 0) return -1;

	pid_t pid = fork();
	if (pid < 0) return -1;

	// NOTE: This block runs on child process.
	if (pid == 0) {
		if (output) {
			close(pipefd[0]);
			dup2(pipefd[1], STDOUT_FILENO);
			close(pipefd[1]);
		} else if (redirect_path) {
			int fd = open((const char *)redirect_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
			if (fd < 0) _exit(127);
			dup2(fd, STDOUT_FILENO);
			close(fd);
		}
		execvp((char *)cmdline[0], (char **)cmdline);
		_exit(127);
	}

	if (output) {
		close(pipefd[1]);
		*output_len = 0;
		ssize_t n;
		while ((n = read(pipefd[0], output + *output_len, 4096)) > 0) *output_len += n;
		close(pipefd[0]);
	}

	int status;
	if (waitpid(pid, &status, 0) < 0) return -1;
	if (WIFEXITED(status))            return WEXITSTATUS(status);
	return -1;
}
