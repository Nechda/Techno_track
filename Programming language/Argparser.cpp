#include "Argparser.h"
#include <string.h>

/**
\brief  Функция парса аргументов командной строки
*/
void parseConsoleArguments(int argc, char** argv, InputParams* ptrInParams)
{
    $
    char** curStr = NULL;
    char option = 0;
    for (int i = 1; i < argc; i++)
    {
        option = argv[i][0] == '-' ? argv[i][1] : 0;
        switch (option)
        {
        case 'i':
            curStr = &ptrInParams->inputFilename;
            break;
        case 'o':
            curStr = &ptrInParams->outputFilename;
            break;
        case 'g':
            ptrInParams->wantDrawTree = 1;
            break;
        default:
            *curStr = argv[i];
            break;
        }
    }
    $$
}