#include "stdafx.h"
#include <Windows.h>
#include "ProcessHook.h"
#include "debug.h"

void ProcessHook(UINT msgId, WPARAM wParam, LPARAM lParam)
{
	char debugData[MAX_PATH] = { 0 };
	PCOPYDATASTRUCT pCopyData = (PCOPYDATASTRUCT)lParam;

	if (msgId == WM_COPYDATA) {

		if (pCopyData->cbData && pCopyData->lpData) {
			wsprintfA(debugData, "CopyData size: %d, data: %s\n", 
				pCopyData->cbData, pCopyData->lpData);
			DbgPrint(debugData);
		}		
	}		
}