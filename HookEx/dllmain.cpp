#include "stdafx.h"
#include <Tlhelp32.h>
#include "WndProc.h"
#include "SendTCP.h"

#define ProcessName L"ClubGG.exe"
#define EXPORT comment(linker, "/EXPORT:" __FUNCTION__ "=" __FUNCDNAME__)
#define ip "127.0.0.1"
#define port 9090

HHOOK hHook;

void DbgPrint(char * str)
{
	CHAR data[0x1000] = { 0 };
	HWND wndMain = FindWindowA("Notepad", NULL);
	HWND edit = FindWindowExA(wndMain, 0, "Edit", NULL);

	memset(data, 0, sizeof(data));
	SendMessageA(edit, WM_GETTEXT, 255, (LPARAM)data);
	strcat_s(data, "\n");
	strcat_s(data, str);
	SendMessageA(edit, WM_SETTEXT, 0, (LPARAM)data);
}

LRESULT CALLBACK CallWndProc(int nCode, WPARAM wParam, LPARAM lParam)
{	
	if (nCode >= HC_ACTION) {
		char debugData[0x1000] = { 0 };
		PCWPSTRUCT msgData = (PCWPSTRUCT)lParam;
		if (msgData->message = WM_COPYDATA) {
			wsprintfA(debugData, "[+] WM_COPYDATA: nCode: %x, wParam: %x, lParam: %x\n", nCode, wParam, lParam);
			DbgPrint(debugData);
		}
	}

	return CallNextHookEx(hHook, nCode, wParam, lParam);
}

LRESULT CALLBACK GetMsgProc(int nCode, WPARAM wParam, LPARAM lParam) 
{
	char debugData[0x1000] = { 0 };
	
	if (nCode >= HC_ACTION) {
		PMSG msgData = (PMSG)lParam;
		memset(debugData, 0, sizeof(debugData));
		wsprintfA(debugData, "[+] GetMsgProc: nCode: %x \n", nCode);		
		DbgPrint(debugData);		
		SendPacket(ip, port, debugData, strnlen_s(debugData, 2000));
	}	
	return CallNextHookEx(hHook, nCode, wParam, lParam);
}

__declspec(dllexport) LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
#pragma EXPORT	
	if (nCode >= HC_ACTION) {
		char * message = "KeyboardProc\n";
		DbgPrint("KeyboardProc\n");		
		SendPacket(ip, port, message, strnlen_s(message, 2000));
	}
	return CallNextHookEx(hHook, nCode, wParam, lParam);
}

void setHook(HMODULE hModule)
{	
	LoadLibraryA("user32.dll");
	DWORD pid = GetCurrentProcessId();
	HWND hWnd = FindMainWindow(pid);
	DWORD threadId = GetWindowThreadProcessId(hWnd, NULL);
	//hHook = SetWindowsHookEx(WH_CALLWNDPROC, CallWndProc, NULL, threadId);
	//hHook = SetWindowsHookEx(WH_GETMESSAGE, GetMsgProc, hModule, 0);
	//hHook = SetWindowsHookEx(WH_GETMESSAGE, GetMsgProc, 0, threadId);
	hHook = SetWindowsHookEx(WH_KEYBOARD, KeyboardProc, 0, threadId);


	char debugData[0x1000] = { 0 };
	char windowText[MAX_PATH] = { 0 };

	GetWindowTextA(hWnd, windowText, sizeof(windowText));

	wsprintfA(debugData, "[+] hHook: %x, pid: %d, tid: %x, %s - 0x%X\n",
		hHook, pid, threadId, windowText, hWnd);

	DbgPrint(debugData);

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

BOOL needHook() {
	DWORD parentPid = getPIDbyName(ProcessName);
	DWORD pid = getPIDbyName(ProcessName, parentPid);
	return GetCurrentProcessId() == pid;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		//if (needHook())
			setHook(hModule);
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		UnhookWindowsHookEx(hHook);
		break;
	}
	return TRUE;
}

