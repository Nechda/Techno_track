#include <iostream>
#include <string.h>
#include <stdio.h>
#include <wchar.h>
#include <stdlib.h>
#include <assert.h>

#define NDEBUG

#ifdef NDEBUG
#define Assert_c(expr) if(!(expr))printf("Expression %ls is false.\n In file: %ls\n line: %d\n",_CRT_WIDE(#expr), _CRT_WIDE(__FILE__), (unsigned)(__LINE__)); ///< Реализация assert для релиза
#else
#define Assert_c(expr) assert(expr); ///< Реализация assert для отладки переключить режим можно директивой #define NDEBUG
#endif




const int LINE_MAX_BUFFER_LEN = 100; ///< Максимальная длина буфера в функции sortStringsFromFile(...)

typedef wchar_t* C_string; ///< Строка из расширенных символов

/**
\brief Функция сравнения двух строк
\param [in]   a    Первая строка
\param [in]   b    Вторая строка
\retrun Возвращает положительное число, если первая строка идет позже (в смысле лексиграфического порядка) чем вторая строка.
Соответсвенно возвращает отрицательное, если вторая строка идет позже первой, и ноль в случае совпадания строк.

\note Это просто обертка функции wcscmp(const wchar_t* first,const wchar_t* second) 
*/
int compare(const void* a, const void* b)
{
    C_string first = *((C_string*)a);
    C_string second = *((C_string*)b);
    return wcscmp(first, second);
}

const int GL_ERROR_NULLPTR = -1; ///< Возвращается, если в функцию getLines(...) передели нулевой указатель
const int GL_ERROR_ACCESS_FAIL = -2; ///< Возвращается, если при открытии файла в функции getLines(...) возникала ошибка

/**
\brief Функция подсчета строк в файле
\param [in]   filename    Имя файла, в котором требуется подсчитать количество строк
\return Количество строк в файле или код ошибки.
*/
int getLines(const char* filename)
{
    Assert_c(filename != NULL);
    if (filename == NULL)
        return GL_ERROR_NULLPTR;

    FILE* file = fopen(filename, "r");
    Assert_c(file != NULL);
    if (file == NULL)
        return GL_ERROR_ACCESS_FAIL;


    int lines = 1;
    wchar_t w = 0;
    while ((w = fgetwc(file)) != WEOF)
        if (w == L'\n')
            lines++;

    fclose(file);
    return lines;
}


/**
\brief Функция сортировки строк
\details Данная функция сортирует строки в лексиграфическом порядке. На вход подаются две строки, содержащие имена файлов. Из одного файла
будут считываться данные, во второй записываться результат.
\param [in]    in_filename    Имя файла, из которго будут считываться строки
\param [in]    out_filename   Имя файла, в который будет записываться результат сортировки
\return Функция ничего не возвращает.
*/
void sortStringsFromFile(const char* in_filename, const char* out_filename)
{
    Assert_c(in_filename != NULL);
    Assert_c(out_filename != NULL);
    if (in_filename == NULL || out_filename == NULL)
        return;

    FILE* file_in = fopen(in_filename, "r");
    FILE* file_out = fopen(out_filename, "w");

    Assert_c(file_in != NULL);
    Assert_c(file_out != NULL);
    if (file_in == NULL || file_out == NULL)
    {
        if (file_in)
            fclose(file_in);
        if (file_out)
            fclose(file_out);
        return;
    }

    int num_of_lines = getLines(in_filename);
    Assert_c(num_of_lines >= 0);
    if (num_of_lines < 0)
    {
        if (file_in)
            fclose(file_in);
        if (file_out)
            fclose(file_out);
        return;
    }

    C_string* arr = (C_string*)malloc(sizeof(C_string)*num_of_lines);
    Assert_c(arr != NULL);
    if (arr == NULL)
    {
        fclose(file_in);
        fclose(file_out);
        return;
    }

    for (int i = 0; i < num_of_lines; i++)
    {
        arr[i] = (C_string)malloc(sizeof(wchar_t)*LINE_MAX_BUFFER_LEN);
        Assert_c(arr[i] != NULL);
        if (arr[i] == NULL)
        {
            fclose(file_in);
            fclose(file_out);
            for (int j = 0; j < i; j++)
                free(arr[j]);
            free(arr);
            return;
        }
    }

    int empty_lines = 0;
    wchar_t BUFF[LINE_MAX_BUFFER_LEN] = { 0 };
    for (int i = 0; i < num_of_lines; i++)
    {
        
        fgetws(BUFF, LINE_MAX_BUFFER_LEN, file_in);
        wchar_t* ptr = wcschr(BUFF, L'\n');
        if (ptr)
            *ptr = 0;
        unsigned int len_line = wcslen(BUFF);
        ptr = (C_string)realloc(arr[i], (len_line+1)*sizeof(wchar_t));

        
        Assert_c(ptr != NULL);
        if (ptr == NULL)
        {
            fclose(file_in);
            fclose(file_out);
            for (int j = 0; j < num_of_lines; j++)
                if(j != i)
                    free(arr[j]);
            free(arr);
            return;
        }

        arr[i] = ptr;

        wcsncpy(arr[i], BUFF, len_line+1);
    }

    qsort(arr, num_of_lines, sizeof(C_string), compare);

    for (int i = 0; i < num_of_lines; i++)
        fprintf(file_out, "%d: %ls\n", i+1, arr[i]);

    for (int i = 0; i < num_of_lines; i++)
        free(arr[i]);
    free(arr);

    fclose(file_in);
    fclose(file_out);
}


int main()
{
    sortStringsFromFile("origin.txt", "remake.txt");
    std::system("pause");
    return 0;
}