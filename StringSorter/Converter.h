#pragma once
#include <wchar.h>
#include <assert.h>


#ifdef NDEBUG
#define Assert_c(expr) if(!(expr))printf("Expression %ls is false.\n In file: %ls\n line: %d\n",_CRT_WIDE(#expr), _CRT_WIDE(__FILE__), (unsigned)(__LINE__)); ///< Реализация assert для релиза
#else
#define Assert_c(expr) assert(expr); ///< Реализация assert для отладки переключить режим можно директивой #define NDEBUG
#endif

const int CONVERTER_ERR_NULL_PTR = -1; ///< Возвращается, если при работе фунций возник нулевой указатель
const int CONVERTER_ERR_CANT_CONVERT = -2; ///< Возвращается, если строка в utf8 имеет символы, для кодирования которых требуется более трех байт.

int utf8StrLen(const char* src); ///< Возвращает число символов, закодированных в utf8
int utf8ToWchar(wchar_t* dest, const char* src); ///< Преобразвует строку utf8, в wchar_t.
