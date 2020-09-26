#include "Sorter.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int compare_wstr(const void* a, const void* b)
{
    C_string first = *((C_string*)a);
    C_string second = *((C_string*)b);
    return wcscmp(first, second);
}

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
    wchar_t w = 0;
    while ((w = fgetwc(file)) != WEOF)
        if (w == L'\n')
            lines++;

    fclose(file);
    return lines;
}


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

    wchar_t BUFF[LINE_MAX_BUFFER_LEN] = { 0 };
    for (int i = 0; i < num_of_lines; i++)
    {

        fgetws(BUFF, LINE_MAX_BUFFER_LEN, file_in);
        /// А нужна ли вообще эта проверка?
        /*
        Assert_c(!ferror(file_in));
        if(ferror(file_in))
        {
        for (int i = 0; i < num_of_lines; i++)
        free(arr[i]);
        free(arr);

        fclose(file_in);
        fclose(file_out);
        return;
        }
        */


        wchar_t* ptr = wcschr(BUFF, L'\n');
        if (ptr)
            *ptr = 0;
        unsigned int len_line = wcslen(BUFF);
        ptr = (C_string)realloc(arr[i], (len_line + 1) * sizeof(wchar_t));


        Assert_c(ptr != NULL);
        arr[i] = ptr == NULL ? arr[i] : ptr;

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