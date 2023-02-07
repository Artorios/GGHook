#include <Windows.h>
#include "Process.h"
#include <Tlhelp32.h>

BOOL EnableDebugPrivileges()
{
	HANDLE hToken;
	LUID luid;
	TOKEN_PRIVILEGES tokenPriv;
	BOOL result = FALSE;

	OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken);
	if (LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luid))
	{
		tokenPriv.PrivilegeCount = 1;
		tokenPriv.Privileges[0].Luid = luid;
		tokenPriv.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

		result = AdjustTokenPrivileges(hToken, false, &tokenPriv, sizeof(tokenPriv), NULL, NULL);
	}

	CloseHandle(hToken);
	return result;
}

DWORD getPIDbyName(WCHAR * processName, DWORD parendPid)
{
	HANDLE pHandle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	PROCESSENTRY32 ProcessEntry;
	DWORD pid;
	ProcessEntry.dwSize = sizeof(ProcessEntry);
	bool Loop = Process32First(pHandle, &ProcessEntry);

	while (Loop)
	{
		if (wcsstr(ProcessEntry.szExeFile, processName))
		{
			pid = ProcessEntry.th32ProcessID;
			if (!parendPid || ProcessEntry.th32ParentProcessID == parendPid)
			{
				CloseHandle(pHandle);
				return pid;
			}
		}
		Loop = Process32Next(pHandle, &ProcessEntry);
	}
	return 0;
}
