#pragma once
#include <stdio.h>
#include <stdlib.h>

void logger(const char* tag, const char* msg);
void loggerAssert(const char* expr, const char* file,const char* function, unsigned line);
FILE* getLoggerStream();
void loggerInit(const char* filename,const char* mode = "a");
void loggerDestr();