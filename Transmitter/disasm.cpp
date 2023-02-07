#include "stdafx.h"
#include "disasm.h"
#include "debug.h"

void DisassembleCode(char *start_offset, int size)
{
	char disAsmData[0x1000] = {};
	DISASM infos;
	int len;
	char *end_offset = (char*)start_offset + size;

	(void)memset(&infos, 0, sizeof(DISASM));
	infos.EIP = (UInt64)start_offset;

	while (!infos.Error) {
		infos.SecurityBlock = (int)end_offset - infos.EIP;
		if (infos.SecurityBlock <= 0) break;
		len = Disasm(&infos);
		switch (infos.Error)
		{
		case OUT_OF_BLOCK:
			//(void)printf("disasm engine is not allowed to read more memory \n");
			break;
		case UNKNOWN_OPCODE:
			//(void)printf("%s\n", &infos.CompleteInstr);
			infos.EIP += 1;
			infos.Error = 0;
			break;
		default:
			sprintf_s(disAsmData, "%s\n", &infos.CompleteInstr);
			//(void)printf("%s\n", &infos.CompleteInstr);
			DbgPrint(disAsmData);
			infos.EIP += len;
		}
	};
	return;
}