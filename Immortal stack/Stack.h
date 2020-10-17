#pragma once
#include"Stack_kernel.h"


/**
\file Stack.h
\brief Данный файл содержит описание макросов, позволяющие создавать стек
       для произвольного типа данных.
\note  Для исползования в своем проекте необходимо прописать следующие строки:
\code
    #define TYPE_ int
    #include "Stack.h"
    #undef TYPE_
\endcode
      После такого включения можно будет использовать стек с типом int.
      Пример работы со стеком:
\code
    #include <stdio.h>
    #include <stdlib.h>

    #define TYPE_ double
    #include "Stack.h"
    #undef TYPE_



    int main()
    {
        Stack(double) stack;
        stackInit(&stack,1);
        stackPush(&stack,10.4);
        stackPush(&stack,20.6);
        stackPush(&stack,30.1);
        stackDump(stack);
        for (int i = 0; i < 10; i++)
        {
            double pop = 0;
            int errorCode = stackPop(&stack, &pop);
            if(!errorCode)
                printf("stack give us:%lf\n", pop);
            if (errorCode == STK_ERROR_STK_IS_EMPTY)
                printf("stack empty already!\n");
        }
        system("pause");
        return 0;
    }
\endcode
*/

/**
\brief Объявления макросов для конкатенации строк
*/
#define CONCAT_(a,b) a##b
#define CONCAT(a,b) CONCAT_(a,b)
#define Stack(type) CONCAT(Stack_,type)

/**
\brief Объявления стурктуры стека для заданного типа
*/
struct CONCAT(Stack_, TYPE_)                          
{                                                   
    const CanaryType leftSide = STK_CANARY_VALUE;
    Hash structHash;
    Hash dataHash;                                
    ui32 elementSize;                              
    ui32 size;                                      
    ui32 capacity;
    TYPE_* data;
    const CanaryType rightSide = STK_CANARY_VALUE;
};


/**
\brief Макросы, реализующие функционал стека с определенным типом данных, посредством функций
       работающих с абстрактным стеком.
@{
*/
inline int stackInit(CONCAT(Stack_, TYPE_)* stk, ui32 capacity)
{
    return _stackInit(stk, capacity, sizeof(TYPE_));
}

inline int stackPush(CONCAT(Stack_, TYPE_)* stk, TYPE_ value)
{
    TYPE_ val = value;
    return _stackPush(stk, &val);
}

inline int stackPop(CONCAT(Stack_, TYPE_)* stk, TYPE_* dest)
{
    return _stackPop(stk, dest);
}

inline void stackDest(CONCAT(Stack_, TYPE_)* stk)
{
    _stackDest(stk);
}

#define stackDump(stk) \
_stackDump( &stk ,__FILE__,__FUNCSIG__,__LINE__,#stk)

/**
}@
*/