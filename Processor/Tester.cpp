#include "Tester.h"
#include "Asm.h"
#include "CPU.h"
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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

    inputFileSize = readFullFile(testProgram, &srcCode);
    if (inputFileSize == ASM_ERROR_CODE)
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

    programSize = readFullFile(binFile, &programMachineCode);
    if (programSize == ASM_ERROR_CODE)
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
        errorCode = cpuRunProgram(programMachineCode, programSize, false);
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
        errorCode = cpuRunProgram(programMachineCode, programSize, false);
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



static int getRandomInt(int start,int end)
{
    return rand()%(end-start+1) + start;
}

static float getRandomFloat()
{
    return getRandomInt(0, 1000) / 1000.0;
}

static void test_SquareEquation()
{
    srand(time(0));
    const char* srcFile = "Tests//SQR_EQ.txt";
    if (tets_Preparation(srcFile))
        return;

    printf("Start square equation solver test\n");


    const int rangeSize = 100;
    const int nTests = 400;


    GeneralReg reg;
    CPUerror errorCode;
    float roots[2] = {};
    char nRoots = 0;
    float coefficients[3] = {};
    const float accuracy = 1E-2;
    float isTestSuccess = 1;
    for (int i = 1; i <= nTests; i++)
    {
        if (isTestSuccess)
        {
            if (i < nTests / 4)
            {
                roots[0] = getRandomInt(-rangeSize, rangeSize) + getRandomFloat();
                roots[1] = getRandomInt(-rangeSize, rangeSize) + getRandomFloat();
            }
            if (i > nTests / 4 && i < nTests / 2)
            {
                roots[0] = 0;
                roots[1] = getRandomInt(-rangeSize, rangeSize) + getRandomFloat();
            }
            if (i > nTests / 2 && i < 3* nTests / 4)
            {
                roots[0] = roots[1] = getRandomInt(-rangeSize, rangeSize) + getRandomFloat();
            }

            nRoots = 2;
            if (fabs(roots[0] - roots[1]) < accuracy)
            {
                roots[0] += roots[1];
                roots[0] /= 2;
                roots[1] = roots[0];
                nRoots = 1;
            }
            coefficients[0] = 1;
            coefficients[1] = -(roots[0] + roots[1]);
            coefficients[2] = roots[0] * roots[1];

            if (i > 3 * nTests / 4)
            {
                roots[0] = getRandomInt(-rangeSize, rangeSize) + getRandomFloat();
                coefficients[0] = 0;
                coefficients[1] = 1;
                coefficients[2] = -roots[0];
                nRoots = 1;
            }

        }
        isTestSuccess = 1;




        reg = { 0, 0, 0, 0 };
        memcpy(&reg, coefficients ,sizeof(coefficients));
        setCpuRegisters(reg);
        errorCode = cpuRunProgram(programMachineCode, programSize, false);
        if (errorCode != CPU_OK)
            printf("CPU finished with the code: %d (%s)\n", errorCode, getStringByErrorCode(errorCode));

        

        getCpuRegisters(&reg);

        float x1 = *(float*)&reg.eax;
        float x2 = *(float*)&reg.ebx;
        char nRootsCalc = (char)(*(float*)&reg.ecx);
        if (nRoots != nRootsCalc)
        {
            printf("Test[%04d]:Fail --- should be (%f,%f); calc (%f,%f); should be nRoots: %d; calc nRoots: %d\n", i, x1, x2, roots[0], roots[1], nRoots, nRootsCalc);
            isTestSuccess = 0;
        }

        float error = 0;
        if (nRoots == 2)
        {
            float error_1 = sqrt(pow(x1 - roots[0], 2) + pow(x2 - roots[1], 2));
            float error_2 = sqrt(pow(x1 - roots[1], 2) + pow(x2 - roots[0], 2));
            error = error_1 < error_2 ? error_1 : error_2;
        }
        else
        {
            error = fabs(x1 - roots[0]);
        }


        if (error < accuracy)
            printf("Test[%04d]:Ok --- ", i);
        else
        {
            printf("Test[%04d]:Fail --- ", i);
            isTestSuccess = 0;
        }
        if (nRoots == 1)
            printf("should be (%f); calc (%f); ", roots[0], x1);
        if (nRoots == 2)
            printf("should be(%f, %f); calc(%f, %f); ", roots[0], roots[1], x1, x2);
        printf("nRoots: %d; calc nRoots : %d\n", nRoots, nRootsCalc);
        if (!isTestSuccess)
        {
            printf("Run program in step by step mode...\n");
            setStepByStepMode(1);
        }
        else
            setStepByStepMode(0);
        
       
    }
    if (programMachineCode)
        free(programMachineCode);
    programMachineCode = NULL;
}



/*
\brief  Функция запускает тесты для CPU
*/
void cpuStartTests()
{
    cupInit(0);
    test_Faclorial();
    test_Fibonacci();
    test_SquareEquation();
}