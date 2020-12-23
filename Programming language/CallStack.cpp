#include "CallStack.h"
#include "Logger.h"
#include <stdlib.h>
#include <string.h>


FILE* fptr = NULL;
unsigned level = 0;

void initCallStack()
{
    #ifndef NDEBUG
        fptr = fopen("callStack", "w");
        if (!fptr)
            return;
        setvbuf(fptr, NULL, _IONBF, 0);
    #endif
}

/**
\brief Функция уничтожения логгера
\note  Данная функция просто вызывает fclose()
*/
void callStackDestr()
{
    #ifndef NDEBUG
        if (fptr)
            fclose(fptr);
        fptr = NULL;
    #endif
}

void pushInfo(unsigned line, char* func)
{
    if (!fptr)
        return;
    level++;
    for (int i = 0; i < level-1; i++)
        fprintf(fptr,"  ");
    fprintf(fptr,"function: %s\n", func);
}

void popInfo(unsigned line, char* func,char* msg)
{
    if (!fptr)
        return;
    if (level < 0)
    {
        fprintf(fptr, "You forgot $!");
        return;
    }
    for (int i = 0; i < level; i++)
        fprintf(fptr, "  ");
    if(!strcmp("", msg))
        fprintf(fptr, "return OK\n", line);
    else
        fprintf(fptr, "return at line: %d   reason:%s \n", line, msg);
    level--;
}