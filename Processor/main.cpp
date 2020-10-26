#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <io.h>
#include <string.h>
#include <ctype.h>
#include "Asm.h"
#include "CPU.h"
#include "Logger.h"

int readFullFile(const char* filename, char** outString,unsigned* readBytesPtr = NULL)
{
    Assert_c(filename);
    Assert_c(outString);
    if (!filename || !outString)
        return ASM_ERROR_CODE;

    FILE* inputFile = fopen(filename, "rb");
    Assert_c(inputFile);
    if (!inputFile)
        return ASM_ERROR_CODE;
    if (ferror(inputFile))
        return ASM_ERROR_CODE;

    fseek(inputFile, 0, SEEK_END);
    long fsize = ftell(inputFile);
    fseek(inputFile, 0, SEEK_SET);

    char* string = (char*)calloc(fsize + 1, sizeof(char));
    Assert_c(string);
    if (!string)
        return ASM_ERROR_CODE;

    unsigned nReadBytes = fread(string, sizeof(char), fsize, inputFile);
    fclose(inputFile);
    string[fsize] = 0;

    *outString = string;
    if (readBytesPtr)
        *readBytesPtr = nReadBytes;
    
    return 0;
}

struct
{
    char* programName = NULL;
    char* inputFilename = NULL;
    char* outputFilename = NULL;
    char* logFilename = "log.log";
}inputParams;

int main(int argc, char** argv)
{
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


    errorCode = readFullFile(inputParams.inputFilename, &codeStr, &inputFileSize);
    if (errorCode)
    {
        printf("Error: We have some troubles with read code from file:%s\n", inputParams.inputFilename);
        return -1;
    }


    if (!inputParams.outputFilename)
    {
        printf("Warning: No output file specified. By default result will write into a.bin.\n");
        inputParams.outputFilename = "a.bin";
    }

    FILE* outStream = fopen(inputParams.outputFilename, "wb");
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


    if (!strcmp("cpu", inputParams.programName))
    {
        cupInit();
        cpuRunProgram(codeStr, inputFileSize);
        free(codeStr);
        printf("CPU finished with the code: %d (%s)\n", errorCode, getStringByErrorCode((CPUerror)errorCode));
        if (!noLogFileFlag)
            printf("More infromation see in log file: %s\n", inputParams.logFilename);
        return errorCode;
    }


    if(!strcmp("compilator", inputParams.programName))
    {
        errorCode = compile(codeStr, outStream);
        fclose(outStream);
        free(codeStr);
        printf("Compilation finished with the code: %d (%s)\n", errorCode, getStringByErrorCode((AsmError)errorCode));
        if (!noLogFileFlag)
            printf("More infromation see in log file: %s\n", inputParams.logFilename);
        return errorCode;
    }
    
    if(!strcmp("disassembler", inputParams.programName))
    {
        errorCode = disasm(codeStr, inputFileSize, outStream);
        free(codeStr);
        printf("Disassembler finished with the code: %d (%s)\n", errorCode, getStringByErrorCode((AsmError)errorCode));
        if (!noLogFileFlag)
            printf("More infromation see in log file: %s\n", inputParams.logFilename);
        return errorCode;
    }


    loggerDestr();
    system("pause");

    return 0;
}