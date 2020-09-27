#include "Sorter.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>



/**
\brief Функция сравнения двух строк
\param [in]   ptr1    Первая строка
\param [in]   ptr2    Вторая строка
\retrun Возвращает положительное число, если первая строка идет позже (в смысле лексиграфического порядка) чем вторая строка.
Соответсвенно возвращает отрицательное, если вторая строка идет позже первой, и ноль в случае совпадания строк.

\note Это просто обертка функции wcscmp(const wchar_t* first,const wchar_t* second)
*/
int compare_wstr(const void* ptr1, const void* ptr2)
{
    Assert_c(ptr1 != NULL);
    Assert_c(ptr2 != NULL);

    if (ptr1 == NULL || ptr2 == NULL)
    {
        errno = CMP_ERROR_NULLPTR;
        return 0;
    }

    C_string s1 = *((C_string*)ptr1);
    C_string s2 = *((C_string*)ptr2);
    while (*s1 && (*s1 == *s2))
        s1++, s2++;
    
    errno = 0;
    return (unsigned int)(*s1) - (unsigned int)(*s2);
}


/**
\brief Функция подсчета непустых строк в файле
\param [in]   filename    Имя файла, в котором требуется подсчитать количество строк
\return Количество строк в файле или код ошибки.
*/
int getNumberLines(const char* filename)
{
    Assert_c(filename != NULL);
    if (filename == NULL)
        return GL_ERROR_NULLPTR;

    FILE* file = fopen(filename, "r");
    Assert_c(file != NULL);
    if (file == NULL)
        return GL_ERROR_ACCESS_FAIL;


    int lines = 1;
    wchar_t w[2] = {0,0};
    bool pingpong = 0;
    while ((w[pingpong] = fgetwc(file)) != WEOF)
    {
        if (w[pingpong] == L'\n' & w[pingpong^1] != L'\n')
            lines++;
        pingpong ^= 1;
    }

    fclose(file);
    return lines;
}

/**
\brief Функция сортировки строк
\details Данная функция сортирует непустые строки в лексиграфическом порядке. На вход подаются две строки, содержащие имена файлов. Из одного файла
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

    int num_of_lines = getNumberLines(in_filename);
    Assert_c(num_of_lines > 0);
    if (num_of_lines <= 0)
    {
        fclose(file_in);
        fclose(file_out);
        return;
    }

    C_string* arr = (C_string*)calloc(num_of_lines, sizeof(C_string));
    Assert_c(arr != NULL);
    if (arr == NULL)
    {
        fclose(file_in);
        fclose(file_out);
        return;
    }


    wchar_t BUFF[LINE_MAX_BUFFER_LEN] = L"";
    for (int i = 0; i < num_of_lines; i++)
    {

        fgetws(BUFF, LINE_MAX_BUFFER_LEN, file_in);

        arr[i] = wcschr(BUFF, L'\n');
        if (arr[i])
            *arr[i] = 0;
        unsigned int len_line = wcslen(BUFF);
        if (len_line == 0)
        {
            i--;
            continue;
        }

        arr[i] = (C_string)calloc((len_line + 1), sizeof(wchar_t));
        Assert_c(arr[i] != NULL);
        if (arr[i] == NULL)
        {
            for (int j = 0; j < i; j++)
                free(arr[i]);
            free(arr);
            fclose(file_in);
            fclose(file_out);
            return;
        }

        wcsncpy(arr[i], BUFF, len_line + 1);
    }

    qsort(arr, num_of_lines, sizeof(C_string), compare_wstr);
        
    for (int i = 0; i < num_of_lines; i++)
        fprintf(file_out, "%d: %ls\n", i + 1, arr[i]);


    for (int i = 0; i < num_of_lines; i++)
        free(arr[i]);
    free(arr);

    fclose(file_in);
    fclose(file_out);
}