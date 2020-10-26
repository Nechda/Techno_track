#pragma once
#include "Asm.h"

/*
\brief Коды ошибок, возвращаемые процессором
*/
enum CPUerror
{
    CPU_OK = 0,
    CPU_ERROR_INVALID_STRUCUTE,
    CPU_ERROR_INVALID_COMMAND,
    CPU_ERROR_EXCEPTION,
    CPU_ERROR_EPI_OUT_OF_RANE,
    CPU_INVALID_INPUT_DATA
};


C_string getStringByErrorCode(CPUerror errorCode);///< По коду ошибки восстанавливаем строку

void cupInit(); ///< Функция инициализации структуры CPU
CPUerror cpuRunProgram(const char* programCode, int size, ui32 ptrStart = 0);/// Функция, запускающая программу на исполнение