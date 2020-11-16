#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include "Argparser.h"
#include "Asm.h"
#include "CPU.h"
#include "Logger.h"
#include "Tester.h"
#include "CallStack.h"

#include <GL\freeglut.h>




/**
\brief Константы, описывающие коды ошибок, возникающие в main
*/
const ui8 ERROR_INIT_INPUT_STREAM           = 1 << 0;
const ui8 ERROR_INIT_OUTPUT_STREAM          = 1 << 1;
const ui8 ERROR_JUMP_DEFAULT_CASE_IN_SWITCH = 1 << 2;

/**
\brief Функция запускает вывод справки по программе
*/
void run_Helper()
{
    $
        printf(
            "There are some flags you can run program with, here is their description:\n"
            "-h                  Shows help to the program and describes all flags that you can use.\n"
            "-c                  Calls compilator, you also need use -i flag\n"
            "-d                  Calls disassembler, you also need use -i flag\n"
            "-r                  Calls virtual CPU to execute your program, you also need use -i flag\n"
            "-e                  This is a mixture of two programs:it first calls compilator, then virtual CPU\n"
            "you also need use -i flag to select file with a program code. There is -o flag, but it's not\n"
            "necessary: by default result of compilation will save into \"a.out\" file.\n"
            "-t                  Calls test programs, that located in Tests folder.\n"
            "-l <filename>       This flag sets the log file, by default log file has name \"log.log\".\n"
            "-i <filename>       This flag sets the input file, from this file program will read infromation.\n"
            "-o <filename>       This flag sets the output file, into this file program will write result,\n"
            "by default file has name \"a.out\" file.\n"
            "-n                  This flag disable writing in log file.\n"
            "-s                  Sets the processor's program execution mode to step-by-step.\n"
            "--memorySize=<size> Sets size of virtual RAM in bytes.(Actual RAM size will be the maximum between 512 and your size)\n"
            "-g                  Enable graphic mode, you will be able to write into video memory, located at 0x400 offset in\n"
            "virtual memory.\n"
            "--config=<filename> Specifies the configuration file, in there you can describe window and font sizes in graphic mode,\n"
            "init entry point adress for your programm also set RAM size\n"
        );
    $$
}

/**
\brief Функция запускает компилятор
*/
int run_Compilator(char* buffer, FILE* outStream, const InputParams inputParams)
{$
    int errorCode = 0;
    errorCode = compile(buffer, outStream);
    printf("Compilation finished with the code: %d (%s)\n", errorCode, getStringByErrorCode(static_cast<AsmError>(errorCode)));
    if (!inputParams.noLogFileFlag && errorCode)
        printf("More infromation see in log file: %s\n", inputParams.logFilename);
    $$ return errorCode;
}

/**
\brief Функция запускает дизасемблер
*/
int run_Disassembler(char* buffer,unsigned nBytes, FILE* outStream, const InputParams inputParams)
{$
    int errorCode = 0;
    errorCode = disasm((ui8*)buffer, nBytes, outStream);
    printf("Disassembler finished with the code: %d (%s)\n", errorCode, getStringByErrorCode(static_cast<AsmError>(errorCode)));
    if (!inputParams.noLogFileFlag && errorCode)
        printf("More infromation see in log file: %s\n", inputParams.logFilename);
    $$ return errorCode;
}

/**
\brief Функция запускает эмулятор процессора
*/
int run_CPU(char* buffer, unsigned nBytes, const InputParams inputParams)
{$
    cupInit(inputParams);

    int errorCode = 0;
    errorCode = cpuRunProgram(buffer, nBytes, 1, inputParams.entryPoint);
    printf("CPU finished with the code: %d (%s)\n", errorCode, getStringByErrorCode(static_cast<CPUerror>(errorCode)));
    if (!inputParams.noLogFileFlag && errorCode)
        printf("More infromation see in log file: %s\n", inputParams.logFilename);
    cpuDestr();
    $$ return errorCode;
}

/**
\brief Функция инициализирует поток ввода, основываясь на данных из InputParams
*/
int initInStream(char* inputFileName,char** buffer, unsigned* readBytes, const InputParams inputParams)
{$
    Assert_c(inputFileName);
    Assert_c(buffer);
    Assert_c(readBytes);
    if (!buffer || !readBytes)
    {
        $$$("Null ptr in input data");
        return ERROR_INIT_INPUT_STREAM;
    }

    if (!inputFileName)
    {
        printf("Error: No input file specified!\n");
        $$$("Null ptr in input data");
        return ERROR_INIT_INPUT_STREAM;
    }

    *readBytes = readFullFile(inputFileName, buffer);
    if (*readBytes == ASM_ERROR_CODE)
    {
        printf("Error: We have some troubles with read code from file:%s\n", inputFileName);
        
        $$$("Can't read file");
        return ERROR_INIT_INPUT_STREAM;
    }
    
    $$ return 0;
}

/**
\brief Функция инициализирует поток вывода, основываясь на данных из InputParams
*/
int initOutStream(char* outputFileName, FILE** outStreamPtr, char* mode,InputParams inputParams)
{$
    Assert_c(outStreamPtr);
    Assert_c(mode);
    if (!outStreamPtr || !mode)
    {
        $$$("Null ptr in input data");
        return ERROR_INIT_OUTPUT_STREAM;
    }

    if (!outputFileName)
    {
        printf("Warning: No output file specified. By default result will write into a.out\n");
        outputFileName = "a.out";
        inputParams.outputFilename = "a.out";
    }

    *outStreamPtr = fopen(outputFileName, mode);
    if (!*outStreamPtr)
    {
        printf("Error: We can't open file %s for writing result.\n", outputFileName);
        $$$("Can't read file");
        return ERROR_INIT_OUTPUT_STREAM;
    }
    if (ferror(*outStreamPtr))
    {
        printf("Error: Stream for writing result has error.\n");
        $$$("Error in stream");
        return ERROR_INIT_OUTPUT_STREAM;
    }
    $$ return 0;
}




int main(int argc, char** argv)
{
    InputParams inputParams;
    initCallStack();
    $
    parseConsoleArguments(argc, argv, &inputParams);
    loggerInit(inputParams.noLogFileFlag ? NULL : inputParams.logFilename, "w");

    char* buffer = NULL;
    int errorCode = 0;
    unsigned inputFileSize = 0;
    FILE* outStream = NULL;
    
    //Смотрим какая программа была выбрана
    switch (inputParams.programName)
    {
        case PROG_COPMILATOR:
        case PROG_DISASSEMBLER:
        case PROG_RUN:
            errorCode |= initOutStream(inputParams.outputFilename, &outStream, inputParams.programName == PROG_DISASSEMBLER ? "w" : "wb", inputParams);
        case PROG_CPU:
            errorCode |= initInStream(inputParams.inputFilename, &buffer, &inputFileSize, inputParams);
            break;
        case PROG_UNDEFINED:
            printf("We don't understand what you want to execute ...\nTry to use -h flag to get more information.");
            loggerDestr();
            $$ return 0;
            break;
        case PROG_HELPER:
            run_Helper();
            loggerDestr();
            $$ return 0;
            break;
        case PROG_TESTER:
            prepareCompilatorsTable();
            cpuStartTests();
            loggerDestr();
            $$ return 0;
        default:
            errorCode |= ERROR_JUMP_DEFAULT_CASE_IN_SWITCH;
            break;
    }
    if (errorCode)
    {
        printf("Error: We have problems with %s stream\n", errorCode & 1 ? "input" : "output");
        $$$("One stream need repair");
        loggerDestr();
        return errorCode;
    }

    //Вспомогательная инициализация
    if (inputParams.useGraphMode)
    {
        glutInit(&argc, argv);
        glutInitDisplayMode(GLUT_SINGLE);
    }
    if (inputParams.useStepByStepMode)
        setStepByStepMode(1);

    //Производим выполнение программы
    prepareCompilatorsTable();
    switch (inputParams.programName)
    {
        case PROG_COPMILATOR:
            errorCode |= run_Compilator(buffer, outStream, inputParams) << 4;
            break;
        case PROG_DISASSEMBLER:
            errorCode |= run_Disassembler(buffer, inputFileSize, outStream, inputParams) << 4;
            break;
        case PROG_RUN:
            errorCode |= run_Compilator(buffer, outStream, inputParams) << 4;
            free(buffer);
            fclose(outStream);
            errorCode |= initInStream(inputParams.outputFilename, &buffer, &inputFileSize, inputParams);
            errorCode |= run_CPU(buffer, inputFileSize, inputParams) << 4;
            break;
        case PROG_CPU:
            errorCode |= run_CPU(buffer, inputFileSize, inputParams) << 4;
            break;
        default:
            errorCode |= ERROR_JUMP_DEFAULT_CASE_IN_SWITCH;
            break;
    }

    free(buffer);
    if(outStream)
        fclose(outStream);

    loggerDestr();

    system("pause");
    $$ return 0;
}