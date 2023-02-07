#include "debug.h"
#include <Windows.h>

void DbgPrint(char * str)
{
	CHAR data[0x1000] = { 0 };
	HWND wndMain = FindWindowA("Notepad", NULL);
	HWND edit = FindWindowExA(wndMain, 0, "Edit", NULL);

	SendMessageA(edit, WM_GETTEXT, 255, (LPARAM)data);
	strcat_s(data, "\n");
	strcat_s(data, str);
	SendMessageA(edit, WM_SETTEXT, 0, (LPARAM)data);
}