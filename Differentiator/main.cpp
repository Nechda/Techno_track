#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ExprTree.h"
#include "Parser.h"
#include "CallStack.h"
#include "Allocator.h"

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
    [x] реализовать верификатор для парсера выражений
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
    [x] написать генерацию пошагового упрощения производной
*/


#define TEST_EVALUATE
#define TEST_DIFFERENTIATE
#define TEST_PARSER

#ifdef TEST_EVALUATE
#define TEST_DEFINE(strExpression,name,code,start,end,delta)\
    double evaluate_##name(double x) code
#include "Tests_evaluate.h"
#undef TEST_DEFINE
#endif

#ifdef TEST_DIFFERENTIATE
#define TEST_DEFINE(strExpression,name,code,start,end,delta)\
    double diff_##name(double x) code
#include "Tests_diff.h"
#undef TEST_DEFINE
#endif


void start_test()
{$
    Parser pr;
    #define ACCURACY 1E-4
    #ifdef TEST_EVALUATE
    {
        typedef double(*funcType)(double);
        struct TestInfo
        {
            funcType func;
            C_string line;
            double start;
            double end;
            double delta;
        };

        TestInfo testDataArray[] = {
            #define TEST_DEFINE(strExpression,name,code,start,end,delta)\
            { evaluate_##name, strExpression, start, end, delta },
            #include "Tests_evaluate.h"
            #undef TEST_DEFINE
        };

        for (ui32 i = 0; i < sizeof(testDataArray) / sizeof(testDataArray[0]); i++)
        {
            Expression exprTree;
            exprTree.genTreeByRoot(pr.parse(testDataArray[i].line));
            exprTree.simplify();

            double orig = 0;
            double eval = 0;
            printf("Start test function %s\n", testDataArray[i].line);
            for (double x = testDataArray[i].start; x < testDataArray[i].end; x += testDataArray[i].delta)
            {
                orig = testDataArray[i].func(x);
                eval = exprTree.evaluate(x);
                if(abs(eval-orig) < ACCURACY)
                    printf("Ok ");
                else
                    printf("\nf(%lf) = %lf,   my_f(%lf) = %lf  difference = %lf\n", x, orig, x, eval, abs(eval-orig));
            }
            printf("\n");
        }

    }
    #endif

    #ifdef TEST_DIFFERENTIATE
    {
        typedef double(*funcType)(double);
        struct TestInfo
        {
            funcType func;
            C_string line;
            double start;
            double end;
            double delta;
        };

        TestInfo testDataArray[] = {
            #define TEST_DEFINE(strExpression,name,code,start,end,delta)\
            { diff_##name, strExpression, start, end, delta },
            #include "Tests_diff.h"
            #undef TEST_DEFINE
        };

        for (ui32 i = 0; i < sizeof(testDataArray) / sizeof(testDataArray[0]); i++)
        {
            Expression exprTree;
            exprTree.genTreeByRoot(pr.parse(testDataArray[i].line));
            exprTree.differentiate();
            exprTree.simplify();

            double orig = 0;
            double eval = 0;
            printf("Start test function %s\n", testDataArray[i].line);
            for (double x = testDataArray[i].start; x < testDataArray[i].end; x += testDataArray[i].delta)
            {
                orig = testDataArray[i].func(x);
                eval = exprTree.evaluate(x);
                if(abs(eval-orig) < ACCURACY)
                    printf("Ok ");
                else
                    printf("\nf(%lf) = %lf,   my_f(%lf) = %lf  difference = %lf\n", x, orig, x, eval, abs(eval-orig));
            }
            printf("\n");
        }

    }
    #endif


    #ifdef TEST_PARSER
    {
        struct TestInfo
        {
            C_string line;
            bool answer;
        };

        TestInfo testDataArray[] =
        {
            #define TEST_DEFINE(expr, answ, comments)\
                {expr, answ },
            #include "Tests_parser.h"
            #undef TEST_DEFINE
        };

        printf("Start testing parser\n");
        for (ui16 i = 0; i < sizeof(testDataArray) / sizeof(testDataArray[0]); i++)
        {
            Expression::TNode* root = NULL;
            root = pr.parse(testDataArray[i].line);
            if (static_cast<bool>(root) == testDataArray[i].answer)
                printf("Ok ");
            else
                printf("Invalid answer for expression: %s\n should be: %d, my answer: %d\n", testDataArray[i].line, testDataArray[i].answer, !testDataArray[i].answer);
            if (root)
                rCleanUp(root);
        }
    }
    #endif

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
        exprTree.getStepByStepSimplificationTex("how_to_simplify", 2, 2);
        //exprTree.differentiate();
        //exprTree.simplify();
        exprTree.drawGraph("simplifiedTree");
        //exprTree.genTex();
        //exprTree.genTexFile("pretty_function");
    }
    system("pause");
    $$
    loggerDestr();
    callStackDestr();
    return 0;
}