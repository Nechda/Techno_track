#include "Tester.h"
#include "Sorter.h"
#include "Converter.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <io.h>

void testing_compare_wstr()
{
    struct Data
    {
        C_string first;
        wchar_t result;
        C_string second;
        
    };
   const Data arr[] =
    {
        { L"1", L'<', L"2" },
        { L"A", L'<', L"Z" },
        { L"a", L'<', L"z" },
        { L"7", L'>', L"2" },
        { L"H", L'>', L"E"},
        { L"s", L'>', L"d"},
        { L"Авокадо", L'<', L"Аметист"},
        { L"Авокадо", L'<', L"авокадо"},
        { L"Авокадо", L'<', L"Яблоко"},
        { L"Котик", L'=', L"Котик"}
    };
    int answ = 0;
    for (int i = 0; i < sizeof(arr) / sizeof(Data); i++)
    {
        printf("Start test %d\n", i + 1);
        answ = cmpWstr(&arr[i].first, &arr[i].second);
        answ = answ > 0 ? L'>' : answ < 0 ? L'<' : L'=';
        if ((wchar_t)answ == arr[i].result)
            printf("Success!\n");
        else
            printf("\n\n______________Wrong answer!______________\n\n");
    }

}


void testing_converter()
{
    const char* filename = "test.txt";

    int fileSize = 0;
    {
        struct stat st;
        stat(filename, &st);
        fileSize = st.st_size;
    }

    int filedesc = open(filename, O_RDONLY);
    if (filedesc == -1)
        return;

    char* data = (char*)calloc(fileSize + 1, sizeof(char));
    Assert_c(data != NULL);
    if (!data)
        return;
    data[fileSize] = 0;

    read(filedesc, data, fileSize);

    int countLetters = utf8StrLen(data);
    printf("Toltal letters: %d\n", countLetters);

    wchar_t* wdata = (wchar_t*)calloc(countLetters + 1, sizeof(wchar_t));
    wdata[countLetters] = 0;
    Assert_c(wdata != NULL);
    if (!wdata)
        return;
    utf8ToWchar(wdata, data);
    printf("Converted string:%ls\n", wdata);


    free(data);
    free(wdata);
    close(filedesc);

}