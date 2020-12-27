#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ExprTree.h"
#include "Parser.h"
#include "CallStack.h"
#include "Argparser.h"



int main(int argc, char** argv)
{
    InputParams inputParams;
    initCallStack();
    loggerInit("log.log");
    $
    parseConsoleArguments(argc, argv, &inputParams);
    printf("Start parsing program...   ");
    Expression exprTree(inputParams.inputFilename);
    printf("Done!\n");
    printf("Tree simplification...     ");
    exprTree.simplify();
    printf("Done!\n");
    if (inputParams.wantDrawTree)
    {
        printf("Start drawing tree...      ");
        exprTree.drawGraph();
        printf("Done!\n");
    }
    printf("Compiation...              ");
    exprTree.compile(inputParams.outputFilename);
    exprTree.getEvaluateStatus();
    system("pause");
    $$
    loggerDestr();
    callStackDestr();
    return 0;
}