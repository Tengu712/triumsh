#include "exec.h"

#include "strutil.h"

#include <Windows.h>

int execute_external_command(const uint8_t *const *cmdline, size_t count) {
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

	STARTUPINFO si = {0};
	si.cb = sizeof(si);
	PROCESS_INFORMATION pi;

	if (!CreateProcessA(NULL, (char *)cmdline_str, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) return -1;

	WaitForSingleObject(pi.hProcess, INFINITE);

	DWORD exit_code;
	GetExitCodeProcess(pi.hProcess, &exit_code);

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	return exit_code;
}
