#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ExprTree.h"
#include "Parser.h"
#include "CallStack.h"

#include <ctype.h>
#include <vector>
#include <stack>



int main()
{
    initCallStack();
    loggerInit("log.log");
    $
    printf("Start parsing program...\n");
    Expression exprTree("program.pr");
    printf("Done!\n");
    //printf("Start draw tree\n");
    //exprTree.drawGraph("originalTree");
    //printf("Start evaluate...\n");
    //exprTree.evaluate();
    //exprTree.getEvaluateStatus();
    exprTree.compile("prog.asm");
    system("pause");
    $$
    loggerDestr();
    callStackDestr();
    return 0;
}