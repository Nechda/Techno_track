#include "Asm.h"
#include "CPU.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "Stack/Stack_kernel.h"
#define TYPE_ ui8
#include "Stack/Stack.h"
#undef TYPE_

const ui32 CPU_RAM_SIZE = 128;

const Mcode ASM_HLT = 9 << 6 | 0 << 4 | 0 << 2 | 0x0;

const ui8 FLAG_CF = 0;
const ui8 FLAG_ZF = 6;
const ui8 FLAG_SF = 7;

struct
{
    bool isValid = 0;
    struct
    {
        ui32 eax;
        ui32 ebx;
        ui32 ecx;
        ui32 edx;
        ui32 esi;
        ui32 edi;
        ui32 esp;
        ui32 ebp;
        ui32 eip;
        ui32 eflags;
        ui32 ecs;
        ui32 eds;
        ui32 ess;
    }Register;
    ui8* RAM = NULL;
    Stack(ui8) stack;
}CPU;

ui32* getRegisterPtr(ui8 number)
{
    if (number > sizeof(CPU.Register)/sizeof(ui32))
        return NULL;
    return (ui32*)&CPU.Register + (number - 1);
}

void cupInit()
{
    CPU.RAM = (ui8*)calloc(CPU_RAM_SIZE, sizeof(ui8));
    memset(CPU.RAM, 0, CPU_RAM_SIZE * sizeof(ui8));
    Assert_c(CPU.RAM);
    if (!CPU.RAM)
    {
        CPU.isValid = 0;
        logger("CPU error", "We can't alloc memory for virtual RAM.");
        return;
    }
    StackError errorCode = (StackError)stackInit(&CPU.stack, 0);
    Assert_c(!errorCode);
    if (errorCode)
    {
        CPU.isValid = 0;
        free(CPU.RAM);
        logger("CPU error", "There are some problems with init stack.");
        return;
    }
    CPU.stack.capacity = -1;
    CPU.isValid = 1;
}

static inline bool getBit(ui32 marchCode,ui8 n)
{
    return marchCode & (1 << n);
}

static inline void setBit(ui32* marchCode, ui8 n, bool value)
{
    *marchCode &= ~(1 << n);
    *marchCode |= (value << n);
}

static inline void getOperandsPointer(Command cmd, ui32** dst, ui32** src)
{
    ui32** ptrOperands[2] = { dst, src };
    OperandType opType;
    ui32* ptr = NULL;
    for (ui8 i = 0; i < cmd.nOperands; i++)
    {
        opType = getOperandType(cmd.machineCode, i);
        switch (opType)
        {
            case OPERAND_REGISTER:
                *ptrOperands[i] = getRegisterPtr(cmd.operand[i]);
                break;
            case OPERAND_NUMBER:
                *ptrOperands[i] = &cmd.operand[i];
                break;
            case OPERAND_MEMORY:
                *ptrOperands[i] = (ui32*)&CPU.RAM[CPU.Register.eds + cmd.operand[i]];
                break;
            case OPERAND_MEM_BY_REG:
                *ptrOperands[i] = (ui32*)&CPU.RAM[CPU.Register.eds + *getRegisterPtr(cmd.operand[i])];
                break;
            default:
                Assert_c(!"Invalid type of operand.");
                break;
        }
    }
       // *dst = getOperandType(cmd.machineCode, 0) == OPERAND_REGISTER ? getRegisterPtr(cmd.operand[0]) : &cmd.operand[0];
   // if(cmd.nOperands > 1)
   //     *src = getOperandType(cmd.machineCode, 1) == OPERAND_REGISTER ? getRegisterPtr(cmd.operand[1]) : &cmd.operand[1];
}

void run_MOV(Command cmd)
{
    ui32* dst = NULL;
    ui32* src = NULL;
    getOperandsPointer(cmd, &dst, &src);

    *dst = *src;
}

void run_ADD(Command cmd)
{
    ui32* dst = NULL;
    ui32* src = NULL;
    getOperandsPointer(cmd, &dst, &src);

    *dst += *src;
}

void run_SUB(Command cmd)
{
    ui32* dst = NULL;
    ui32* src = NULL;
    getOperandsPointer(cmd, &dst, &src);

    *dst -= *src;
}

void run_DIV(Command cmd)
{
    ui32* dst = NULL;
    ui32* src = NULL;
    getOperandsPointer(cmd, &dst, &src);

    if (*src == 0)
    {
        // нужно бросать исключение
    }
    else
    {
        *dst /= *src;
    }
}

void run_MUL(Command cmd)
{
    ui32* dst = NULL;
    ui32* src = NULL;
    getOperandsPointer(cmd, &dst, &src);

    *dst *= *src;
}

void run_POP(Command cmd)
{
    ui32* dst = NULL;
    ui32* src = NULL;
    getOperandsPointer(cmd, &dst, &src);
    ui8* data = (ui8*)dst;
    for (ui8 i = 0; i < sizeof(ui32); i++)
        stackPop(&CPU.stack, &data[sizeof(ui32)-1-i]);
    CPU.Register.esp -= sizeof(ui32);
}

void run_PUSH(Command cmd)
{
    ui32* dst = NULL;
    ui32* src = NULL;
    getOperandsPointer(cmd, &dst, &src);
    ui8* data = (ui8*)dst;
    for (ui8 i = 0; i < sizeof(ui32); i++)
        stackPush(&CPU.stack, &data[i]);
    CPU.Register.esp += sizeof(ui32);
}

void run_JMP(Command cmd)
{
    CPU.Register.eip = CPU.Register.ecs + (int)cmd.operand[0];
}

void run_CMP(Command cmd)
{
    ui32* dst = NULL;
    ui32* src = NULL;
    getOperandsPointer(cmd, &dst, &src);

    ui32 result = *dst - *src;

    setBit(&CPU.Register.eflags, FLAG_CF, result >> (sizeof(ui32) * 8 - 1)   );
    setBit(&CPU.Register.eflags, FLAG_ZF, result == 0 ? 1 : 0                );
    setBit(&CPU.Register.eflags, FLAG_SF, (int)result >= 0 ? 0 : 1           );

}

void run_JE(Command cmd)
{
    if(getBit(CPU.Register.eflags,FLAG_ZF))
        CPU.Register.eip = CPU.Register.ecs + (int)cmd.operand[0];
}

void run_JNE(Command cmd)
{
    if (!getBit(CPU.Register.eflags, FLAG_ZF))
        CPU.Register.eip = CPU.Register.ecs + (int)cmd.operand[0];
}

void run_JA(Command cmd)
{
    if (!getBit(CPU.Register.eflags, FLAG_CF) && !getBit(CPU.Register.eflags, FLAG_ZF))
        CPU.Register.eip = CPU.Register.ecs + (int)cmd.operand[0];
}

void run_JAE(Command cmd)
{
    if (!getBit(CPU.Register.eflags, FLAG_CF))
        CPU.Register.eip = CPU.Register.ecs + (int)cmd.operand[0];
}

void run_JB(Command cmd)
{
    if (getBit(CPU.Register.eflags, FLAG_CF))
        CPU.Register.eip = CPU.Register.ecs + (int)cmd.operand[0];
}

void run_JBE(Command cmd)
{
    if (getBit(CPU.Register.eflags, FLAG_CF) || getBit(CPU.Register.eflags, FLAG_ZF))
        CPU.Register.eip = CPU.Register.ecs + (int)cmd.operand[0];
}

void run_CALL(Command cmd)
{
    ui32* dst = NULL;
    ui32* src = NULL;
    getOperandsPointer(cmd, &dst, &src);

    ui8* data = (ui8*)&CPU.Register.eip;
    for (ui8 i = 0; i < sizeof(ui32); i++)
        stackPush(&CPU.stack, &data[i]);
    CPU.Register.esp += sizeof(ui32);
    CPU.Register.eip = CPU.Register.ecs + (int)cmd.operand[0];
}

void run_RET(Command cmd)
{
    ui32 ptrReturn = 0;

    ui8* data = (ui8*)&ptrReturn;
    for (ui8 i = 0; i < sizeof(ui32); i++)
        stackPop(&CPU.stack, &data[sizeof(ui32) - 1 - i]);
    CPU.Register.esp -= sizeof(ui32);
    CPU.Register.eip = ptrReturn;
}

void (*runFunction[])(Command)=
{
    run_MOV,
    run_ADD,
    run_SUB,
    run_DIV,
    run_MUL,
    run_POP,
    run_PUSH,
    run_JMP,
    NULL,
    run_CMP,
    run_JE,
    run_JNE,
    run_JA,
    run_JAE,
    run_JB,
    run_JBE,
    run_CALL,
    run_RET
};

const ui32 FUNCTION_TABLE_SIZE = sizeof(runFunction) / sizeof(runFunction[0]);

void cpuDump(FILE* outStream)
{
    fprintf(outStream, "CPU{\n");
    fprintf(outStream, "    Registers{\n");
    fprintf(outStream, "        eax:0x%X\n", CPU.Register.eax);
    fprintf(outStream, "        ebx:0x%X\n", CPU.Register.ebx);
    fprintf(outStream, "        ecx:0x%X\n", CPU.Register.ecx);
    fprintf(outStream, "        edx:0x%X\n", CPU.Register.edx);
    fprintf(outStream, "        esi:0x%X\n", CPU.Register.esi);
    fprintf(outStream, "        edi:0x%X\n", CPU.Register.edi);
    fprintf(outStream, "        ebp:0x%X\n", CPU.Register.ebp);
    fprintf(outStream, "        eip:0x%X\n", CPU.Register.eip);
    fprintf(outStream, "        efl:0x%X\n", CPU.Register.eflags);
    fprintf(outStream, "        ecs:0x%X\n", CPU.Register.ecs);
    fprintf(outStream, "        eds:0x%X\n", CPU.Register.eds);
    fprintf(outStream, "        ess:0x%X\n", CPU.Register.ess);
    fprintf(outStream, "    }\n");
    fprintf(outStream, "    RAM:\n");
    fprintf(outStream, "    Segment offset |  0x00  0x01  0x02  0x03  0x04  0x05  0x06  0x07  0x08  0x09  0x0A  0x0B  0x0C  0x0D  0x0E  0x0F\n");
    fprintf(outStream, "    ----------------------------------------------------------------------------------------------------------------\n");
    for (int i = 0; i < CPU_RAM_SIZE; i+=16)
    {
        fprintf(outStream, "    Segment:0x%05X|",i);
        for (int j = 0; j < 16; j++)
        {
            ui8 data = i + j < CPU_RAM_SIZE ? CPU.RAM[i + j] : 0;
            fprintf(outStream, "  0x%02X", data & 0xFF);
        }
        fprintf(outStream, "\n");   
    }
    fprintf(outStream, "}\n");

}

static void cpuRun()
{
    ui8* ptr = &CPU.RAM[CPU.Register.eip];
    Command cmd;
    while (*((Mcode*)ptr) != ASM_HLT)
    {
        ptr = &CPU.RAM[CPU.Register.eip];
        cmd.machineCode = *((Mcode*)ptr);
        ptr += sizeof(Mcode);
        CPU.Register.eip += sizeof(Mcode);
        cmd.nOperands = getNumberOperands(cmd.machineCode);
        
        for (int index = 0; index < cmd.nOperands; index++)
        {
            OperandType opType = getOperandType(cmd.machineCode, index);
            if (opType == OPERAND_REGISTER || opType == OPERAND_MEM_BY_REG)
            {
                cmd.operand[index] = *((ui8*)ptr);
                CPU.Register.eip += sizeof(ui8);
                ptr += sizeof(ui8);
            }
            if (opType == OPERAND_NUMBER || opType == OPERAND_MEMORY)
            {
                cmd.operand[index] = *((ui32*)ptr);
                CPU.Register.eip += sizeof(ui32);
                ptr += sizeof(ui32);
            }
        }

        ui32 calledFunction = (getPureMachCode(cmd.machineCode) >> 6) - 1;
        if (calledFunction >= FUNCTION_TABLE_SIZE)
        {
            logger("CPU error", "Invalid machine code of command.");
            cpuDump(getLoggerStream());
            stackDump(CPU.stack, getLoggerStream());
            return;
        }
        runFunction[calledFunction](cmd);
        ptr = &CPU.RAM[CPU.Register.eip];

        CPU.stack.data = &CPU.RAM[CPU.Register.ess];
        CPU.stack.size = CPU.Register.esp;

        disasmCommand(cmd);
        cpuDump();
        system("pause");
        //system("cls");

    }
}

void cpuRunProgram(const char* programCode,int size,ui32 ptrStart)
{
    if (!CPU.isValid)
    {
        logger("CPU error", "You try to evaluate program on broken CPU.");
        return;
    }
    memcpy(&CPU.RAM[ptrStart], programCode, size);
    CPU.RAM[ptrStart+size] = ASM_HLT; ///защита от дурака
    CPU.Register.eip = ptrStart;
    CPU.Register.ecs = ptrStart;
    CPU.Register.eds = ptrStart;
    CPU.Register.ess = ptrStart;
    CPU.Register.esp = size + 1; /// стек будет лежать за кодом

    CPU.stack.data = &CPU.RAM[CPU.Register.ess];
    CPU.stack.size = CPU.Register.esp;
    cpuRun();
    printf("Program successful complete!\n");
}