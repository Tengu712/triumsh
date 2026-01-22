#include "exec_internal.h"

#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

int execute_external_command(ExecParams params) {
	int pipefd[2];
	if (params.output && pipe(pipefd) < 0) return -1;

	pid_t pid = fork();
	if (pid < 0) return -1;

	// NOTE: This block runs on child process.
	if (pid == 0) {
		if (params.output) {
			close(pipefd[0]);
			dup2(pipefd[1], STDOUT_FILENO);
			close(pipefd[1]);
		} else if (params.destination && params.destination != stdout) {
			dup2(fileno(params.destination), STDOUT_FILENO);
		}
		execvp((char *)params.cmdline[0], (char **)params.cmdline);
		_exit(127);
	}

	if (params.output) {
		close(pipefd[1]);
		*params.output_len = 0;
		ssize_t n;
		while ((n = read(pipefd[0], params.output + *params.output_len, 4096)) > 0) *params.output_len += n;
		close(pipefd[0]);
	}

	int status;
	if (waitpid(pid, &status, 0) < 0) return -1;
	if (WIFEXITED(status))            return WEXITSTATUS(status);
	return -1;
}
