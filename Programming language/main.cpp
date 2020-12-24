#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ExprTree.h"
#include "Parser.h"
#include "CallStack.h"

#include <ctype.h>
#include <vector>
#include <stack>

/*
TODO:
    [ ] реализовать облась видимости для переменных
    [ ] оператор цикла
    [ ] массивы фиксированной длины
    [ ] передача в функцию по ссылке
*/




int main()
{
    initCallStack();
    loggerInit("log.log");
    $
    printf("Start parsing program...\n");
    Expression exprTree("program.pr");
    printf("Done!\n");

    printf("Start evaluate...\n");
    exprTree.evaluate();
    exprTree.getEvaluateStatus();
    system("pause");
    $$
    loggerDestr();
    callStackDestr();
    return 0;
}