#pragma once
#include "Logger.h"

#ifndef NDEBUG
    #define $ pushInfo(__LINE__, __FUNCSIG__);
    #define $$ popInfo(__LINE__, __FUNCSIG__,"");
    #define $$$(msg) popInfo(__LINE__, __FUNCSIG__, msg);
    
    void pushInfo(unsigned line, char* func);
    void popInfo(unsigned line, char* func, char* msg);
#else
    #define $
    #define $$
    #define $$$(msg)
#endif

void initCallStack();
void callStackDestr();