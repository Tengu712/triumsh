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

	// Create a handle to redirect the output to a file.
	HANDLE redirect_file = INVALID_HANDLE_VALUE;
	if (params.destination && params.destination != stdout) {
		redirect_file = (HANDLE)_get_osfhandle(_fileno(params.destination));
		if (redirect_file == INVALID_HANDLE_VALUE) return -1;
		SetHandleInformation(redirect_file, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT);
		inherit_needed = TRUE;
		si.hStdOutput = redirect_file;
	}

	// Create pipes to capture the output.
	HANDLE out_read_pipe  = NULL;
	HANDLE out_write_pipe = NULL;
	if (params.output) {
		SECURITY_ATTRIBUTES sa = {sizeof(SECURITY_ATTRIBUTES), NULL, TRUE};
		if (!CreatePipe(&out_read_pipe, &out_write_pipe, &sa, 0)) return -1;
		SetHandleInformation(out_read_pipe, HANDLE_FLAG_INHERIT, 0);
		inherit_needed = TRUE;
		si.hStdOutput = out_write_pipe;
	}

	// Create pipes to provide input.
	HANDLE in_read_pipe  = NULL;
	HANDLE in_write_pipe = NULL;
	if (params.input) {
		SECURITY_ATTRIBUTES sa = {sizeof(SECURITY_ATTRIBUTES), NULL, TRUE};
		if (!CreatePipe(&in_read_pipe, &in_write_pipe, &sa, 0)) {
			if (out_read_pipe)  CloseHandle(out_read_pipe);
			if (out_write_pipe) CloseHandle(out_write_pipe);
			return -1;
		}
		SetHandleInformation(in_write_pipe, HANDLE_FLAG_INHERIT, 0);
		inherit_needed = TRUE;
		si.hStdInput = in_read_pipe;
	}

	// Set up.
	if (si.hStdOutput != NULL || si.hStdInput != NULL) {
		si.dwFlags = STARTF_USESTDHANDLES;
		if (si.hStdOutput == NULL) si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
		if (si.hStdInput == NULL)  si.hStdInput  = GetStdHandle(STD_INPUT_HANDLE);
		si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
	}

	// Execute.
	PROCESS_INFORMATION pi;
	if (!CreateProcessA(NULL, (char *)cmdline_str, NULL, NULL, inherit_needed, 0, NULL, NULL, &si, &pi)) {
		if (out_read_pipe)  CloseHandle(out_read_pipe);
		if (out_write_pipe) CloseHandle(out_write_pipe);
		if (in_read_pipe)   CloseHandle(in_read_pipe);
		if (in_write_pipe)  CloseHandle(in_write_pipe);
		return -1;
	}

	// Write the input.
	if (params.input) {
		CloseHandle(in_read_pipe);
		DWORD written;
		WriteFile(in_write_pipe, params.input, (DWORD)params.input_len, &written, NULL);
		CloseHandle(in_write_pipe);
	}

	// Read the output.
	if (params.output) {
		CloseHandle(out_write_pipe);
		*params.output_len = 0;
		DWORD n;
		while (ReadFile(out_read_pipe, params.output + *params.output_len, 4096, &n, NULL) && n > 0) *params.output_len += n;
		CloseHandle(out_read_pipe);
	}

	// Wait & exit.
	WaitForSingleObject(pi.hProcess, INFINITE);
	DWORD exit_code;
	GetExitCodeProcess(pi.hProcess, &exit_code);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	return exit_code;
}
