#pragma once
#include "Asm.h"

void cupInit();
void cpuRunProgram(const char* programCode, int size, ui32 ptrStart = 0);
void cpuDump(FILE* outStream = stdout);