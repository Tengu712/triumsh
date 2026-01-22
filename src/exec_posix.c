#include "exec_internal.h"

#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

int execute_external_command(ExecParams params) {
	int out_pipefd[2] = {-1, -1};
	int in_pipefd[2]  = {-1, -1};
	if (params.output && pipe(out_pipefd) < 0) return -1;
	if (params.input && pipe(in_pipefd) < 0) {
		if (out_pipefd[0] >= 0) { close(out_pipefd[0]); close(out_pipefd[1]); }
		return -1;
	}

	pid_t pid = fork();
	if (pid < 0) return -1;

	// NOTE: This block runs on child process.
	if (pid == 0) {
		if (params.output) {
			close(out_pipefd[0]);
			dup2(out_pipefd[1], STDOUT_FILENO);
			close(out_pipefd[1]);
		} else if (params.destination && params.destination != stdout) {
			dup2(fileno(params.destination), STDOUT_FILENO);
		}
		if (params.input) {
			close(in_pipefd[1]);
			dup2(in_pipefd[0], STDIN_FILENO);
			close(in_pipefd[0]);
		}
		execvp((char *)params.cmdline[0], (char **)params.cmdline);
		_exit(127);
	}

	// Write input to the child.
	if (params.input) {
		close(in_pipefd[0]);
		write(in_pipefd[1], params.input, params.input_len);
		close(in_pipefd[1]);
	}

	// Read output from the child.
	if (params.output) {
		close(out_pipefd[1]);
		*params.output_len = 0;
		ssize_t n;
		while ((n = read(out_pipefd[0], params.output + *params.output_len, 4096)) > 0) *params.output_len += n;
		close(out_pipefd[0]);
	}

	int status;
	if (waitpid(pid, &status, 0) < 0) return -1;
	if (WIFEXITED(status))            return WEXITSTATUS(status);
	return -1;
}
