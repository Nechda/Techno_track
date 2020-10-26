#pragma once
#include "Asm.h"

enum CPUerror
{
    CPU_OK = 0,
    CPU_ERROR_INVALID_STRUCUTE,
    CPU_ERROR_INVALID_COMMAND,
    CPU_ERROR_EXCEPTION,
    CPU_ERROR_EPI_OUT_OF_RANE
};


C_string getStringByErrorCode(CPUerror errorCode);
void cupInit();
CPUerror cpuRunProgram(const char* programCode, int size, ui32 ptrStart = 0);
void cpuDump(FILE* outStream = stdout);