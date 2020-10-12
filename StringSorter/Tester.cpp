#include "Tester.h"
#include "Sorter.h"
#include "Converter.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <io.h>
#include <clocale>


/**
\brief Тестировщик компаратора строк из wchar_t
*/
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

static inline int getFileSize(const char* filename)///< вычисляет размер файла в байтах
{
    struct stat st;
    stat(filename, &st);
    return st.st_size;
}


/**
\brief Тестировщик конвертора из utf8 в wchar_t
\note  Функция считывает строку из файла "test.txt" двумя способами.
       Первый использует функцию read, затем коневертирует байты в wchar_t.
       Второй использует locale и считывает данные через fgetwc число символов,
       указанное в константе TESTER_MAX_BUFFER_LENGTH. Затем данные строки сравниваются.
       Сначала вычисляется длина каждой строки, если тест функция проходит,то дальше сравниваются
       сами элементы двух массивов.
*/
void testing_converter()///< Тестировщик конвертора из utf8 в wchar_t. 
{
    const char* filename = "test.txt";
    int errorCode = 0;
    
    int fileSize = getFileSize(filename);

    int fileDesc = open(filename, O_RDONLY);
    if (fileDesc == -1)
        return;

    char* data = (char*)calloc(fileSize + 1, sizeof(char));
    Assert_c(data != NULL);
    if (!data)
    {
        close(fileDesc);
        return;
    }
    data[fileSize] = 0;

    errorCode = read(fileDesc, data, fileSize);
    Assert_c(errorCode!=-1);
    if (errorCode == -1)
    {
        free(data);
        close(fileDesc);
        return;
    }

    int countLetters = utf8StrLen(data);

    wchar_t* wData = (wchar_t*)calloc(countLetters + 1, sizeof(wchar_t));
    wData[countLetters] = 0;
    Assert_c(wData != NULL);
    if (!wData)
    {
        free(data);
        close(fileDesc);
        return;
    }

    errorCode = utf8ToWchar(wData, data);
    Assert_c(errorCode != -1);
    if (errorCode == -1)
    {
        free(wData);
        free(data);
        close(fileDesc);
        return;
    }

    free(data);
    close(fileDesc);


    std::setlocale(LC_ALL, "en_US.utf8");
    FILE* inFile = fopen(filename, "r");
    wchar_t* ansData = (wchar_t*)calloc(TESTER_MAX_BUFFER_LENGTH+1, sizeof(wchar_t));
    Assert_c(ansData!=NULL);
    if (!ansData)
    {
        free(wData);
        fclose(inFile);
        return;
    }
    ansData[TESTER_MAX_BUFFER_LENGTH] = 0;
  
    wchar_t c = 0;
    int index = 0;
    while ((c = fgetwc(inFile)) != WEOF && index < TESTER_MAX_BUFFER_LENGTH)
        ansData[index++] = c;
    ansData[index] = 0;

    fclose(inFile);
    
    {
        printf("Start testing utf8 converter....\n");
        printf("Stage 1: compare length of string\n");
        int strLen_utf8 = wcslen(wData);
        int strLen_wchr = wcslen(ansData);
        if (strLen_utf8 != strLen_wchr)
            printf("Error occur!\n Len of string converted by utf8 is: %d, but should be %d\n", strLen_utf8, strLen_wchr);
        else
            printf("Test successfully completed!\n");
    }

    {
        printf("Stage 2: check data in string\n");
        if (!wcscmp(wData, ansData))
            printf("Test successfully completed!\n");
        else
            printf("Error occur!\nConverted string:\n%ls\nBut should be:\n%ls\n", wData, ansData);
    }
    free(wData);
    free(ansData);
    return;
}