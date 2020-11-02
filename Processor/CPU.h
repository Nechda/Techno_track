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

void setStepByStepMode(bool flag); ///< Устанавливает режим работы процессора step by step

void cupInit(ui32 ramSize); ///< Функция инициализации структуры CPU
CPUerror cpuRunProgram(const char* programCode, int size, ui32 ptrStart = 0);/// Функция, запускающая программу на исполнение
void cpuDestr(); ///< Функция делает cleanUp структуры CPU

/*
\brief Структура, описывающая набор регистров общего назначения
*/
struct GeneralReg
{
    ui32 eax;
    ui32 ebx;
    ui32 ecx;
    ui32 edx;
};

/*
\brief  Пара функций для установки регистров общего назначения процессора
@{
*/
void setCpuRegisters(GeneralReg reg);
void getCpuRegisters(GeneralReg* reg);
/**}@*/
