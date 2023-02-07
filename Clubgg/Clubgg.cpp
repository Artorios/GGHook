#include "stdafx.h"
#include <Windows.h>
#include <Tlhelp32.h>
#include <stdio.h>

#define ProcessName L"ClubGG.exe"
#define DllPath L"D:\\MyPrograms\\Clubgg\\Debug\\HookEx.dll"

typedef LONG(NTAPI *NtSuspendProcess)(IN HANDLE ProcessHandle);
typedef LONG(NTAPI *NtResumeProcess)(IN HANDLE ProcessHandle);

struct handle_data {
	unsigned long process_id;
	HWND window_handle;
};

HHOOK hHook;

BOOL is_main_window(HWND handle)
{
	return GetWindow(handle, GW_OWNER) == (HWND)0 && IsWindowVisible(handle);
}

BOOL CALLBACK enum_windows_callback(HWND handle, LPARAM lParam)
{
	handle_data& data = *(handle_data*)lParam;
	unsigned long process_id = 0;
	GetWindowThreadProcessId(handle, &process_id);
	if (data.process_id != process_id || !is_main_window(handle))
		return TRUE;
	data.window_handle = handle;
	return FALSE;
}

HWND FindMainWindow(unsigned long process_id)
{
	handle_data data;
	data.process_id = process_id;
	data.window_handle = 0;
	EnumWindows(enum_windows_callback, (LPARAM)&data);
	return data.window_handle;
}


void suspend(DWORD processId)
{
	HANDLE processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);

	NtSuspendProcess pfnNtSuspendProcess = (NtSuspendProcess)GetProcAddress(
		GetModuleHandleA("ntdll"), "NtSuspendProcess");

	pfnNtSuspendProcess(processHandle);
	CloseHandle(processHandle);
}

void resume(DWORD processId)
{
	HANDLE processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);

	NtResumeProcess pfnNtResumeProcess = (NtResumeProcess)GetProcAddress(
		GetModuleHandleA("ntdll"), "NtResumeProcess");

	pfnNtResumeProcess(processHandle);
	CloseHandle(processHandle);
}

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

DWORD getPIDbyName(WCHAR * processName, DWORD parendPid = 0)
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

BOOL remoutLaodLibrary(DWORD pid, LPCWSTR dllPath)
{
	DWORD size = lstrlenW(dllPath) * 2;
	HANDLE hProcess = 0;
	LPVOID buf = NULL;
	LPVOID pLoadLibrary = NULL;

	if (EnableDebugPrivileges()) {

		hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, pid);

		pLoadLibrary = GetProcAddress(GetModuleHandle(L"kernel32.dll"), "LoadLibraryW");
		buf = VirtualAllocEx(hProcess, NULL, size, MEM_COMMIT, PAGE_READWRITE);

		if (buf && pLoadLibrary) {
			if (WriteProcessMemory(hProcess, buf, dllPath, size, NULL)) {
				CloseHandle(CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)pLoadLibrary, buf, 0, NULL));
				return 1;
			}
		}
	}
	return 0;
}


void setHook() {
	DWORD parentPid = getPIDbyName(ProcessName);
	DWORD pid = getPIDbyName(ProcessName, parentPid);
	printf("pid: %x\n", pid);
	HWND hWnd = FindMainWindow(pid);

	DWORD thread_id = GetWindowThreadProcessId(hWnd, NULL);	
	HMODULE dll = LoadLibrary(DllPath);
	//HOOKPROC address = (HOOKPROC)GetProcAddress(dll, "KeyboardProc");
	HOOKPROC address = (HOOKPROC)GetProcAddress(dll, "CallWndProc");
	hHook = SetWindowsHookEx(WH_KEYBOARD, address, dll, thread_id);
	getchar();
	UnhookWindowsHookEx(hHook);
}

int main()
{
	setHook();
	return 1;
/*
	HMODULE hMod = LoadLibraryW(DllPath);
	printf("%x\n", hMod);
	system("pause");
	return 0;
// */
	DWORD parentPid = getPIDbyName(ProcessName);
	DWORD pid = getPIDbyName(ProcessName, parentPid);
	printf("pid: %x\n", pid);

	if (EnableDebugPrivileges())
	{
		suspend(pid);

		if (remoutLaodLibrary(pid, DllPath))
			printf("[+] Transmitter dll has bean injected!\n");
		else
			printf("[-] Transmitter dll injected error!!\n");	

		resume(pid);
	}
	
	//system("pause");
    return 0;
}