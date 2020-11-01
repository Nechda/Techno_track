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



/**
TODO:
    Надо разобраться с функциями дампа процессора: сделать их независимыми от того, был ли инициализирован лог файл или нет.
*/


/**
\brief  Структура, описывающая входные данные командной строки
*/
struct
{
    char* programName = NULL;
    char* inputFilename = NULL;
    char* outputFilename = NULL;
    char* logFilename = "log.log";
}inputParams;


/**
\defgroup gr1 Константы, номера исполняемой программы
\breif  Константы, определяющие номер исполняемой программы
@{*/
const ui8 PROGRAM_CODE_CPU = 1;
const ui8 PROGRAM_CODE_COMPILATOR = 2;
const ui8 PROGRAM_CODE_DISASSEMBLER = 3;
/**}@*/

int main(int argc, char** argv)
{
    #ifndef NDEBUG
        if(argc == 1)
        {
            loggerInit("log.log", "w");
            cpuStartTests();
            loggerDestr();
            system("pause");
            return 0;
        }
    #endif

    bool noLogFileFlag = 0;
    char** curStr = NULL;
    for (int i = 1; i < argc; i++)
    {
        if (!strcmp("-p", argv[i]))
        {
            curStr = &inputParams.programName;
            continue;
        }
        if (!strcmp("-i", argv[i]))
        {
            curStr = &inputParams.inputFilename;
            continue;
        }
        if (!strcmp("-o", argv[i]))
        {
            curStr = &inputParams.outputFilename;
            continue;
        }
        if (!strcmp("-l", argv[i]))
        {
            curStr = &inputParams.logFilename;
            continue;
        }
        if (!strcmp("-ln", argv[i]))
        {
            noLogFileFlag = 1;
            continue;
        }

        *curStr = argv[i];
    }

    loggerInit(noLogFileFlag ? NULL : inputParams.logFilename,"w");

    if (!inputParams.programName)
    {
        printf("Error: You didn't choose program that want to execute!\n");
        printf("Run the program with -p <program name>\n");
        printf("<program name> could be \"compilator\",\"disassembler\",\"cpu\"\n");
        return -1;
    }

    

    char* codeStr = NULL;
    int errorCode = 0;
    unsigned inputFileSize = 0;

    if (!inputParams.inputFilename)
    {
        printf("Error: No input file specified!\n");
        return -1;
    }


    inputFileSize = readFullFile(inputParams.inputFilename, &codeStr);
    if (inputFileSize == ASM_ERROR_CODE)
    {
        printf("Error: We have some troubles with read code from file:%s\n", inputParams.inputFilename);
        return -1;
    }


    ui8 programIndex = 0;
    if (!strcmp("cpu", inputParams.programName))
        programIndex = PROGRAM_CODE_CPU;
    if (!strcmp("compilator", inputParams.programName))
        programIndex = PROGRAM_CODE_COMPILATOR;
    if (!strcmp("disassembler", inputParams.programName))
        programIndex = PROGRAM_CODE_DISASSEMBLER;

    if (!programIndex)
    {
        printf("Error: Invalid name program!\n");
        return -1;
    }

    if (programIndex == PROGRAM_CODE_CPU)
    {
        cupInit();
        errorCode = cpuRunProgram(codeStr, inputFileSize);
        free(codeStr);
        printf("CPU finished with the code: %d (%s)\n", errorCode, getStringByErrorCode((CPUerror)errorCode));
        if (!noLogFileFlag)
            printf("More infromation see in log file: %s\n", inputParams.logFilename);
        return errorCode;
    }


    if (!inputParams.outputFilename)
    {
        printf("Warning: No output file specified. By default result will write into a.out\n");
        inputParams.outputFilename = "a.out";
    }


    FILE* outStream = fopen(inputParams.outputFilename, programIndex == PROGRAM_CODE_COMPILATOR ? "wb" : "w");
    if (!outStream)
    {
        printf("Error: We can't open file %s for writing result.\n", inputParams.outputFilename);
        return -1;
    }
    if (ferror(outStream))
    {
        printf("Error: Stream for writing result has error.\n");
        return -1;
    }



    if(programIndex == PROGRAM_CODE_COMPILATOR)
    {
        errorCode = compile(codeStr, outStream);
        fclose(outStream);
        free(codeStr);
        printf("Compilation finished with the code: %d (%s)\n", errorCode, getStringByErrorCode((AsmError)errorCode));
        if (!noLogFileFlag)
            printf("More infromation see in log file: %s\n", inputParams.logFilename);
        return errorCode;
    }
    
    if(programIndex == PROGRAM_CODE_DISASSEMBLER)
    {
        errorCode = disasm((ui8*)codeStr, inputFileSize, outStream);
        fclose(outStream);
        free(codeStr);
        printf("Disassembler finished with the code: %d (%s)\n", errorCode, getStringByErrorCode((AsmError)errorCode));
        if (!noLogFileFlag)
            printf("More infromation see in log file: %s\n", inputParams.logFilename);
        return errorCode;
    }


    loggerDestr();

    return 0;
}