#pragma once
#include <Windows.h>

HWND FindMainWindow(unsigned long process_id);

HMODULE GetWndInstance(HWND hWnd);

LPVOID GetWndProc(HWND hWnd);