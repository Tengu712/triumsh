#pragma once

// Executes a command. Returns -1 on failure,
// otherwise returns the command's exit code.
int execute_command(char *const cmdline);
