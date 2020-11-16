#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <chrono>
#define _USE_MATH_DEFINES
#include <math.h>

#include <GL\freeglut.h>

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
const Mcode ASM_HLT = 0 << 6 | 0 << 4 | 0 << 2 | 0x0; ///< Именно эта команда будет завершать работу процессора

/*
\brief Номера битов в регистре eflag, отвечающие различным флагам
*/
const ui8 FLAG_CF = 0;
const ui8 FLAG_ZF = 6;
const ui8 FLAG_SF = 7;
/*
}@
*/


/**
\brief Константы, задающие режим работы с графикой
*/

const ui32 VIDEO_MEMORY_PTR = 0x400;    ///< именно в сюда нужно будет писать данные в память, чтобы можно было что-то выводить на экран
const ui16 STANDART_FONT_SIZE = 105;    ///< Размер шрифта в юнитах, данная константа получена через glutWidth(...)

void drawFromVideoMemory();///< Функция, которая будет рисовать на экран все, что располагается в видео буффере



/*
\brief Описание структуры процессора
*/
struct CPUStruct
{
    bool isValid = 0;
    bool isFloatPointMath = 0;
    int  interruptCode = 0;
    bool stepByStep = 0;
    bool isGraphMode = 0;
    bool isVideoMemoryChanged = 0;
    struct
    {
        ui32 x = 0;
        ui32 y = 0;
    }ChangedPixel;
    ui32 ramSize = 512;
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

/*
\brief Описание структуры консольного окна
*/
struct Window
{
    ui16 nCols = 80;
    ui16 nLines = 25;
    ui16 fontWidth = 8;
    ui16 fontHeight = 8;
    //эти два значения будут исползоваться для вывода шрифтов на экран
    float ratioX;
    float ratioY;
    ui16 winWidth;
    ui16 winHeight;
}window;

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
void cupInit(const InputParams inParam)
{
    CPU.isGraphMode = inParam.useGraphMode;
    CPU.ramSize = inParam.memorySize;

    if (CPU.ramSize > 1024 * 512)
    {
        CPU.isValid = 0;
        logger("CPU error", "I'm not sure that you really want too much memory: %d.", CPU.ramSize);
        return;
    }
    CPU.ramSize = CPU.ramSize > 512 ? CPU.ramSize : 512;
    CPU.RAM = (ui8*)calloc(CPU.ramSize, sizeof(ui8));
    memset(CPU.RAM, 0, CPU.ramSize * sizeof(ui8));
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


    if (CPU.isGraphMode)
    {
        ///тут важно, что данные структуры имеют одинаковое расположение полей
        memcpy(&window, &inParam.Window, sizeof(inParam.Window));
        window.winWidth = window.nCols * window.fontWidth;
        window.winHeight = window.nLines * window.fontHeight;

        window.ratioX = (float)window.fontWidth  / STANDART_FONT_SIZE;
        window.ratioY = (float)window.fontHeight / STANDART_FONT_SIZE;

        glutInitWindowSize(window.winWidth, window.winHeight);
        glutInitWindowPosition(0, 0);
        glutCreateWindow("Screen");
        glutDisplayFunc(drawFromVideoMemory);

        glViewport(0, 0, window.winWidth, window.winHeight);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluOrtho2D(0, window.winWidth, 0, window.winHeight);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
    }
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
    printRegInfo(eax);printRegInfo(ebx);printRegInfo(ecx);printRegInfo(edx);
    printRegInfo(esi);printRegInfo(edi);printRegInfo(ebp);printRegInfo(eip);
    printRegInfo(efl);printRegInfo(ecs);printRegInfo(eds);printRegInfo(ess);
    #undef printRegInfo
    fprintf(outStream, "    }\n");
   
    #ifdef DUMP_PRINT_MEMORY
    fprintf(outStream, "    RAM:\n");
    fprintf(outStream, "    Segment offset |  0x00  0x01  0x02  0x03  0x04  0x05  0x06  0x07  0x08  0x09  0x0A  0x0B  0x0C  0x0D  0x0E  0x0F\n");
    fprintf(outStream, "    ----------------------------------------------------------------------------------------------------------------\n");
    for (int i = 0; i < CPU.ramSize ; i += 16)
    {
        fprintf(outStream, "    Segment:0x%05X|", i);
        for (int j = 0; j < 16; j++)
        {
            ui8 data = i + j < CPU.ramSize ? CPU.RAM[i + j] : 0;
            fprintf(outStream, "  0x%02X", data & 0xFF);
        }
        fprintf(outStream, "\n");
    }
    #endif
    fprintf(outStream, "}\n");

    #ifdef DUMP_PRINT_STACK
    stackDump(CPU.stack, getLoggerStream());
    #endif
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
    ui32 offset = 0;
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
            offset = CPU.Register.eds + cmd.operand[i];
            if (offset+sizeof(ui32) >= CPU.ramSize)
            {
                Assert_c(!"The command tries to access a nonexistent memory area!");
                CPU.interruptCode = 3;
                break;
            }
            if (CPU.isGraphMode && offset >= VIDEO_MEMORY_PTR && offset <= VIDEO_MEMORY_PTR + sizeof(ui8) * window.nCols * window.nLines)
            {
                CPU.isVideoMemoryChanged = 1;
                CPU.ChangedPixel.x = offset - VIDEO_MEMORY_PTR;
                CPU.ChangedPixel.y = CPU.ChangedPixel.x;
                CPU.ChangedPixel.x %= window.nCols;
                CPU.ChangedPixel.y /= window.nCols;
            }
            *ptrOperands[i] = (ui32*)&CPU.RAM[offset];
            break;
        case OPERAND_MEM_BY_REG:
            offset = CPU.Register.eds + *getRegisterPtr(cmd.operand[i]);
            if (offset+sizeof(ui32)>= CPU.ramSize)
            {
                Assert_c(!"The command tries to access a nonexistent memory area!");
                CPU.interruptCode = 3;
                break;
            }
            if (CPU.isGraphMode && offset >= VIDEO_MEMORY_PTR && offset <= VIDEO_MEMORY_PTR + sizeof(ui8) * window.nCols * window.nLines)
            {
                CPU.isVideoMemoryChanged = 1;
                CPU.ChangedPixel.x = offset - VIDEO_MEMORY_PTR;
                CPU.ChangedPixel.y = CPU.ChangedPixel.x;
                CPU.ChangedPixel.x %= window.nCols;
                CPU.ChangedPixel.y /= window.nCols;
            }
            *ptrOperands[i] = (ui32*)&CPU.RAM[offset];
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
    #define DEF(name, machineCode, validStrOperand_1, validStrOperand_2, code) run_##name,
    #include "Extend.h"
    #undef DEF
};

const ui32 FUNCTION_TABLE_SIZE = sizeof(runFunction)/sizeof(FunctionType); ///< Размер таблицы функций, введенный для удобства.



/*
\brief  Функция, запускает выполнение программы, начиная с текущего значение CPU.Register.eip
\param  [in]  writeResultInLog  Флаг, отвечающий за то, хотим ли мы увидеть результат работы программы в логе
\return Возвращается код ошибки или CPU_OK
*/
static CPUerror cpuRun(bool writeResultInLog = true)
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

        if (CPU.stepByStep)
        {
            disasmCommand(cmd, stdout);
            cpuDump(stdout);
            #ifdef _WIN32
            system("pause");
            system("cls");
            #elif __linux__
            system("read -rsp $\"Press any key to continue...\n\" -n 1");
            system("clear");
            #endif

        }


        ui32 indexCalledFunc = (getPureMachCode(cmd.machineCode) >> 6);
        if (indexCalledFunc >= FUNCTION_TABLE_SIZE)
        {
            logger("CPU error", "Invalid machine code of command.");
            cpuDump(getLoggerStream());
            return CPU_ERROR_INVALID_COMMAND;
        }
        runFunction[indexCalledFunc](cmd);
        ptr = &CPU.RAM[CPU.Register.eip];

        if (CPU.interruptCode)
        {
            logger("CPU error", "Catch exception after execution command:");
            disasmCommand(cmd, getLoggerStream());
            cpuDump(getLoggerStream());
            return CPU_ERROR_EXCEPTION;
        }
        if (CPU.Register.eip >= CPU.ramSize)
        {
            logger("CPU error", "Register epi quite big for RAM.");
            cpuDump(getLoggerStream());
            return CPU_ERROR_EPI_OUT_OF_RANE;
        }
        CPU.stack.data = &CPU.RAM[CPU.Register.ess];
        CPU.stack.size = CPU.Register.esp;

        if (CPU.isGraphMode && CPU.isVideoMemoryChanged)
        {
            drawFromVideoMemory();
            CPU.isVideoMemoryChanged = 0;
        }
    }

    if (writeResultInLog)
    {
        logger("Cpu", "Program successful complete! Damped CPU:\n");
        cpuDump(getLoggerStream());
    }

    return CPU_OK;
}


/*
\brief  Функция загружает программу в RAM процессора, затем запускает её на выпонение
\param  [in]  programCode  Массив байтов, содержащий программу
\param  [in]  size         Размер загружаемой программы
\param  [in]  ptrStart     Номер ячейки, начиная с которой будет производиться копирование в RAM
\return Возвращается код ошибки или CPU_OK
*/
CPUerror cpuRunProgram(const char* programCode, int size, bool writeResultInLog, ui32 ptrStart)
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
    Assert_c(ptrStart + size + 1 < CPU.ramSize);
    if (ptrStart + size + 1 >= CPU.ramSize)
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

    /// Перед видеопамятью расположим информацию о количестве строк и столбиков
    if (CPU.isGraphMode && CPU.ramSize >= VIDEO_MEMORY_PTR)
    {
        *((ui16*)&CPU.RAM[VIDEO_MEMORY_PTR - 2 * sizeof(ui16)]) = window.nLines;
        *((ui16*)&CPU.RAM[VIDEO_MEMORY_PTR - 1 * sizeof(ui16)]) = window.nCols;
    }


    CPU.stack.data = &CPU.RAM[CPU.Register.ess];
    CPU.stack.size = CPU.Register.esp;
    CPUerror errorCode = cpuRun(writeResultInLog);
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



static inline void renderChar(float x, float y, const char c)
{
    glColor3f(0,0,0);
    glRectf(x, y, x + window.fontWidth, y + window.fontHeight);

    glColor3f(1, 1, 1);
    glPushMatrix();
    glTranslatef(x, y, 0);
    glScalef(window.ratioX, window.ratioY, 1.0);
    glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN, c);
    glPopMatrix();
}


void drawFromVideoMemory()
{
    ///если раскомментировать этот кусок, то получится красивый эффект
    /*
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0, 0, 0, 0.01);
    glRectf(0, 0, window.winWidth, window.winHeight);
    */

    
    static const char* string = (char*)&CPU.RAM[VIDEO_MEMORY_PTR];    
    int col = CPU.ChangedPixel.x;
    int line = CPU.ChangedPixel.y;
    renderChar(col * window.fontWidth, window.winHeight - (line+1) * window.fontWidth, string[col + line * window.nCols]);

    glFlush();
}
