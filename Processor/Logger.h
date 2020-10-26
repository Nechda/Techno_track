#pragma once
#include <stdio.h>
#include <stdlib.h>


FILE* getLoggerStream();


void logger(const char* tag, const char* format, ...);
void loggerAssert(const char* expr, const char* file,const char* function, unsigned line);
void loggerInit(const char* filename,const char* mode = "a");
void loggerDestr();