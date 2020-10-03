#pragma once
#include <wchar.h>
#include <assert.h>

#define NDEBUG

#ifdef NDEBUG
#define Assert_c(expr) if(!(expr))printf("Expression %ls is false.\n In file: %ls\n line: %d\n",_CRT_WIDE(#expr), _CRT_WIDE(__FILE__), (unsigned)(__LINE__)); ///< Реализация assert для релиза
#else
#define Assert_c(expr) assert(expr); ///< Реализация assert для отладки переключить режим можно директивой #define NDEBUG
#endif

enum Direction {Direct = 1, Inverse = 0}; ///< Направление сравнения строк


typedef wchar_t* C_string; ///< Строка из расширенных символов



const int SS_ERROR_NULLPTR = -1; ///< Возвращается, если по ходу работы функции возникает нулевой указатель
const int SS_ERROR_ACCESS_FAIL = -2; ///< Возвращается, если при ошибке доступа к файлу
const int SS_INVALID_DATA = -3; ///< Возвращается, если переданные параметры не входят в область доустимых


int getNumberLines(const char* filename);
int cmpWstr(const void* ptr1, const void* ptr2, int inc = 1);
void sortFromFile(const char* in_filename, const char* out_filename, Direction dir = Direct);
void poermGenerator(const char* in_filename, const char* in_syllable, const char* out_filename, const char* out_syllable, const int number_Stanaza = 1);