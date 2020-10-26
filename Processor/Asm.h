#pragma once
#include <stdio.h>
#include "Logger.h"


#define Assert_c(expr) if(!(expr)) loggerAssert(#expr,__FILE__,__FUNCSIG__,__LINE__);  ///< Реализация assert для релиза переключить режим можно директивой #define NDEBUG


typedef unsigned char ui8;
typedef unsigned short ui16;
typedef unsigned int  ui32;

typedef ui16 Mcode;
typedef char* C_string;

const ui8 COMPILLER_COMMAND_HAS_TWO_OPERANDS            = 1 << 0;
const ui8 COMPILLER_COMMAND_FIRST_OPERAND_IS_NUBBER     = 1 << 1;
const ui8 COMPILLER_COMMAND_SECOND_OPERAND_IS_NUMBER    = 1 << 2;   

const int ASM_ERROR_CODE = -1;

enum AsmError
{
    ASM_OK = 0,
    ASM_ERROR_INVALID_INPUT_DATA = -1,
    ASM_ERROR_OUT_OF_MEMORY = -2,
    ASM_ERROR_GEN_LABLE_TABLE = -3,
    ASM_ERROR_GEN_MACHINE_CODE = -4,
    ASM_ERROR_CANT_WRITE_INTO_FILE = -5,
    ASM_ERROR_INVALID_SYNTAX = -6,
    ASM_ERROR_INVALID_MACHINE_CODE = -7,
    ASM_ERROR_INVALID_OPERANDS_NUMBER = -8,
    ASM_ERROR_INVALID_OPERAND_SYNTAX = -9,
    ASM_ERROR_INVALID_OPERAND_TYPE_FOR_COMMAND = -10
};


struct Command
{
    Mcode machineCode = 0;
    ui32 operand[2] = {};
    ui32 nOperands = 0;
};

enum OperandType
{
    OPERAND_REGISTER,
    OPERAND_NUMBER,
    OPERAND_MEMORY,
    OPERAND_MEM_BY_REG
};


inline ui8 getNumberOperands(Mcode marchCode);
inline void setOperandType(Mcode* marchCode, ui8 opIndex, OperandType type);
inline OperandType getOperandType(Mcode marchCode, ui8 opIndex);
inline Mcode getPureMachCode(Mcode machCode);

C_string getStringByErrorCode(AsmError errorCode);

AsmError compile(const char* code, FILE* outStream = stdout);

void disasmCommand(Command cmd, FILE* outStream = stdout);
AsmError disasm(const char* code,int size, FILE* outStream = stdout);