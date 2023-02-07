#include "stdafx.h"
#include <string>
#include <iostream>
#include "Transmitter.h"
#include "debug.h"
#include "Process.h"
#include "WndProc.h"


BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		DbgPrint("Transmitter.dll Loaded!");
		SetHook(true);
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:		
	case DLL_PROCESS_DETACH:
		DbgPrint("Transmitter.dll UnLoaded!");
		break;
	}

	return TRUE;
}

#ifdef ES

#if _WIN64
#pragma comment(lib, "EasyHook64.lib")
#else
#pragma comment(lib, "EasyHook32.lib")
#endif

#include <easyhook.h>
#define ProcessName L"ClubGG.exe"

typedef LRESULT(WINAPI *WndProc)(HWND, UINT, WPARAM, LPARAM);

extern "C" void __declspec(dllexport) __stdcall NativeInjectionEntryPoint(REMOTE_ENTRY_INFO* inRemoteInfo);


WndProc address = NULL;

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


LRESULT WINAPI NewWndProc(HWND hWnd, UINT msgId, WPARAM wParam, LPARAM lParam) {
	//ProcessHook(msgId, wParam, lParam);
	//return address(hWnd, msgId, wParam, lParam);
	return 0;
}

void __stdcall NativeInjectionEntryPoint(REMOTE_ENTRY_INFO* inRemoteInfo)
{
	std::cout << "\n\nNativeInjectionEntryPointt(REMOTE_ENTRY_INFO* inRemoteInfo)\n\n" <<
		"IIIII           jjj               tt                dd !!! \n"
		" III  nn nnn          eee    cccc tt      eee       dd !!! \n"
		" III  nnn  nn   jjj ee   e cc     tttt  ee   e  dddddd !!! \n"
		" III  nn   nn   jjj eeeee  cc     tt    eeeee  dd   dd     \n"
		"IIIII nn   nn   jjj  eeeee  ccccc  tttt  eeeee  dddddd !!! \n"
		"              jjjj                                         \n\n";

	std::cout << "Injected by process Id: " << inRemoteInfo->HostPID << "\n";
	std::cout << "Passed in data size: " << inRemoteInfo->UserDataSize << "\n";
	if (inRemoteInfo->UserDataSize == sizeof(DWORD))
	{
		address = *reinterpret_cast<WndProc *>(inRemoteInfo->UserData);
		std::cout << "Old WndProc: " << address << "\n";
	}

	// Perform hooking
	HOOK_TRACE_INFO hHook = { NULL }; // keep track of our hook

	std::cout << "\n";
	std::cout << "Win32 Beep found at address: " << address << "\n";

	// Install the hook
	NTSTATUS result = LhInstallHook(
		address,
		NewWndProc,
		NULL,
		&hHook);
	if (FAILED(result))
	{
		std::wstring s(RtlGetLastErrorString());
		std::wcout << "Failed to install hook: ";
		std::wcout << s;
	}
	else
	{
		std::cout << "Hook 'myBeepHook installed successfully.";
	}

	// If the threadId in the ACL is set to 0,
	// then internally EasyHook uses GetCurrentThreadId()
	ULONG ACLEntries[1] = { 0 };

	// Disable the hook for the provided threadIds, enable for all others
	LhSetExclusiveACL(ACLEntries, 1, &hHook);

	return;
}
#endif