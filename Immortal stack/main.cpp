#include <stdio.h>
#include <stdlib.h>
#include "Logger.h"
#include "Tester.h"


int main()
{
    loggerInit("Stack.log","w");
    stackTester();
    system("pause");
    loggerDestr();
    return 0;
}