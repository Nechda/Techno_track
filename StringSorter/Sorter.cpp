#include "Sorter.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


int compare_wstr(const void* ptr1, const void* ptr2)
{
    Assert_c(ptr1 != NULL);
    Assert_c(ptr2 != NULL);

    if (ptr1 == NULL || ptr2 == NULL)
        return CMP_ERROR_NULLPTR;

    C_string s1 = *((C_string*)ptr1);
    C_string s2 = *((C_string*)ptr2);
    
    while (*s1 && (*s1 == *s2))
    {
        s1++;
        s2++;
    }
    
    int difference = (unsigned int)(*s1) - (unsigned int)(*s2);

    return difference > 0 ? 1 : difference< 0 ? -1 : 0;
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
    int number_empty_strings = 0;

    for (int i = 0; i < num_of_lines-number_empty_strings; i++)
    {

        fgetws(BUFF, LINE_MAX_BUFFER_LEN, file_in);

        arr[i] = wcschr(BUFF, L'\n');
        if (arr[i])
            *arr[i] = 0;
        unsigned int len_line = wcslen(BUFF);
        if (len_line == 0)
        {
            number_empty_strings++;
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

    int num_of_valid_lines = num_of_lines - number_empty_strings;
    int array_size = num_of_lines;

    if (num_of_valid_lines > 0)
    {
        C_string* ptr = (C_string*)realloc(arr, num_of_valid_lines * sizeof(C_string));

        arr = ptr == NULL ? arr : ptr;
        array_size = ptr == NULL ? num_of_lines : num_of_valid_lines;

        qsort(arr, num_of_valid_lines, sizeof(C_string), compare_wstr);
        
        for (int i = 0; i < num_of_valid_lines; i++)
            fprintf(file_out, "%d: %ls\n", i + 1, arr[i]);



    }


    for (int i = 0; i < array_size; i++)
        free(arr[i]);
    free(arr);

    fclose(file_in);
    fclose(file_out);
}