#pragma once
#include <Windows.h>

BOOL EnableDebugPrivileges();

DWORD getPIDbyName(WCHAR * processName, DWORD parendPid = 0);