#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <chrono>
#include <math.h>


#include "Asm.h"
#include "CPU.h"

#include "Stack/Stack_kernel.h"
#define TYPE_ ui8
#include "Stack/Stack.h"
#undef TYPE_


/*
\brief Константы, определяющие работу процессора
@{
*/
ui32 CPU_RAM_SIZE = 512; ///< Размер виртуальной RAM в байтах
const Mcode ASM_HLT = 9 << 6 | 0 << 4 | 0 << 2 | 0x0; ///< Именно эта команда будет завершать работу процессора

/*
\brief Номера битов в регистре eflag, отвечающие различным флагам
*/
const ui8 FLAG_CF = 0;
const ui8 FLAG_ZF = 6;
const ui8 FLAG_SF = 7;
/*
}@
*/

/*
\brief Описание структуры процессора
*/
struct
{
    bool isValid = 0;
    bool isFloatPointMath = 0;
    int  interruptCode = 0;
    bool stepByStep = 0;
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
        ui32 efl; ///< сокращение от eflags
        ui32 ecs;
        ui32 eds;
        ui32 ess;
    }Register;
    ui8* RAM = NULL;
    Stack(ui8) stack;
}CPU;

/**
\brief Устанавливает режим работы процессора step by step
*/
void setStepByStepMode(bool flag)
{
    CPU.stepByStep = flag;
}

/*
\brief  Функция возвращает указатель на поле структуры CPU.Register,
в качестве аргумента принимается нормер регистра (см таблицу с регистрами).
\param  [in]  number  Номер регистра, адрес которого кужно получить
\return Указатель на поле структуры CPU.Register, в случае ошибки возвращается NULL
*/
ui32* getRegisterPtr(ui8 number)
{
    if (number > sizeof(CPU.Register) / sizeof(ui32))
        return NULL;
    return (ui32*)&CPU.Register + (number - 1);
}

/*
\brief  Функция возвращает поясняющую строку, по коду ошибки процессора
\param  [in]  errorCode  Код ошибки
\return Возвращается строка, поясняющее код ошибки
*/
C_string getStringByErrorCode(CPUerror errorCode)
{
    switch (errorCode)
    {
    case CPU_OK:
        return "Ok";
        break;
    case CPU_ERROR_INVALID_STRUCUTE:
        return "CPU structure has been broken";
        break;
    case CPU_ERROR_INVALID_COMMAND:
        return "CPU find command that he doesn't know";
        break;
    case CPU_ERROR_EXCEPTION:
        return "An exceptional situation has occurred";
        break;
    case CPU_ERROR_EPI_OUT_OF_RANE:
        return "EPI register is too large for CPU's RAM";
        break;
    case CPU_INVALID_INPUT_DATA:
        return "The data passed is not valid";
    default:
        return "Undefined error code";
        break;
    }
}




/*
\brief  Функция производит инициализацию структуры CPU
\param  [in]  ramSize  Размер виртуальной памяти процессора
\note   Если при инициализации структуры возникли ошибки, то полю isValid присваивается значение 0
Причина возникновения ошибки записывается в лог файл.
*/
void cupInit(ui32 ramSize)
{
    if (ramSize > 1024 * 512)
    {
        CPU.isValid = 0;
        return;
    }
    CPU_RAM_SIZE = ramSize > CPU_RAM_SIZE ? ramSize : CPU_RAM_SIZE;
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
        CPU.RAM = NULL;
        logger("CPU error", "There are some problems with init stack.");
        return;
    }
    CPU.stack.capacity = -1;
    CPU.isValid = 1;
}


/*
\brief  Функция делает cleanUp структуры CPU
*/
void cpuDestr()
{
    if (CPU.RAM)
        free(CPU.RAM);
    stackDest(&CPU.stack);
    CPU.isValid = 0;
}

/*
\brief  Функции для работы с битами: set\get
@{
*/
static inline bool getBit(ui32 marchCode, ui8 n)
{
    return marchCode & (1 << n);
}

static inline void setBit(ui32* marchCode, ui8 n, bool value)
{
    *marchCode &= ~(1 << n);
    *marchCode |= (value << n);
}
/*
}@
*/


/*
\brief  Функция вычисления указателей на память, с которой будет работать команда
\param  [in]  cmd   команда
\param  [in,out]  dst  указатель на область памяти, отвечающее первому операнду в команде
\param  [in,out]  src  указатель на область памяти, отвечающее второму операнду в команде
\note   В случае ошибки будет брошен Assert_c.
Пример работы: если команда имела вид mov eax,ebx, то в dst и src будут равны:
dst = &CPU.Register.eax, src = &CPU.Register.ebx
*/
static inline void getOperandsPointer(Command cmd, ui32** dst, ui32** src)
{
    ui32** ptrOperands[2] = { dst, src };
    OperandType opType;
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
}

/*
\brief  Функция проверяет является ли число нулем с заданной точностью
\param  [in]  num       Исследуемое число
\param  [in]  accuracy  Точность сравнения
\return true, если число близко к нулю и false в противном случае.
*/
static inline bool isZero(float num, float accuracy = 1E-7)
{
    return fabs(num) < accuracy ? 1 : 0;
}

/*
\brief Набор функций, реализующие действие команд над процессором
@{*/
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
    if (!CPU.isFloatPointMath)
        *dst += *src;
    else
        *((float*)(dst)) += *((float*)(src));
}

void run_SUB(Command cmd)
{
    ui32* dst = NULL;
    ui32* src = NULL;
    getOperandsPointer(cmd, &dst, &src);

    if (!CPU.isFloatPointMath)
        *dst -= *src;
    else
        *((float*)(dst)) -= *((float*)(src));
}

void run_DIV(Command cmd)
{
    ui32* dst = NULL;
    ui32* src = NULL;
    getOperandsPointer(cmd, &dst, &src);



    if (!CPU.isFloatPointMath)
    {
        if (*src == 0)
            CPU.interruptCode = 1; // при делении на ноль, возникает прерывание
        else
            *dst /= *src;
    }
    else
    {
        float divisitor = *((float*)src);
        if (isZero(divisitor))
            CPU.interruptCode = 1; // при делении на ноль, возникает прерывание
        else
            *((float*)dst) /= divisitor;
    }
}

void run_MUL(Command cmd)
{
    ui32* dst = NULL;
    ui32* src = NULL;
    getOperandsPointer(cmd, &dst, &src);

    if (!CPU.isFloatPointMath)
        *dst *= *src;
    else
    {
        float mul = *((float*)src);
        *((float*)(dst)) *= mul;
    }
}

void run_POP(Command cmd)
{
    ui32* dst = NULL;
    ui32* src = NULL;
    getOperandsPointer(cmd, &dst, &src);
    ui8* data = (ui8*)dst;
    for (ui8 i = 0; i < sizeof(ui32); i++)
        stackPop(&CPU.stack, &data[sizeof(ui32) - 1 - i]);

    //в данной реализации процессора в качестве стека используется immortal stack,
    //соттветственно при push он растет в сторону больших адресов,
    //а при pop адрес вершины уменьшается
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

    //в данной реализации процессора в качестве стека используется immortal stack,
    //соттветственно при push он растет в сторону больших адресов,
    //а при pop адрес вершины уменьшается
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

    if (!CPU.isFloatPointMath)
    {
        ui32 result = *dst - *src;

        setBit(&CPU.Register.efl, FLAG_CF, result >> (sizeof(ui32) * 8 - 1));
        setBit(&CPU.Register.efl, FLAG_ZF, result == 0 ? 1 : 0);
        setBit(&CPU.Register.efl, FLAG_SF, (int)result >= 0 ? 0 : 1);
    }
    else
    {
        float result = *(float*)dst - *(float*)src;

        setBit(&CPU.Register.efl, FLAG_CF, (int)result >> (sizeof(ui32) * 8 - 1));
        setBit(&CPU.Register.efl, FLAG_ZF, isZero(result));
        setBit(&CPU.Register.efl, FLAG_SF, result >= 0 ? 0 : 1);
    }

}

void run_JE(Command cmd)
{
    if (getBit(CPU.Register.efl, FLAG_ZF))
        CPU.Register.eip = CPU.Register.ecs + (int)cmd.operand[0];
}

void run_JNE(Command cmd)
{
    if (!getBit(CPU.Register.efl, FLAG_ZF))
        CPU.Register.eip = CPU.Register.ecs + (int)cmd.operand[0];
}

void run_JA(Command cmd)
{
    if (!getBit(CPU.Register.efl, FLAG_CF) && !getBit(CPU.Register.efl, FLAG_ZF))
        CPU.Register.eip = CPU.Register.ecs + (int)cmd.operand[0];
}

void run_JAE(Command cmd)
{
    if (!getBit(CPU.Register.efl, FLAG_CF))
        CPU.Register.eip = CPU.Register.ecs + (int)cmd.operand[0];
}

void run_JB(Command cmd)
{
    if (getBit(CPU.Register.efl, FLAG_CF))
        CPU.Register.eip = CPU.Register.ecs + (int)cmd.operand[0];
}

void run_JBE(Command cmd)
{
    if (getBit(CPU.Register.efl, FLAG_CF) || getBit(CPU.Register.efl, FLAG_ZF))
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
/*}@*/


/*
\brief  Объвление дополнительных функций
*/
#define DEF(name, machineCode, validStrOperand_1, validStrOperand_2, code) \
void run_##name(Command cmd) code
#include "Extend.h"
#undef DEF


/*
\breif Массив функций, реализующих поведение процессора
*/
typedef void(*FunctionType)(Command);

FunctionType runFunction[] =
{
    run_MOV,  run_ADD, run_SUB,  run_DIV,
    run_MUL,  run_POP, run_PUSH, run_JMP,
    NULL,     run_CMP, run_JE,   run_JNE,
    run_JA,   run_JAE, run_JB,   run_JBE,
    run_CALL, run_RET,
    #define DEF(name, machineCode, validStrOperand_1, validStrOperand_2, code) run_##name,
    #include "Extend.h"
    #undef DEF
};

const ui32 FUNCTION_TABLE_SIZE = sizeof(runFunction)/sizeof(FunctionType); ///< Размер таблицы функций, введенный для удобства.


/*
\brief Функция, производящая дамп процессора, результат закидывается в outStream
\param [in] outStream указатель на поток вывода
*/
void cpuDump(FILE* outStream)
{
    Assert_c(outStream);
    if (!outStream)
        return;
    fprintf(outStream, "CPU{\n");
    fprintf(outStream, "    Registers{\n");
    #define printRegInfo(regName)\
    fprintf(outStream, "        " #regName ":0x%04X  (int: %d) \t(float: %f)\n", CPU.Register.##regName,CPU.Register.##regName,*((float*)&CPU.Register.##regName))
    printRegInfo(eax);
    printRegInfo(ebx);
    printRegInfo(ecx);
    printRegInfo(edx);
    printRegInfo(esi);
    printRegInfo(edi);
    printRegInfo(ebp);
    printRegInfo(eip);
    printRegInfo(efl);
    printRegInfo(ecs);
    printRegInfo(eds);
    printRegInfo(ess);
    #undef printRegInfo
    fprintf(outStream, "    }\n");
    fprintf(outStream, "    RAM:\n");
    fprintf(outStream, "    Segment offset |  0x00  0x01  0x02  0x03  0x04  0x05  0x06  0x07  0x08  0x09  0x0A  0x0B  0x0C  0x0D  0x0E  0x0F\n");
    fprintf(outStream, "    ----------------------------------------------------------------------------------------------------------------\n");
    for (int i = 0; i < CPU_RAM_SIZE; i += 16)
    {
        fprintf(outStream, "    Segment:0x%05X|", i);
        for (int j = 0; j < 16; j++)
        {
            ui8 data = i + j < CPU_RAM_SIZE ? CPU.RAM[i + j] : 0;
            fprintf(outStream, "  0x%02X", data & 0xFF);
        }
        fprintf(outStream, "\n");
    }
    fprintf(outStream, "}\n");
}


/*
\brief  Функция, запускает выполнение программы, начиная с текущего значение CPU.Register.eip
\return Возвращается код ошибки или CPU_OK
*/
static CPUerror cpuRun()
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

        ui32 indexCalledFunc = (getPureMachCode(cmd.machineCode) >> 6) - 1;
        if (indexCalledFunc >= FUNCTION_TABLE_SIZE)
        {
            logger("CPU error", "Invalid machine code of command.");
            cpuDump(getLoggerStream());
            stackDump(CPU.stack, getLoggerStream());
            return CPU_ERROR_INVALID_COMMAND;
        }
        runFunction[indexCalledFunc](cmd);
        ptr = &CPU.RAM[CPU.Register.eip];

        if (CPU.interruptCode)
        {
            logger("CPU error", "Catch exception after execution command:");
            disasmCommand(cmd, getLoggerStream());
            cpuDump(getLoggerStream());
            stackDump(CPU.stack, getLoggerStream());
            return CPU_ERROR_EXCEPTION;
        }
        if (CPU.Register.eip >= CPU_RAM_SIZE)
        {
            logger("CPU error", "Register epi quite big for RAM.");
            cpuDump(getLoggerStream());
            stackDump(CPU.stack, getLoggerStream());
            return CPU_ERROR_EPI_OUT_OF_RANE;
        }
        CPU.stack.data = &CPU.RAM[CPU.Register.ess];
        CPU.stack.size = CPU.Register.esp;

        if (CPU.stepByStep)
        {
            disasmCommand(cmd);
            cpuDump(stdout);
            system("pause");
            system("cls");
        }


    }

    logger("Cpu", "Program successful complete! Damped CPU:\n");
    cpuDump(getLoggerStream());
    stackDump(CPU.stack, getLoggerStream());

    return CPU_OK;
}


/*
\brief  Функция загружает программу в RAM процессора, затем запускает её на выпонение
\param  [in]  programCode  Массив байтов, содержащий программу
\param  [in]  size         Размер загружаемой программы
\param  [in]  ptrStart     Номер ячейки, начиная с которой будет производиться копирование в RAM
\return Возвращается код ошибки или CPU_OK
*/
CPUerror cpuRunProgram(const char* programCode, int size, ui32 ptrStart)
{
    if (!CPU.isValid)
    {
        logger("CPU error", "You try to evaluate program on broken CPU.");
        return CPU_ERROR_INVALID_STRUCUTE;
    }
    Assert_c(programCode);
    if (!programCode)
    {
        logger("CPU error", "You try execute program, located by NULL pointer.");
        return CPU_INVALID_INPUT_DATA;
    }
    Assert_c(size > 0);
    if (size <= 0)
    {
        logger("CPU error", "You try execute program, that have incorrect size:%d", size);
        return CPU_INVALID_INPUT_DATA;
    }
    Assert_c(ptrStart + size + 1 < CPU_RAM_SIZE);
    if (ptrStart + size + 1 >= CPU_RAM_SIZE)
    {
        logger("CPU error", "Your program doesn't fit in RAM. Try to change ptrStart or write small program");
        return CPU_INVALID_INPUT_DATA;
    }

    memcpy(&CPU.RAM[ptrStart], programCode, size);
    *((Mcode*)&CPU.RAM[ptrStart + size]) = ASM_HLT; ///на всякий случай поставим код остановки, после всей программы
    CPU.Register.eip = ptrStart;
    CPU.Register.ecs = ptrStart;
    CPU.Register.eds = ptrStart;
    CPU.Register.ess = ptrStart;
    CPU.Register.esp = size + 1; /// стек будет лежать за кодом

    CPU.stack.data = &CPU.RAM[CPU.Register.ess];
    CPU.stack.size = CPU.Register.esp;
    CPUerror errorCode = cpuRun();
    return errorCode;
}



/**
\brief  Функция установки регистров общего назначения процессора
\param  [in]  reg  Структура с регистрами общего назначения
*/
void setCpuRegisters(GeneralReg reg)
{
    memcpy(&CPU.Register, &reg, sizeof(GeneralReg));
}

/**
\brief  Функция считывания регистров общего назначения процессора
\param  [in,out]  reg  Указатель на структуру с регистрами общего назначения
*/
void getCpuRegisters(GeneralReg* reg)
{
    memcpy(reg, &CPU.Register, sizeof(GeneralReg));
}
