#include "stdafx.h"
#include <Windows.h>
#include "Transmitter.h"
#include "ProcessHook.h"
#include "WndProc.h"
#include "Process.h"
#include "debug.h"

#define ProcessName L"ClubGG.exe"

#pragma pack(push, 1)
struct OriginalFuncBytes {
	BYTE body[FUNC_BYTES_TO_COPY];
} FuncData;
#pragma pack(pop)

#pragma pack(push, 1)
struct TRUMP {
	BYTE push;			// 0x68
	LPVOID address;		// xx xx xx xx
	BYTE ret;			// c3		
} JMP;
#pragma pack(pop)

WndProc address = NULL;
HANDLE mutex = NULL;

void getWndAddress(WndProc &address) {
	char debugData[0x1000] = { 0 };
	char windowText[MAX_PATH] = { 0 };
	DWORD parentPid = getPIDbyName(ProcessName);
	DWORD pid = getPIDbyName(ProcessName, parentPid);
	HWND hWnd = FindMainWindow(pid);
	HMODULE unityPlayerModule = GetWndInstance(hWnd);

	GetWindowTextA(hWnd, windowText, sizeof(windowText));	

	address = (WndProc)GetWndProc(hWnd);	
	wsprintfA(debugData, "[+] pid: %d, %s - 0x%X\n", pid, windowText, hWnd);
	wsprintfA(debugData, "%s\n\t UnityPlayer.dll: 0x%X\n", debugData, unityPlayerModule);
	wsprintfA(debugData, "%s\n\t WndProc: 0x%X\n", debugData, address);
	
	DbgPrint(debugData);	
}

void SetHook(bool set) {
	DWORD oldProtect = 0;

	getWndAddress(address);
	
	VirtualProtect(address, 1, PAGE_EXECUTE_READWRITE, &oldProtect);

	__asm
	{
		int 3;
	}
	if (set) {
		mutex = CreateMutexA(NULL, TRUE, "GG-HOOK");
		if (GetLastError() != ERROR_ALREADY_EXISTS)
		{
			MoveMemory(FuncData.body, address, FUNC_BYTES_TO_COPY);
			JMP.push = 0x68;
			JMP.address = NewWndProc;
			JMP.ret = 0xc3;
			MoveMemory(address, &JMP, FUNC_BYTES_TO_COPY);
		}
	}
	else
		MoveMemory(address, FuncData.body, FUNC_BYTES_TO_COPY);

	VirtualProtect(address, 1, oldProtect, &oldProtect);
	if (!set && mutex)
		ReleaseMutex(mutex);
}

LRESULT WINAPI TrueWndProc(HWND hWnd, UINT msgId, WPARAM wParam, LPARAM lParam) {
	LRESULT result;
	SetHook(false);
	result = address(hWnd, msgId, wParam, lParam);
	SetHook(true);
	return result;
}

LRESULT WINAPI NewWndProc(HWND hWnd, UINT msgId, WPARAM wParam, LPARAM lParam) {
	//ProcessHook(msgId, wParam, lParam);
	return NewWndProc(hWnd, msgId, wParam, lParam);
}