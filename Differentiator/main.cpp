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
    [x] добавить парсер из строки в структуру
    [x] сделать парсер стандартных арифметических выражений
    [x] написать генератор теха
    [x] написать упрощение дерева: проброска констант + уничтожение эквивалетных действий
    [x] реализовать хранение чисел в double
    [x] добавить стандартные функции sin,cos,tan,cot,ln
    [x] добавить приоритет стандартным функциям
    [x] реализовать стандартную функцию извелечения корня ( у \sqrt есть особенность, связанная с тем, что она требует {}, а не обчные скобки ())
    [x] реализовать операцию возведения в степень ( скорее всего она будет иметь тот же приоритет, что и *,/ )
    [x] дописать в упрощение степенных шаблонов x^1, x^0, 0^x, 1^x
    [x] написать макросы для удобного добавления новых функций
    [x] разобраться с enum'мами для функций в парсере
    [x] переписать функцию genTexInteration
    [x] переписать функцию identitySimplify
    [x] реализовать корректное распознавание унадрного минуса
    [ ] реализовать верификатор для парсера выражений
    [x] реалиовать вычисления дерева, с подставленным значением переменной
    [x] написать тесты
    [x] добавить рекуретные функции копирования
    [ ] добавить функцию рекурентного дифференцирования
    [x] написать функции генерации теха учитывая стандартные функции sin,cos,tan,cot,ln
    [x] написать генератор теха который будет понимать корни и степени
    [x] разнести весь код по отдельным файлам
    [x] прописать везде макросы, для отслеживания стека вызовов
    [x] дописать Assert'ы где только можно
    [x] конструктор копирования для класса Expression
*/

#define TEST_DEFINE(strExpression,name,code,start,end,delta)\
    double evaluate_##name(double x) code
#include "Tests.h"
#undef TEST_DEFINE

void start_test()
{
    
    C_string line = NULL;
    Parser pr;
    #define TEST_DEFINE(strExpression,name,code,start,end,delta)\
    {\
        line = strExpression;\
        Expression exprTree; \
        exprTree.genTreeByRoot(pr.parse(line));\
        exprTree.simplify();\
        double orig = 0;\
        double eval = 0;\
        printf("Start test function %s\n", strExpression);\
        for (double x = start; x < end; x += delta)\
        {\
            orig = evaluate_##name(x);\
            eval = exprTree.evaluate(x);\
            if(abs(eval-orig) < 1E-4)\
                printf("Ok ");\
            else\
                printf("\nf(%lf) = %lf,   my_f(%lf) = %lf  difference = %lf\n", x, orig, x, eval, abs(eval-orig));\
        }\
    }
    #include "Tests.h"
    #undef TEST_DEFINE
    
}



int main()
{
    initCallStack();
    loggerInit("log.log");
    $
    //start_test();
    C_string line = "ln(100)/ln(10) *x + sin(x+10.0 + (5*3)/2) + tan(11.5)/x";
    Parser pr;
    Expression exprTree;
    exprTree.genTreeByRoot(pr.parse(line));
    exprTree.drawGraph("originalTree");
    exprTree.simplify();
    exprTree.drawGraph("simplifiedTree");
    exprTree.genTex();
    
    system("pause");
    $$
    loggerDestr();
    callStackDestr();
    return 0;
}