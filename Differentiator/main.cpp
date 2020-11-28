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
    [x] чтение дерева из инфиксной нотации
    [x] переделать поле data в какой-нибудь темплейт (было просто копирование строк)
    [x] добавить парсер из строки в структуру
    [x] сделать парсер стандартных арифметических выражений
    [ ] написать генератор теха
    [x] написать упрощение дерева: проброска констант + уничтожение эквивалетных действий
    [ ] добавить стандартные функции
    [x] добавить рекуретные функции копирования
    [ ] добавить функцию рекурентного дифференцирования
    [ ] написать функции генерации теха учитывая стандартные функции
    [x] разнести весь код по отдельным файлам
    [x] прописать везде макросы, для отслеживания стека вызовов
    [ ] дописать в некоторых местах Asset_c(?)
    [x] конструктор копирования для класса Expression
*/



int main()
{
    initCallStack();
    loggerInit("log.log");
    $
    C_string line = "2*2+100*2+(100/x*20+x)";
    Parser pr;
    Expression exprTree;
    exprTree.genTreeByRoot(pr.parse(line));
    exprTree.simplify();
    exprTree.drawGraph();
    system("pause");
    $$
    loggerDestr();
    callStackDestr();
    return 0;
}