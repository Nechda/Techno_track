#pragma once
#include <stdio.h>
#include "Logger.h"


/*
\brief Упростим себе жизнь, введя короткие названия стандартных типов
@{
*/
typedef unsigned char ui8;
typedef unsigned short ui16;
typedef unsigned int  ui32;

typedef char i8;
typedef short i16;
typedef int  i32;

typedef ui16 Mcode;
typedef char* C_string;
/*
@}
*/

const int ASM_ERROR_CODE = -1; /// Стандартный код ошибки, для вспомогательных функций
/*
\brief Коды ошибок, возвращаемые компилятором и дизасемблером
*/
enum AsmError
{
    ASM_OK = 0,
    ASM_ERROR_INVALID_INPUT_DATA,
    ASM_ERROR_OUT_OF_MEMORY,
    ASM_ERROR_GEN_LABLE_TABLE,
    ASM_ERROR_GEN_MACHINE_CODE,
    ASM_ERROR_CANT_WRITE_INTO_FILE,
    ASM_ERROR_INVALID_SYNTAX,
    ASM_ERROR_INVALID_MACHINE_CODE,
    ASM_ERROR_INVALID_OPERANDS_NUMBER,
    ASM_ERROR_INVALID_OPERAND_SYNTAX,
    ASM_ERROR_INVALID_OPERAND_TYPE_FOR_COMMAND,
    ASM_ERROR_CANT_READ_LEXEMA
};


union OperandUnion
{
    ui32 ivalue;
    float fvalue;
};

/*
\brief Посредствам данной стуктуры реализуется "команда" процессора
*/
struct Command
{
    Mcode machineCode = 0;
    OperandUnion operand[2] = {0, 0};
    ui32 nOperands = 0;
};




/*
\brief Допустимые типы операндов
*/
enum OperandType
{
    OPERAND_REGISTER,
    OPERAND_NUMBER,
    OPERAND_MEMORY,
    OPERAND_MEM_BY_REG
};



int readFullFile(const char* filename, char** outString); ///< Функция считывает полностью файл в буффер
int removeExtraChar(char** ptrStr, const char* dontDelChar = ""); ///< Функция удаляет из строки все лишние символы

/*
\brief Функции для работы с машинным кодом, позволяющие быстро определеть свойства команды
\details Подробное описание каждой функции смотри в Asm.cpp
@{
*/
inline ui8 getNumberOperands(Mcode marchCode);
inline void setOperandType(Mcode* marchCode, ui8 opIndex, OperandType type);
inline OperandType getOperandType(Mcode marchCode, ui8 opIndex);
inline Mcode getPureMachCode(Mcode machCode);
/*
@}
*/

void prepareCompilatorsTable(); ///< Функция формирует таблицу команд, с которыми может работать компилятор

C_string getStringByErrorCode(AsmError errorCode); ///< По коду ошибки восстанавливаем строку

AsmError compile(const char* code, FILE* outStream = stdout); ///< Функция, производящая компиляцию ассемблерного кода

void disasmCommand(Command cmd, FILE* outStream = stdout); ///< Функция, производяющая дизасемблирование команды, а результат закидывается в outStream
AsmError disasm(const ui8* code,int size, FILE* outStream = stdout);///< Функция, производящая дизасемблирование кода, а результат закидывается в outStream