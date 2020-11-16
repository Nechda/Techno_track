#include "Argparser.h"
#include "Asm.h"
#include <string.h>

/**
\brief  Функция парса конфигурационного файла
*/
void parseConfig(const char* configFileName, InputParams* ptrInParams)
{
    $
    Assert_c(configFileName);
    if (!configFileName)
    {
        $$$("Null ptr in input data");
        return;
    }
    int errorCode = 0;
    char* buff = NULL;
    errorCode = readFullFile(configFileName, &buff);
    if (errorCode == ASM_ERROR_CODE)
    {
        logger("Config parser error", "We have problems with reading config file.\n");
        if (buff)
            free(buff);
        $$$("Can't read config file");
        return;
    }

    errorCode = removeExtraChar(&buff, "=\n");
    if (errorCode == ASM_ERROR_CODE)
    {
        logger("Config parser error", "We have problems with formating config file.\n");
        if (buff)
            free(buff);
        $$$("Problems with del extra characters from buffer");
        return;
    }

    char* tok = strtok(buff, "=\n");
    ConfigParamType paramNumber;
    int paramValue = 0;
    while (tok != NULL)
    {
        paramNumber = CONFIG_UNDEFINED_PARAM;
        if (!strcmp(tok, "memorySize"))
            paramNumber = CONFIG_MEMORY_SIZE;
        if (!strcmp(tok, "nCols"))
            paramNumber = CONFIG_NCOLS;
        if (!strcmp(tok, "nLines"))
            paramNumber = CONFIG_NLINES;
        if (!strcmp(tok, "entryPoint"))
            paramNumber = CONFIG_ENTRY_POINT;
        if (!strcmp(tok, "fontWidth"))
            paramNumber = CONFIG_FONT_WIDTH;
        if (!strcmp(tok, "fontHeight"))
            paramNumber = CONFIG_FONT_HEIGHT;

        if (paramNumber != CONFIG_UNDEFINED_PARAM)
        {
            tok = strtok(NULL, "=\n");
            sscanf(tok, "%d", &paramValue);
        }

        switch (paramNumber)
        {
            case CONFIG_MEMORY_SIZE:
                ptrInParams->memorySize = paramValue;
                break;
            case CONFIG_ENTRY_POINT:
                ptrInParams->entryPoint = paramValue;
                break;
            case CONFIG_NCOLS:
                ptrInParams->Window.nCols = paramValue;
                break;
            case CONFIG_NLINES:
                ptrInParams->Window.nLines = paramValue;
                break;
            case CONFIG_FONT_WIDTH:
                ptrInParams->Window.fontWidth = paramValue;
                break;
            case CONFIG_FONT_HEIGHT:
                ptrInParams->Window.fontHeight = paramValue;
                break;
            default:
                logger("Config parser error", "Invalid syntax of config file! We can't find param \"%s\" in our table.\n", tok);
                if (buff)
                    free(buff);
                $$$("Invalid syntax in config file");
                return;
        }
        tok = strtok(NULL, "=\n");
    }

    free(buff);
    $$
}


/**
\brief  Функция парса аргументов командной строки
*/
void parseConsoleArguments(int argc, char** argv, InputParams* ptrInParams)
{
    $
    char** curStr = NULL;
    char* buf = NULL;
    char option = 0;
    int intBuf = 0;
    for (int i = 1; i < argc; i++)
    {
        option = argv[i][0] == '-' ? argv[i][1] : 0;
        switch (option)
        {
        case 'h':
            ptrInParams->programName = PROG_HELPER;
            break;
        case 'c':
            ptrInParams->programName = PROG_COPMILATOR;
            break;
        case 'r':
            ptrInParams->programName = PROG_CPU;
            break;
        case 'd':
            ptrInParams->programName = PROG_DISASSEMBLER;
            break;
        case 'e':
            ptrInParams->programName = PROG_RUN;
            break;
        case 't':
            ptrInParams->programName = PROG_TESTER;
            break;
        case 'l':
            curStr = &ptrInParams->logFilename;
            break;
        case 'i':
            curStr = &ptrInParams->inputFilename;
            break;
        case 'o':
            curStr = &ptrInParams->outputFilename;
            break;
        case 'n':
            ptrInParams->noLogFileFlag = 1;
            break;
        case 's':
            ptrInParams->useStepByStepMode = 1;
            break;
        case 'g':
            ptrInParams->useGraphMode = 1;
            continue;
        case '-':
            argv[i] += 2;
            buf = strtok(argv[i], "=");


            if (!strcmp(buf, "config"))
                parseConfig(strtok(NULL, "="),ptrInParams);
            else
                sscanf(strtok(NULL, "="), "%d", &intBuf);

            if (!strcmp(buf, "nCols"))
                ptrInParams->Window.nCols = intBuf;
            if (!strcmp(buf, "nLines"))
                ptrInParams->Window.nLines = intBuf;
            if (!strcmp(buf, "memorySize"))
                ptrInParams->memorySize = intBuf;
            if (!strcmp(buf, "entryPoint"))
                ptrInParams->entryPoint = intBuf;

            argv[i] -= 2;
            break;
        default:
            *curStr = argv[i];
            break;
        }
    }
    $$
}