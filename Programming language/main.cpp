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
    [x] void Parser::parse_fact(Expression::TNode** ptrNode, ui32& p, Expression::TNode* parent) -> createNode(...) переделать аргумент так, чтобы можно было передавать юнион
    [x] переписать функцию рисования дерева
    [x] переписать функции из ExprTree.cpp с учетом того, что детей может быть TREE_CHILD_NUMBER штук
     \-->[x] переписана функция рисования дерева из ExprTree.cpp
*/




int main()
{
    initCallStack();
    loggerInit("log.log");
    $

    Expression exprTree("program.txt");
    exprTree.drawGraph("originalTree");
    exprTree.simplify();
    exprTree.drawGraph("simplifiedTree");
    exprTree.evaluate();
    system("pause");
    $$
    loggerDestr();
    callStackDestr();
    return 0;
}