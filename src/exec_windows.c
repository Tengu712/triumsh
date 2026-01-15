#ifdef _WIN32

#include "exec.h"

#include <Windows.h>

int execute_command(char *const cmdline) {
	STARTUPINFO si = {0};
	si.cb = sizeof(si);
	PROCESS_INFORMATION pi;

	if (!CreateProcessA(NULL, cmdline, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) return -1;

	WaitForSingleObject(pi.hProcess, INFINITE);

	DWORD exit_code;
	GetExitCodeProcess(pi.hProcess, &exit_code);

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	return exit_code;
}

#endif
