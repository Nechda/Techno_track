#include "Tester.h"
#include "Asm.h"
#include "CPU.h"
#include <math.h>

char* programMachineCode = 0;
unsigned programSize = 0;

/*
\brief    Функция выполняет подготовку к выполнению теста
\details  Функция считывает файл с исходным кодом программы теста,
          компилирует её, а результат компиляции записывается в глобальные переменные
          programMachineCode и programSize
*/
static int tets_Preparation(const char* testProgram)
{
    const char* binFile = "Tests//prog.bin";
    char* srcCode = NULL;
    unsigned inputFileSize = 0;
    int errorCode = 0;

    errorCode = readFullFile(testProgram, &srcCode, &inputFileSize);
    if (errorCode)
    {
        printf("Error: We have some troubles with read code from file:%s\n", testProgram);
        return -1;
    }


    FILE* outStream = fopen(binFile, "wb");
    if (!outStream)
    {
        printf("Error: We can't open file %s for writing result.\n", binFile);
        return -1;
    }
    if (ferror(outStream))
    {
        printf("Error: Stream for writing result has error.\n");
        return -1;
    }

    errorCode = compile(srcCode, outStream);
    fclose(outStream);
    free(srcCode);

    if (errorCode)
    {
        printf("Compilation finished with the code: %d (%s)\n", errorCode, getStringByErrorCode((AsmError)errorCode));
        return -1;
    }

    errorCode = readFullFile(binFile, &programMachineCode, &programSize);
    if (errorCode)
    {
        printf("Error: We have some troubles with read code from file:%s\n", testProgram);
        return -1;
    }
    return 0;
}

/*
\brief   Набор функций, реализующий тест программы, считающей факториал числа
@{
*/
static ui32 faclortial_CheckFinction(ui32 n) ///< Просто вычисление факториала числа
{
    ui32 res = 1;
    for (; n; n--)
        res *= n;
    return res;
}

static void test_Faclorial() ///< Тестируем процессор на программе, которая считает факториал
{
    const char* srcFile = "Tests//FACTORIAL.txt";
    if (tets_Preparation(srcFile))
        return;

    printf("Start factorial test\n");

    GeneralReg reg;
    CPUerror errorCode;
    for (int i = 1; i < 15; i++)
    {
        reg = {0, 0, (ui32)i, 0};
        setCpuRegisters(reg);
        errorCode = cpuRunProgram(programMachineCode, programSize);
        if(errorCode != CPU_OK)
            printf("CPU finished with the code: %d (%s)\n", errorCode, getStringByErrorCode(errorCode));
        getCpuRegisters(&reg);
        ui32 answ = faclortial_CheckFinction(i);
        if (reg.eax != answ)
            printf("Test[%d]:Failed --- Program calculate factorial %d! = %d, but should be: %d\n",i, i, reg.eax, answ);
        else
            printf("Test[%d]:Ok\n",i);
    }
    if(programMachineCode)
        free(programMachineCode);
    programMachineCode = NULL;
}
/**}@*/


/*
\brief   Набор функций, реализующий тест программы, считающей факториал числа
@{
*/
static ui32 fibonacci_CheckFinction(ui32 n) ///< Просто вычисление n-того числа Фибоначчи
{
    ui32 a, b;
    a = 0;
    b = 1;
    for (; n; n--)
    {
        a += b;
        a ^= b;
        b ^= a;
        a ^= b;
    }
    return a;
}

static void test_Fibonacci() ///< Тестируем процессор на программе, которая считает числа Фибоначчи
{
    const char* srcFile = "Tests//FIB.txt";
    if (tets_Preparation(srcFile))
        return;

    printf("Start fibonacci test\n");

    GeneralReg reg;
    CPUerror errorCode;
    for (int i = 1; i < 15; i++)
    {
        reg = { 0, 0, (ui32)i, 0 };
        setCpuRegisters(reg);
        errorCode = cpuRunProgram(programMachineCode, programSize);
        if (errorCode != CPU_OK)
            printf("CPU finished with the code: %d (%s)\n", errorCode, getStringByErrorCode(errorCode));
        getCpuRegisters(&reg);
        ui32 answ = fibonacci_CheckFinction(i);
        if (reg.eax != answ)
            printf("Test[%d]:Failed --- Program calculate fib_{%d} = %d, but should be: %d\n", i, i, reg.eax, answ);
        else
            printf("Test[%d]:Ok\n", i);
    }
    if (programMachineCode)
        free(programMachineCode);
    programMachineCode = NULL;
}
/**}@*/

/*
\brief  Функция запускает тесты для CPU
*/
void cpuStartTests()
{
    cupInit();
    test_Faclorial();
    test_Fibonacci();
}