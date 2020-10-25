#include "Asm.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <iostream>


struct Lexema
{
    char* command;
    Mcode  machineCode;
};


/*
byte stucture of command
            10                     2                       2                 2
 command general code | type of second operand | type of first operand  | nOperands
*/

/*
    todo:
        ƒобавить возможность командам обращатьс€ к RAM через [...]
*/


const Lexema Table[] = {
    {"mov",     1 << 6 | 0 << 4 | 0 << 2 | 0x2 },  ///< done
    {"add",     2 << 6 | 0 << 4 | 0 << 2 | 0x2 },  ///< done
    {"sub",     3 << 6 | 0 << 4 | 0 << 2 | 0x2 },  ///< done
    {"div",     4 << 6 | 0 << 4 | 0 << 2 | 0x2 },  ///< done
    {"mul",     5 << 6 | 0 << 4 | 0 << 2 | 0x2 },  ///< done
    {"pop",     6 << 6 | 0 << 4 | 0 << 2 | 0x1 },  ///< done
    {"push",    7 << 6 | 0 << 4 | 0 << 2 | 0x1 },  ///< done
    {"jmp",     8 << 6 | 0 << 4 | 0 << 2 | 0x1 },  ///< done
    {"hlt",     9 << 6 | 0 << 4 | 0 << 2 | 0x0 },  ///< done
    {"cmp",    10 << 6 | 0 << 4 | 0 << 2 | 0x2 },  ///< process (implemented CF,ZF,SF )
    {"je",     11 << 6 | 0 << 4 | 0 << 2 | 0x1 },  ///< done
    {"jne",    12 << 6 | 0 << 4 | 0 << 2 | 0x1 },  ///< done
    {"ja",     13 << 6 | 0 << 4 | 0 << 2 | 0x1 },  ///< done
    {"jae",    14 << 6 | 0 << 4 | 0 << 2 | 0x1 },  ///< done
    {"jb",     15 << 6 | 0 << 4 | 0 << 2 | 0x1 },  ///< done
    {"jbe",    16 << 6 | 0 << 4 | 0 << 2 | 0x1 },  ///< done
    {"call",   17 << 6 | 0 << 4 | 0 << 2 | 0x1 },  ///< done
    {"ret",    18 << 6 | 0 << 4 | 0 << 0 | 0x0 }   ///< done
};

const Lexema Registers[] =
{
    {"eax",     1},
    {"ebx",     2},
    {"ecx",     3},
    {"edx",     4},
    {"esi",     5},
    {"edi",     6},
    {"esp",     7},
    {"ebp",     8},
    {"eip",     9},
    {"eflags", 10},
    {"ecs",    11},
    {"eds",    12},
    {"ess",    13}
};


const ui32 COMMAND_TABLE_SIZE = sizeof(Table)/sizeof(Lexema);
const ui32 REGISTER_TABLE_SIZE = sizeof(Registers) / sizeof(Lexema);

ui8 getNumberOperands(Mcode marchCode)
{
    return marchCode & 0x3;
}

void setOperandType(Mcode* marchCode, ui8 opIndex, OperandType type)
{
    *marchCode &= ~(0b11 << (2 * opIndex + 2));
    *marchCode |= ((ui8)type & 0b11) << (2 * opIndex + 2);
}

OperandType getOperandType(Mcode marchCode, ui8 opIndex)
{
    ui8 shift = (2 * opIndex + 2);
    marchCode >>= shift;
    marchCode &= 0b11;
    return (OperandType)(marchCode);
}

Mcode getPureMachCode(Mcode machCode)
{
    return machCode & (~0b111100);
}

struct
{
    enum LexemaType
    {
        LEX_ERROR = -1,
        LEX_COMMAND = 1,
        LEX_REGISTER = 2,
        LEX_NUMBER = 3,
        LEX_LABEL = 4,
        LEX_MEMORY = 5,
        LEX_MEM_BY_REG = 6
    };

    struct Label
    {
        char name[32];
        ui32 pos;
    };

    static inline bool isNumber(const char* str)
    {
        Assert_c(str);
        if (!str)
            return false;

        if (strstr(str, "0x"))
            return true;
        while (*str)
        {
            if (!isdigit(*str))
                return false;
            str++;
        }
        return true;
    }

    static LexemaType getLexemaType(const char* str)
    {
        static char buffer[32] = {};
        Assert_c(str);
        if (!str)
            return LEX_ERROR;

        if (strchr(str, ':'))
            return LEX_LABEL;

        if (isNumber(str))
            return LEX_NUMBER;

        for (int i = 0; i < COMMAND_TABLE_SIZE; i++)
            if (!strcmp(str, Table[i].command))
                return LEX_COMMAND;

        LexemaType result = LEX_ERROR;
        if (strchr(str, '[') && strchr(str, ']'))
        {
            sscanf(str, "[%s]", buffer);
            strchr(buffer, ']')[0] = 0;
            str = buffer;
            result = LEX_MEMORY;
        }

        for (int i = 0; i < REGISTER_TABLE_SIZE; i++)
            if (!strcmp(str, Registers[i].command))
            {
                result = result == LEX_ERROR ? LEX_REGISTER : LEX_MEM_BY_REG;
                break;
            }


        return result;
    }

    static int genLableTable(const char* codeLine,Label* lables)
    {
        Assert_c(codeLine);
        Assert_c(lables);
        if (!codeLine || !lables)
            return ASM_ERROR_CODE;

        static char buf[32] = {};
        ui32 nBytes = 0;
        LexemaType lxt;
        while (*codeLine)
        {
            sscanf(codeLine, "%s ", buf);
            codeLine += strlen(buf) + 1;
            lxt = getLexemaType(buf);
            switch (lxt)
            {
            case LEX_COMMAND:
                nBytes+=sizeof(Mcode);
                break;
            case LEX_LABEL:
                lables->pos = nBytes;
                memcpy(lables->name, buf, 32);
                *(strchr(lables->name, ':')) = 0;
                lables++;
                break;
            case LEX_NUMBER:
                nBytes += sizeof(ui32);
                break;
            case LEX_REGISTER:
                nBytes++;
                break;
            case LEX_MEMORY:
                nBytes += sizeof(ui32);
                break;
            case LEX_MEM_BY_REG:
                nBytes += sizeof(ui8);
                break;
            default:
                nBytes += sizeof(ui32);
                break;
            }
        }
        return nBytes;
    }

    static int getLabel(char* labelName, Label* lables, ui32 nLables)
    {
        Assert_c(labelName);
        Assert_c(lables);
        if (!labelName || !lables)
            return ASM_ERROR_CODE;

        for (int i = 0; i < nLables; i++)
            if (!strcmp(lables[i].name, labelName))
                return i;
        return ASM_ERROR_CODE;
    }

    static inline void* readNextLexema(char** ptrCodeLine,LexemaType* lxt)
    {
        Assert_c(ptrCodeLine);
        Assert_c(lxt);
        if (!ptrCodeLine || !lxt)
            return NULL;
        Assert_c(*ptrCodeLine);
        if(!*ptrCodeLine)
            return NULL;

        static char buffer[32] = {};
        static ui32 bufInt = 0;
        sscanf(*ptrCodeLine, "%s ", buffer);
        *ptrCodeLine = strchr(*ptrCodeLine, ' ') + 1;

        *lxt = getLexemaType(buffer);
        
        if(*lxt == LEX_COMMAND)
            for(int i=0;i<COMMAND_TABLE_SIZE;i++)
                if (!strcmp(buffer, Table[i].command))
                {
                    bufInt = Table[i].machineCode;
                    return &bufInt;
                }
        if (*lxt == LEX_REGISTER)
            for (int i = 0; i<REGISTER_TABLE_SIZE; i++)
                if (!strcmp(buffer, Registers[i].command))
                {
                    bufInt = Registers[i].machineCode;
                    return &bufInt;
                }
        if (*lxt == LEX_NUMBER)
        {
            if (strchr(buffer, 'x'))
            {
                sscanf(buffer, "0x%X", &bufInt);
                return &bufInt;
            }
            else
            {
                sscanf(buffer, "%d", &bufInt);
                return &bufInt;
            }
        }

        return &buffer;
    }

    static int getMemoryOperand(char* str, Label* lables, ui32 nLables)
    {
        Assert_c(str);
        if (!str)
            return ASM_ERROR_CODE;
        if (str[0] != '[')
            return ASM_ERROR_CODE;
        str++;

        Assert_c(lables);
        if (!lables)
            return ASM_ERROR_CODE;

        char* endStr = strchr(str, ']');
        Assert_c(endStr);
        if (!endStr)
            return ASM_ERROR_CODE;
        *endStr = 0;
        
        int resOperand = 0;

        if (isNumber(str))
        {
            if (strchr(str, 'x'))
                sscanf(str, "0x%X", &resOperand);
            else
                sscanf(str, "%d", &resOperand);
            return resOperand;
        }

        resOperand = getLabel(str, lables, nLables);
        if (resOperand != ASM_ERROR_CODE)
            return lables[resOperand].pos;;

        for (int i = 0; i<sizeof(Registers) / sizeof(Lexema); i++)
            if (!strcmp(str, Registers[i].command))
                return Registers[i].machineCode;

        return ASM_ERROR_CODE;
    }

    static AsmError genBytes(const char* codeLine,Label* lables, ui32 nLables,ui8* ptrBytes)
    {
        Assert_c(codeLine);
        Assert_c(lables);
        Assert_c(ptrBytes);
        if (!codeLine || !lables || !ptrBytes)
            return ASM_ERROR_INVALID_INPUT_DATA;


        char* ptr = NULL;
        ui8* bytes = ptrBytes;
        int currentPosition = 0;
        LexemaType lxt;
        Command cmd;
        while (*codeLine)
        {
            ptr = (char*)readNextLexema((char**)&codeLine,&lxt);
            if (lxt == LEX_LABEL)
                continue;
            if (lxt != LEX_COMMAND)
            {
                logger("Compilator error", "Lexema: \"%s\" is not command, but should be...",ptr);
                return ASM_ERROR_INVALID_SYNTAX;
            }
            
            cmd.machineCode = *((Mcode*)ptr);
            currentPosition += sizeof(Mcode);
            cmd.nOperands = getNumberOperands(cmd.machineCode);
            for (int i = 0; i < cmd.nOperands; i++)
            {
                ptr = (char*)readNextLexema((char**)&codeLine, &lxt);
                if (lxt == LEX_NUMBER)
                {
                    cmd.operand[i] = *((ui32*)ptr);
                    setOperandType(&cmd.machineCode, i, OPERAND_NUMBER);
                    currentPosition += sizeof(ui32);
                }
                if (lxt == LEX_REGISTER)
                {
                    cmd.operand[i] = *((ui8*)ptr);
                    setOperandType(&cmd.machineCode, i, OPERAND_REGISTER);
                    currentPosition += sizeof(ui8);
                }
                if (lxt == LEX_MEMORY || lxt == LEX_MEM_BY_REG)
                {
                    cmd.operand[i] = getMemoryOperand(ptr,lables,nLables);
                    if (cmd.operand[i] == ASM_ERROR_CODE)
                    {
                        logger("Compilator error", "Invalid link to memory: \"%s\" ", ptr);
                        return ASM_ERROR_INVALID_OPERAND_SYNTAX;
                    }
                    setOperandType(&cmd.machineCode, i, lxt == LEX_MEMORY ? OPERAND_MEMORY : OPERAND_MEM_BY_REG);
                    currentPosition += lxt == LEX_MEMORY ? sizeof(ui32) : sizeof(ui8);
                }
                if (lxt == LEX_LABEL)
                {
                    logger("Compilator error", "Invalid operand: \"%s\" ", ptr);
                    return ASM_ERROR_INVALID_OPERAND_SYNTAX;
                }
                if (lxt == LEX_ERROR)
                {
                    int labelIndex = getLabel(ptr, lables, nLables);
                    if (labelIndex == -1)
                    {
                        logger("Compilator error", "Invalid lexema: \"%s\" ", ptr);
                        return ASM_ERROR_INVALID_SYNTAX;
                    }
                    setOperandType(&cmd.machineCode, i, OPERAND_NUMBER);
                    currentPosition += sizeof(ui32);
                    cmd.operand[i] = lables[labelIndex].pos;
                }
                
            }
            
            *((Mcode*)bytes) = cmd.machineCode;
            bytes += sizeof(Mcode);

            for (int i = 0; i < cmd.nOperands; i++)
            {
                OperandType opType = getOperandType(cmd.machineCode, i);
                if (opType == OPERAND_NUMBER || opType == OPERAND_MEMORY)
                {
                    *((ui32*)bytes) = cmd.operand[i];
                    bytes += sizeof(ui32);
                }
                if (opType == OPERAND_REGISTER || opType == OPERAND_MEM_BY_REG)
                {
                    *((ui8*)bytes) = cmd.operand[i];
                    bytes += sizeof(ui8);
                }
            }

        }

        return ASM_OK;
    }

    static AsmError throwMachineCodeIntoStream(ui8* bytes, ui32 nBytes, FILE* outStream = stdout)
    {
        Assert_c(bytes);
        Assert_c(outStream);
        if (!bytes || !outStream)
            return ASM_ERROR_INVALID_INPUT_DATA;
        if (ferror(outStream))
        {
            logger("Access error", "There are problems with file stream...");
            return ASM_ERROR_CANT_WRITE_INTO_FILE;
        }

        if (outStream == stdout)
        {
            for (int i = 0; i < nBytes; i++)
            {
                printf("0x%02X ", bytes[i] & 0xFF);
                if ( (i+1) % 16 == 0)
                    printf("\n");
            }
        }
        else
            fwrite(bytes, sizeof(ui8), nBytes, outStream);

        return ASM_OK;
    }

    static AsmError getCode(C_string codeLine, ui32 nLabels, FILE* outStream = stdout)
    {
        Assert_c(codeLine);
        Assert_c(outStream);
        if (!codeLine || !outStream)
            return ASM_ERROR_INVALID_INPUT_DATA;

        Label* lables = (Label*)calloc(nLabels, sizeof(Label));
        Assert_c(lables);
        if (!lables)
            return ASM_ERROR_OUT_OF_MEMORY;
        ui32 nBytes = genLableTable(codeLine, lables);
        Assert_c(nBytes != -1);
        if (nBytes == -1)
        {
            free(lables);
            return ASM_ERROR_GEN_LABLE_TABLE;
        }
        ui8* bytes = (ui8*)calloc(nBytes + 1, sizeof(ui8));
        Assert_c(bytes);
        if (!bytes)
        {
            free(lables);
            return ASM_ERROR_OUT_OF_MEMORY;
        }

        AsmError errorCode = ASM_OK;

        errorCode = genBytes(codeLine,lables, nLabels, bytes);
        if (errorCode != ASM_OK)
        {
            free(lables);
            free(bytes);
            return ASM_ERROR_GEN_MACHINE_CODE;
        }

        errorCode = throwMachineCodeIntoStream(bytes, nBytes, outStream);
        
        free(lables);
        free(bytes);

        if (errorCode != ASM_OK)
            return ASM_ERROR_CANT_WRITE_INTO_FILE;

        return ASM_OK;
    }

}Compiler;

struct
{

    static C_string getCommandName(Command cmd)
    {
        cmd.machineCode = getPureMachCode(cmd.machineCode);
        for (int i = 0; i < COMMAND_TABLE_SIZE; i++)
            if (Table[i].machineCode == cmd.machineCode)
                return Table[i].command;
        return NULL;
    }

    static C_string getRegisterName(ui8 machineCode)
    {
        for (int i = 0; i < REGISTER_TABLE_SIZE; i++)
            if (Registers[i].machineCode == machineCode)
                return Registers[i].command;
        return NULL;
    }

    static C_string getStrByNumber(ui32 num)
    {
        static char buff[32] = {};
        sprintf(buff, "0x%X", num);
        return buff;
    }

    static void disasmCommand(Command cmd,FILE* outStream = stdout)
    {
        C_string commandName = NULL;
        C_string operandStr[2] = { NULL, NULL };

        commandName = getCommandName(cmd);
        if (!commandName)
        {
            logger("Disassembler error", "The 0x%X code doesn't match any commands", cmd.machineCode & 0xFFFF);
            return;
        }

        bool isInvalidOperands = 0;
        for (int i = 0; i < cmd.nOperands; i++)
        {
            OperandType opType = getOperandType(cmd.machineCode, i);
            if (opType == OPERAND_NUMBER || opType == OPERAND_MEMORY)
                operandStr[i] = getStrByNumber(cmd.operand[i]);
            if (opType == OPERAND_REGISTER || opType == OPERAND_MEM_BY_REG)
                operandStr[i] = getRegisterName(cmd.operand[i]);
        }

        if (isInvalidOperands)
        {
            logger("Disassembler error", "Invalid code for operands!");
            return;
        }

        fprintf(outStream, "%s ", commandName);
        for (int i = 0; i < cmd.nOperands; i++)
        {
            OperandType opType = getOperandType(cmd.machineCode, i);
            if (opType == OPERAND_REGISTER || opType == OPERAND_NUMBER)
                fprintf(outStream, "%s", operandStr[i]);
            if (opType == OPERAND_MEMORY || opType == OPERAND_MEM_BY_REG)
                fprintf(outStream, "[%s]", operandStr[i]);
            if (cmd.nOperands == 2 && i == 0)
                fprintf(outStream, ", ");
        }
        fprintf(outStream, "\n");
    }

    static AsmError getCode(ui8* bytes, ui32 nBytes, FILE* outStream = stdout)
    {
        Assert_c(bytes);
        Assert_c(outStream);
        if (!bytes || !outStream)
            return ASM_ERROR_INVALID_INPUT_DATA;
        if (ferror(outStream))
            return ASM_ERROR_CANT_WRITE_INTO_FILE;

        ui8* endPtr = bytes + nBytes;
        ui8* strPtr = bytes;
        Command cmd;
        C_string commandName = NULL;
        C_string operandStr[2] = {NULL, NULL};

        fprintf(outStream, "Offset: Lexema:\n");
        while (bytes < endPtr)
        {
            cmd.machineCode = *((Mcode*)bytes);
            fprintf(outStream, "0x%04X: ", bytes - strPtr );
            bytes += sizeof(Mcode);
            commandName = getCommandName(cmd);
            if (!commandName)
            {
                logger("Disassembler error", "The 0x%X code doesn't match any commands", cmd.machineCode & 0xFFFF);
                return ASM_ERROR_INVALID_MACHINE_CODE;
            }


            cmd.nOperands = getNumberOperands(cmd.machineCode);
            if (cmd.nOperands > 2)
            {
                logger("Disassembler error", "Invalid number of operands: %d, should be less than 3", cmd.nOperands);
                return ASM_ERROR_INVALID_OPERANDS_NUMBER;
            }

            bool isInvalidOperands = 0;
            for (int i = 0; i < cmd.nOperands; i++)
            {
                OperandType opType = getOperandType(cmd.machineCode, i);
                if (opType == OPERAND_NUMBER || opType == OPERAND_MEMORY)
                {
                    cmd.operand[i] = *((ui32*)bytes);
                    bytes += sizeof(ui32);
                    operandStr[i] = getStrByNumber(cmd.operand[i]);
                    
                }
                if (opType == OPERAND_REGISTER || opType == OPERAND_MEM_BY_REG)
                {
                    cmd.operand[i] = *((ui8*)bytes);
                    bytes += sizeof(ui8);
                    operandStr[i] = getRegisterName(cmd.operand[i]);
                }
                isInvalidOperands |= !operandStr[i];
            }

            if (isInvalidOperands)
            {
                logger("Disassembler error", "Invalid code for operands!");
                return ASM_ERROR_INVALID_OPERAND_SYNTAX;
            }


            fprintf(outStream, "%s ", commandName);
            for (int i = 0; i < cmd.nOperands; i++)
            {
                OperandType opType = getOperandType(cmd.machineCode, i);
                if (opType == OPERAND_REGISTER || opType == OPERAND_NUMBER)
                    fprintf(outStream, "%s", operandStr[i]);
                if (opType == OPERAND_MEMORY || opType == OPERAND_MEM_BY_REG)
                    fprintf(outStream, "[%s]", operandStr[i]);
                if (cmd.nOperands == 2 && i == 0)
                    fprintf(outStream, ", ");
            }
            fprintf(outStream, "\n");
        }
        return ASM_OK;
    }

}Disassembler;


AsmError compile(const char* code, FILE* outStream)
{
    Assert_c(code);
    Assert_c(outStream);
    if (!code || !outStream)
        return ASM_ERROR_INVALID_INPUT_DATA;

    C_string codeStr = NULL;
    ui32 nLabels = 0;
    {
        int strLen = strlen(code);
        codeStr = (C_string)calloc(strLen + 2, sizeof(ui8));
        Assert_c(codeStr);
        if (!codeStr)
            return ASM_ERROR_OUT_OF_MEMORY;
        ui8 c[2] = {};
        bool pingpong = 0;
        int index = 0;
        for (int i = 0; i < strLen; i++)
        {
            c[pingpong] = code[i];
            if (c[pingpong] == ':')
                nLabels++;

            #define isValidChr(x) (isalpha(x) || isdigit(x) || x==':' || x == '_' || x == '[' || x == ']')
            if (!isValidChr(c[pingpong]) && !isValidChr(c[pingpong ^ 1]))
                continue;
            if (isValidChr(code[i]))
                codeStr[index] = code[i];
            else
                codeStr[index] = ' ';
            index++;
            #undef isValidChr(x)

            pingpong ^= 1;
        }
        if (codeStr[index - 1] != ' ')
            codeStr[index++] = ' ';
        codeStr[index++] = 0;
        codeStr = (C_string)realloc(codeStr, index * sizeof(ui8));
        Assert_c(codeStr);
        if (!codeStr)
            return ASM_ERROR_OUT_OF_MEMORY;
    }

    AsmError errorCode = ASM_OK;
    errorCode = Compiler.getCode(codeStr, nLabels, outStream);
    free(codeStr);
    return errorCode;
}

AsmError disasm(const char* code, int size, FILE* outStream)
{
    return Disassembler.getCode((ui8*)code, size, outStream);;
}

void disasmCommand(Command cmd)
{
    Disassembler.disasmCommand(cmd);
}