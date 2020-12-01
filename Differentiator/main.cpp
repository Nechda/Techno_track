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
    [ ] ?реализовать верификатор для парсера выражений?
    [x] реалиовать вычисления дерева, с подставленным значением переменной
    [x] написать тесты
    [x] добавить рекуретные функции копирования
    [x] добавить функцию рекурентного дифференцирования
    [x] реализовать дифференцирование для стандартных операций
    [x] реализовать макросы, позволяющие просто добавлять новые функции в базу дифференциатора
    [x] написать функции генерации теха учитывая стандартные функции sin,cos,tan,cot,ln
    [x] написать генератор теха который будет понимать корни и степени
    [x] разнести весь код по отдельным файлам
    [x] прописать везде макросы, для отслеживания стека вызовов
    [x] дописать Assert'ы где только можно
    [x] конструктор копирования для класса Expression
*/

#define TEST_DEFINE(strExpression,name,code,start,end,delta)\
    double evaluate_##name(double x) code
#include "Tests_evaluate.h"
#undef TEST_DEFINE

#define TEST_DEFINE(strExpression,name,code,start,end,delta)\
    double diff_##name(double x) code
#include "Tests_diff.h"
#undef TEST_DEFINE

void start_test()
{$
    C_string line = NULL;
    Parser pr;
    #define ACCURACY 1E-4
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
            if(abs(eval-orig) < ACCURACY)\
                printf("Ok ");\
            else\
                printf("\nf(%lf) = %lf,   my_f(%lf) = %lf  difference = %lf\n", x, orig, x, eval, abs(eval-orig));\
        }\
        printf("\n");\
    }
    #include "Tests_evaluate.h"
    #undef TEST_DEFINE
    #define TEST_DEFINE(strExpression,name,code,start,end,delta)\
    {\
        line = strExpression;\
        Expression exprTree; \
        exprTree.genTreeByRoot(pr.parse(line));\
        exprTree.differentiate();\
        exprTree.simplify();\
        double orig = 0;\
        double eval = 0;\
        printf("Start test diff of function %s\n", strExpression);\
        for (double x = start; x < end; x += delta)\
        {\
            orig = diff_##name(x);\
            eval = exprTree.evaluate(x);\
            if(abs(eval-orig) < ACCURACY)\
                printf("Ok ");\
            else\
                printf("\nf(%lf) = %lf,   my_f(%lf) = %lf  difference = %lf\n", x, orig, x, eval, abs(eval-orig));\
        }\
        printf("\n");\
    }
    #include "Tests_diff.h"
    #undef TEST_DEFINE
    $$
}



int main()
{
    initCallStack();
    loggerInit("log.log");
    $

    bool isTestingMode = 0;
    if(isTestingMode)
        start_test();
    else
    {
        Expression exprTree("expr.txt");
        exprTree.drawGraph("originalTree");
        exprTree.differentiate();
        exprTree.simplify();
        exprTree.drawGraph("simplifiedTree");
        exprTree.genTex();
        exprTree.genTexFile("pretty_function");
    }
    system("pause");
    $$
    loggerDestr();
    callStackDestr();
    return 0;
}