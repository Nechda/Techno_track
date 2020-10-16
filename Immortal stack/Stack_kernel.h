#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

//#define NDEBUG

#ifdef NDEBUG
#define Assert_c(expr) if(!(expr))printf("Expression %ls is false.\n In file: %ls\n line: %d\n",_CRT_WIDE(#expr), _CRT_WIDE(__FILE__), (unsigned)(__LINE__)); ///< Реализация assert для релиза
#else
#define Assert_c(expr) (void* )0; ///< Реализация assert для отладки переключить режим можно директивой #define NDEBUG
#endif

/**
\brief Для краткости дальнейших описаний вводим короткие имена стандартных типов
@{
*/
typedef unsigned char ui8;
typedef unsigned int ui32;
typedef unsigned long long ui64;
typedef ui64 CanaryType;
typedef ui64 Hash;
/**}@*/

/**
\brief Константы, регулирующие поведение стека
@{
*/
const CanaryType STK_CANARY_VALUE = 0x99996666AAAA5555; ///< Magic number, который записывается в переменные-канарейки, для отслеживания атак на структуру слева и справа.
const ui64 STK_BUFFER_ADDITION = 8;                     ///< Константа, на которую увеличиывается буфер capacity, после того, как место заканчивается
/**}@*/


/**
\brief Константы ошибок
@{
*/
const int STK_ERROR_NULL_PTR            = -1; ///< Ошибка возникает, если в функцию передали нулевой указатель
const int STK_ERROR_OUT_OF_MEMORY       = -2; ///< Ошибка возникает, если не удалось выделить память calloc или возникла ошибка при вызове realloc
const int STK_ERROR_INVALID_PTR         = -3; ///< Ошибка возникает, если указатель на считываемые данные или data в структуре является NULL
const int STK_ERROR_OUT_OF_RANGE        = -4; ///< Ошибка возникает, если размер стека больше, чем выделенная память
const int STK_ERROR_ATTACK              = -5; ///< Ошибка возникает, если структура стека или данные были испорчены, посредством изменения канареечных переменных
const int STK_ERROR_STK_IS_EMPTY        = -6; ///< Ошибка возникает, если из пустого стека пытаются вытащить данные
const int STK_ERROR_CHANGED_DATA        = -7; ///< Ошибка возникает, если данные были испочены, проверяется посредством подсчета хеша по массиву данных
const int STK_ERROR_CHANGED_STRUCTURE   = -8; ///< Ошибка возникает, если структура стека была испорчена, проверяется посредством подсчета хеша по структуре стека
/**}@*/


/**
\brief    Абстрактная структура стека, которая не сореджит информации о типе данных.
\detail   Весь функционал стека реализуется посредством данной структуры, затем макросами
          создаются конкретные стеки с нужным типом данных.
*/
struct _BaseStack
{
    const CanaryType leftSide = STK_CANARY_VALUE;
    Hash structHash;
    Hash dataHash;
    ui32 elementSize;
    ui32 size;
    ui32 capacity;
    void* data;
    const CanaryType rightSide = STK_CANARY_VALUE;
};


/**
\brief   Функции для работы с абстрактным стеком.
@{
*/
int stackInit_(void* stk, const ui32 capacity, const ui32 elementSize);
int stackValidity(const void* stk);
int stackPush_(void* stk, void* value);
int stackPop_(void* stk, void* dest);
void stackDump_(void* stk, const char* file, const char* func, const ui32 line, const char* variableName);
void stackDest_(void* stk);
/**
}@
\note Подробное описание каждой функции дано в файле Stack.cpp
*/