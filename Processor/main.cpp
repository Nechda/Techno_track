#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <io.h>
#include "Asm.h"
#include "CPU.h"
#include "Logger.h"

int readFullFile(const char* filename, char** outString,unsigned* readBytesPtr = NULL)
{


    FILE *f = fopen(filename, "rb");
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *string = (char*)calloc(fsize + 1,sizeof(char));
    unsigned nReadBytes = fread(string, 1, fsize, f);
    fclose(f);

    string[fsize] = 0;

    *outString = string;
    if (readBytesPtr)
        *readBytesPtr = nReadBytes;
    
    return 0;
}

int main(int argc, char** argv)
{
    loggerInit("compilator.log","w");

    char* inputFilename = "ASM_CODE.txt";
    char* outputFilename = "a.bin";
    char* codeStr = NULL;
    int errorCode = 0;

    //производим компил€цию в файл a.bin
    if(true)
    {
        errorCode = readFullFile(inputFilename, &codeStr);
        if (errorCode)
        {
            printf("We have some troubles with read code from file:%s\n", inputFilename);
            return -1;
        }

        FILE* outStream = fopen(outputFilename, "wb");

        errorCode = compile(codeStr, outStream);
        fclose(outStream);
        free(codeStr);
    }
    
    
    

    //теперь производим дизасеблирование
    if(true)
    {
        ui32 fileSize = 0;
        errorCode = readFullFile(outputFilename, &codeStr, &fileSize);
        if (errorCode)
        {
            printf("We have some troubles with read code from file:%s\n", outputFilename);
            return -1;
        }

        errorCode = disasm(codeStr, fileSize);
        free(codeStr);
    }

    //попробуем выполнить
    if(true)
    {
        ui32 fileSize = 0;
        errorCode = readFullFile(outputFilename, &codeStr, &fileSize);
        if (errorCode)
        {
            printf("We have some troubles with read code from file:%s\n", outputFilename);
            return -1;
        }
        cupInit();
        cpuRunProgram(codeStr, fileSize);
        free(codeStr);
    }

    loggerDestr();

    system("pause");
    return 0;
}