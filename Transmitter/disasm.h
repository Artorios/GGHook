#pragma once
#include <Windows.h>
#include <stdio.h>

extern "C" {
#include "beaengine\BeaEngine.h"
}

void DisassembleCode(char *start_offset, int size);
