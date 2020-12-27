#pragma once
#include "Logger.h"
#include "CallStack.h"



/**
\brief  Структура, описывающая входные данные командной строки
*/
struct InputParams
{
    char* inputFilename = NULL;
    char* outputFilename = NULL;
    bool wantDrawTree = 0;
};


/**
\brief  Функция парса аргументов командной строки
*/
void parseConsoleArguments(int argc, char** argv, InputParams* ptrInParams);