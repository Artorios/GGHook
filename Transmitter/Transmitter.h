#pragma once
#include <Windows.h>

#define FUNC_BYTES_TO_COPY 6

typedef LRESULT (WINAPI *WndProc)(HWND, UINT, WPARAM, LPARAM);

void SetHook(bool set = true);

LRESULT WINAPI TrueWndProc(HWND hWnd, UINT msgId, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI NewWndProc(HWND hWnd, UINT msgId, WPARAM wParam, LPARAM lParam);