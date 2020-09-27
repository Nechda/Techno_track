#pragma once
#include <wchar.h>
#include <assert.h>

#define NDEBUG

#ifdef NDEBUG
#define Assert_c(expr) if(!(expr))printf("Expression %ls is false.\n In file: %ls\n line: %d\n",_CRT_WIDE(#expr), _CRT_WIDE(__FILE__), (unsigned)(__LINE__)); ///< Реализация assert для релиза
#else
#define Assert_c(expr) assert(expr); ///< Реализация assert для отладки переключить режим можно директивой #define NDEBUG
#endif

const int LINE_MAX_BUFFER_LEN = 100; ///< Максимальная длина буфера в функции sortStringsFromFile(...)

typedef wchar_t* C_string; ///< Строка из расширенных символов


const int CMP_ERROR_NULLPTR = -2;///< Хранится в errno, если в функцию compare_wstr(...) передели нулевой указатель

/**
\brief Функция сравнения двух строк
\param [in]   ptr1    Первая строка
\param [in]   ptr2    Вторая строка
\retrun Возвращает положительное число, если первая строка идет позже (в смысле лексиграфического порядка) чем вторая строка.
Соответсвенно возвращает отрицательное, если вторая строка идет позже первой, и ноль в случае совпадания строк.

\note Это просто обертка функции wcscmp(const wchar_t* first,const wchar_t* second)
*/
int compare_wstr(const void* ptr1, const void* ptr2);

const int GL_ERROR_NULLPTR = -1; ///< Возвращается, если в функцию getLines(...) передели нулевой указатель
const int GL_ERROR_ACCESS_FAIL = -2; ///< Возвращается, если при открытии файла в функции getLines(...) возникала ошибка


/**
\brief Функция подсчета непустых строк в файле
\param [in]   filename    Имя файла, в котором требуется подсчитать количество строк
\return Количество строк в файле или код ошибки.
*/
int getNumberLines(const char* filename);

/**
\brief Функция сортировки строк
\details Данная функция сортирует непустые строки в лексиграфическом порядке. На вход подаются две строки, содержащие имена файлов. Из одного файла
будут считываться данные, во второй записываться результат.
\param [in]    in_filename    Имя файла, из которго будут считываться строки
\param [in]    out_filename   Имя файла, в который будет записываться результат сортировки
\return Функция ничего не возвращает.
*/
void sortStringsFromFile(const char* in_filename, const char* out_filename);