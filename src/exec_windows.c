#include "exec_internal.h"

#include "strutil.h"

#include <Windows.h>

int execute_external_command(const uint8_t *const *cmdline, size_t count, uint8_t *output, size_t *output_len) {
	static uint8_t cmdline_str[32768];

	uint8_t *p = cmdline_str;
	for (size_t i = 0; i < count; ++i) {
		int has_space = 0;
		size_t len = strlen_cheking_space(cmdline[i], &has_space);
		if (has_space) {
			*p++ = '"';
			p += cpystr_escaping_char(p, cmdline[i], '"');
			*p++ = '"';
		} else {
			memcpy(p, cmdline[i], len);
			p += len;
		}
		if (i < count - 1) *p++ = ' ';
	}
	*p = '\0';

	HANDLE hReadPipe = NULL, hWritePipe = NULL;
	if (output) {
		SECURITY_ATTRIBUTES sa = {sizeof(SECURITY_ATTRIBUTES), NULL, TRUE};
		if (!CreatePipe(&hReadPipe, &hWritePipe, &sa, 0)) return -1;
		SetHandleInformation(hReadPipe, HANDLE_FLAG_INHERIT, 0);
	}

	STARTUPINFO si = {0};
	si.cb = sizeof(si);
	if (output) {
		si.dwFlags = STARTF_USESTDHANDLES;
		si.hStdOutput = hWritePipe;
		si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
		si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
	}
	PROCESS_INFORMATION pi;

	if (!CreateProcessA(NULL, (char *)cmdline_str, NULL, NULL, output ? TRUE : FALSE, 0, NULL, NULL, &si, &pi)) {
		if (output) {
			CloseHandle(hReadPipe);
			CloseHandle(hWritePipe);
		}
		return -1;
	}

	if (output) {
		CloseHandle(hWritePipe);
		*output_len = 0;
		DWORD n;
		while (ReadFile(hReadPipe, output + *output_len, 4096, &n, NULL) && n > 0) *output_len += n;
		CloseHandle(hReadPipe);
	}

	WaitForSingleObject(pi.hProcess, INFINITE);

	DWORD exit_code;
	GetExitCodeProcess(pi.hProcess, &exit_code);

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	return exit_code;
}
