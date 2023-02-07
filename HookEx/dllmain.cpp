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

int filterException(int code, PEXCEPTION_POINTERS ex)
{
	char debugData[1000] = { 0 };
	wsprintfA(debugData, "Filtering: %x", code);	
	SendPacket(ip, port, debugData, strnlen_s(debugData, 2000));
	return EXCEPTION_EXECUTE_HANDLER;
}

__declspec(dllexport) LRESULT CALLBACK CallWndProc(int nCode, WPARAM wParam, LPARAM lParam)
{	
	char debugData[1000] = { 0 };
#pragma EXPORT	
	__try
	{
		PCWPSTRUCT msgData = (PCWPSTRUCT)lParam;
		//if (msgData->message == WM_COPYDATA) {
		wsprintfA(debugData, "[+] WM_COPYDATA: mdgId: %x, wParam: %x, lParam: 0x%x",
			msgData->message, wParam, lParam);
		SendPacket(ip, port, debugData, strnlen_s(debugData, 2000));
		//}
	}
	__except (filterException(GetExceptionCode(), GetExceptionInformation()))
	{

		//SendPacket(ip, port, debugData, strnlen_s(debugData, 2000));
	}


	if (nCode >= HC_ACTION) {		
		PCWPSTRUCT msgData = (PCWPSTRUCT)lParam;
	}
	return CallNextHookEx(hHook, nCode, wParam, lParam);
}

__declspec(dllexport) LRESULT CALLBACK GetMsgProc(int nCode, WPARAM wParam, LPARAM lParam)
{
#pragma EXPORT
	char debugData[0x1000] = { 0 };
	
	if (nCode >= HC_ACTION) {
		__try {
			PMSG msgData = (PMSG)lParam;			
			wsprintfA(debugData, "[+] GetMsgProc: nCode: %x, wParam: %x, lParam: 0x%x",
				nCode, wParam, lParam);
			SendPacket(ip, port, debugData, strnlen_s(debugData, 2000));
		}
		__except (filterException(GetExceptionCode(), GetExceptionInformation())) {}
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

__declspec(dllexport) LRESULT CALLBACK CBTProc(int nCode, WPARAM wParam, LPARAM lParam)
{
#pragma EXPORT	
	char debugData[1000] = { 0 };
	if (nCode >= HC_ACTION) {
		PCWPSTRUCT msgData = (PCWPSTRUCT)lParam;
		__try
		{
			//if (wParam && msgData->message == WM_COPYDATA) {
			wsprintfA(debugData, "[+] CBTProc: nCode: %x, wParam: %x, lParam: 0x%x",
				nCode, wParam, lParam);
			SendPacket(ip, port, debugData, strnlen_s(debugData, 2000));
			//}
		}
		__except (filterException(GetExceptionCode(), GetExceptionInformation()))
		{
		}
	}
	return CallNextHookEx(hHook, nCode, wParam, lParam);
}

void setHook(HMODULE hModule)
{	
	LoadLibraryA("user32.dll");
	DWORD pid = GetCurrentProcessId();
	HWND hWnd = FindMainWindow(pid);
	DWORD threadId = GetWindowThreadProcessId(hWnd, NULL);
	hHook = SetWindowsHookEx(WH_CALLWNDPROC, CallWndProc, 0, threadId);	
	//hHook = SetWindowsHookEx(WH_GETMESSAGE, GetMsgProc, 0, threadId);
	//hHook = SetWindowsHookEx(WH_KEYBOARD, KeyboardProc, 0, threadId);

	//hHook = SetWindowsHookEx(WH_CBT, GetMsgProc, 0, threadId);


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

