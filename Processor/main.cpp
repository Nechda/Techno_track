#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include "Asm.h"
#include "CPU.h"
#include "Logger.h"
#include "Tester.h"


enum ProgramType
{
    PROG_UNDEFINED = 0,
    PROG_COPMILATOR,
    PROG_DISASSEMBLER,
    PROG_CPU,
    PROG_RUN,
    PROG_TESTER
};

/**
\brief  Структура, описывающая входные данные командной строки
*/
struct
{
    ProgramType programName = PROG_UNDEFINED;
    char* inputFilename = NULL;
    char* outputFilename = NULL;
    char* memorySize = NULL;
    char* logFilename = "log.log";
    bool noLogFileFlag = 0;
}inputParams;




/**
\brief Константы, описывающие коды ошибок, возникающие в main
*/
const ui8 ERROR_INIT_INPUT_STREAM           = 1 << 0;
const ui8 ERROR_INIT_OUTPUT_STREAM          = 1 << 1;
const ui8 ERROR_JUMP_DEFAULT_CASE_IN_SWITCH = 1 << 2;


int run_Compilator(char* buffer, FILE* outStream)
{
    int errorCode = 0;
    errorCode = compile(buffer, outStream);
    printf("Compilation finished with the code: %d (%s)\n", errorCode, getStringByErrorCode((AsmError)errorCode));
    if (!inputParams.noLogFileFlag && errorCode)
        printf("More infromation see in log file: %s\n", inputParams.logFilename);
    return errorCode;
}

int run_Disassembler(char* buffer,unsigned nBytes, FILE* outStream)
{
    int errorCode = 0;
    errorCode = disasm((ui8*)buffer, nBytes, outStream);
    printf("Disassembler finished with the code: %d (%s)\n", errorCode, getStringByErrorCode((AsmError)errorCode));
    if (!inputParams.noLogFileFlag && errorCode)
        printf("More infromation see in log file: %s\n", inputParams.logFilename);
    return errorCode;
}

int run_CPU(char* buffer, unsigned nBytes)
{
    ui32 ramSize = 0;
    if (inputParams.memorySize)
        sscanf(inputParams.memorySize, "%d", &ramSize);
    cupInit(ramSize);

    int errorCode = 0;
    errorCode = cpuRunProgram(buffer, nBytes);
    printf("CPU finished with the code: %d (%s)\n", errorCode, getStringByErrorCode((CPUerror)errorCode));
    if (!inputParams.noLogFileFlag && errorCode)
        printf("More infromation see in log file: %s\n", inputParams.logFilename);
    cpuDestr();
    return errorCode;
}

int initInStream(char* inputFileName,char** buffer, unsigned* readBytes)
{
    Assert_c(inputFileName);
    Assert_c(buffer);
    Assert_c(readBytes);
    if (!buffer || !readBytes)
        return ERROR_INIT_INPUT_STREAM;

    if (!inputFileName)
    {
        printf("Error: No input file specified!\n");
        return ERROR_INIT_INPUT_STREAM;
    }

    *readBytes = readFullFile(inputFileName, buffer);
    if (*readBytes == ASM_ERROR_CODE)
    {
        printf("Error: We have some troubles with read code from file:%s\n", inputFileName);
        return ERROR_INIT_INPUT_STREAM;
    }
    return 0;
}

int initOutStream(char* outputFileName, FILE** outStreamPtr, char* mode)
{
    Assert_c(outStreamPtr);
    Assert_c(mode);
    if (!outStreamPtr || !mode)
        return ERROR_INIT_OUTPUT_STREAM;

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
        return ERROR_INIT_OUTPUT_STREAM;
    }
    if (ferror(*outStreamPtr))
    {
        printf("Error: Stream for writing result has error.\n");
        return ERROR_INIT_OUTPUT_STREAM;
    }
    return 0;
}

void printHelp()
{
    printf(
        "There are some flags you can run program with, here is their description:\n"
        "-h                Shows help to the program and describes all flags that you can use.\n"
        "-c                Calls compilator, you also need use -i flag\n"
        "-d                Calls disassembler, you also need use -i flag\n"
        "-r                Calls virtual CPU to execute your program, you also need use -i flag\n"
        "-e                This is a mixture of two programs:it first calls compilator, then virtual CPU\n"
        "you also need use -i flag to select file with a program code. There is -o flag, but it's not\n"
        "necessary: by default result of compilation will save into \"a.out\" file.\n"
        "-t                Calls test programs, that located in Tests folder.\n"
        "-l <filename>     This flag sets the log file, by default log file has name \"log.log\".\n"
        "-i <filename>     This flag sets the input file, from this file program will read infromation.\n"
        "-o <filename>     This flag sets the output file, into this file program will write result,\n"
        "by default file has name \"a.out\" file.\n"
        "-n                This flag disable writing in log file.\n"
        "-s                Sets the processor's program execution mode to step-by-step.\n"
        "-m <size>         Sets size of virtual RAM in bytes.(Actual RAM size will be the maximum between 128 and your size)\n"
    );
}

void parseConsoleArguments(int argc, char** argv)
{
    char** curStr = NULL;
    char option = 0;
    for (int i = 1; i < argc; i++)
    {
        option = argv[i][0] == '-' && !argv[i][2] ? argv[i][1] : 0;
        switch (option)
        {
            case 'h':
                printHelp();
                break;
            case 'c':
                inputParams.programName = PROG_COPMILATOR;
                break;
            case 'r':
                inputParams.programName = PROG_CPU;
                break;
            case 'd':
                inputParams.programName = PROG_DISASSEMBLER;
                break;
            case 'e':
                inputParams.programName = PROG_RUN;
                break;
            case 't':
                inputParams.programName = PROG_TESTER;
                break;
            case 'l':
                curStr = &inputParams.logFilename;
                break;
            case 'i':
                curStr = &inputParams.inputFilename;
                break;
            case 'o':
                curStr = &inputParams.outputFilename;
                break;
            case 'm':
                curStr = &inputParams.memorySize;
                break;
            case 'n':
                inputParams.noLogFileFlag = 1;
                break;
            case 's':
                setStepByStepMode(1);
                break;
            default:
                *curStr = argv[i];
                break;
        }
    }
}

int main(int argc, char** argv)
{
    parseConsoleArguments(argc, argv);

    if (inputParams.programName == PROG_UNDEFINED)
    {
        printf("We don't understand what you want to execute ...\nTry to use -h flag to get more information.");
        return 0;
    }

    loggerInit(inputParams.noLogFileFlag ? NULL : inputParams.logFilename, "w");

    if (inputParams.programName == PROG_TESTER)
    {
        prepareCompilatorsTable();
        cpuStartTests();
        loggerDestr();
        return 0;
    }

    

    char* buffer = NULL;
    int errorCode = 0;
    unsigned inputFileSize = 0;
    FILE* outStream = NULL;
    
    //Инициализируем "потоки ввода и вывода"
    switch (inputParams.programName)
    {
        case PROG_COPMILATOR:
        case PROG_DISASSEMBLER:
        case PROG_RUN:
            errorCode |= initOutStream(inputParams.outputFilename, &outStream, inputParams.programName == PROG_COPMILATOR ? "wb" : "w");
        case PROG_CPU:
            errorCode |= initInStream(inputParams.inputFilename, &buffer, &inputFileSize);
            break;
        default:
            errorCode |= ERROR_JUMP_DEFAULT_CASE_IN_SWITCH;
            break;
    }
    if (errorCode)
    {
        printf("Error: We have problems with %s stream\n", errorCode & 1 ? "input" : "output");
        return errorCode;
    }

    //Производим выполнение программы
    prepareCompilatorsTable();
    switch (inputParams.programName)
    {
    case PROG_COPMILATOR:
        errorCode |= run_Compilator(buffer, outStream) << 4;
        break;
    case PROG_DISASSEMBLER:
        errorCode |= run_Disassembler(buffer, inputFileSize, outStream) << 4;
        break;
    case PROG_RUN:
        errorCode |= run_Compilator(buffer, outStream) << 4;
        free(buffer);
        fclose(outStream);
        errorCode |= initInStream(inputParams.outputFilename, &buffer, &inputFileSize);
        errorCode |= run_CPU(buffer, inputFileSize) << 4;
        break;
    case PROG_CPU:
        errorCode |= run_CPU(buffer, inputFileSize) << 4;
        break;
    default:
        errorCode |= ERROR_JUMP_DEFAULT_CASE_IN_SWITCH;
        break;
    }

    free(buffer);
    if(outStream)
        fclose(outStream);

    loggerDestr();

    //system("pause");
    return 0;
}