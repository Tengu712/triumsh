#include "exec_internal.h"

#include "strutil.h"

#include <io.h>
#include <Windows.h>

int execute_external_command(ExecParams params) {
	static uint8_t cmdline_str[32768];

	// Build command line.
	uint8_t *p = cmdline_str;
	for (size_t i = 0; i < params.count; ++i) {
		int has_space = 0;
		size_t len = strlen_cheking_space(params.cmdline[i], &has_space);
		if (has_space) {
			*p++ = '"';
			p += cpystr_escaping_char(p, params.cmdline[i], '"');
			*p++ = '"';
		} else {
			memcpy(p, params.cmdline[i], len);
			p += len;
		}
		if (i < params.count - 1) *p++ = ' ';
	}
	*p = '\0';

	// Define variables.
	BOOL inherit_needed = FALSE;
	STARTUPINFO      si = {0};
	si.cb = sizeof(STARTUPINFO);

	// Create pipes to capture the output.
	HANDLE read_pipe  = NULL;
	HANDLE write_pipe = NULL;
	if (params.output) {
		SECURITY_ATTRIBUTES sa = {sizeof(SECURITY_ATTRIBUTES), NULL, TRUE};
		if (!CreatePipe(&read_pipe, &write_pipe, &sa, 0)) return -1;
		SetHandleInformation(read_pipe, HANDLE_FLAG_INHERIT, 0);
		inherit_needed = TRUE;
		si.hStdOutput = write_pipe;
	}

	// Create a handle to redirect the output to a file.
	HANDLE redirect_file = INVALID_HANDLE_VALUE;
	if (params.destination && params.destination != stdout) {
		redirect_file = (HANDLE)_get_osfhandle(_fileno(params.destination));
		if (redirect_file == INVALID_HANDLE_VALUE) return -1;
		SetHandleInformation(redirect_file, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
		inherit_needed = TRUE;
		si.hStdOutput = redirect_file;
	}

	// Set up.
	if (si.hStdOutput != NULL) {
		si.dwFlags = STARTF_USESTDHANDLES;
		si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
		si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
	}

	// Execute.
	PROCESS_INFORMATION pi;
	if (!CreateProcessA(NULL, (char *)cmdline_str, NULL, NULL, inherit_needed, 0, NULL, NULL, &si, &pi)) {
		if (read_pipe)  CloseHandle(read_pipe);
		if (write_pipe) CloseHandle(write_pipe);
		return -1;
	}

	// Read the output.
	if (params.output) {
		CloseHandle(write_pipe);
		*params.output_len = 0;
		DWORD n;
		while (ReadFile(read_pipe, params.output + *params.output_len, 4096, &n, NULL) && n > 0) *params.output_len += n;
		CloseHandle(read_pipe);
	}

	// Wait & exit.
	WaitForSingleObject(pi.hProcess, INFINITE);
	DWORD exit_code;
	GetExitCodeProcess(pi.hProcess, &exit_code);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	return exit_code;
}
