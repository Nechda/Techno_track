#include "Tester.h"
#include "Sorter.h"
#include <stdio.h>

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
        answ = compare_wstr(&arr[i].first, &arr[i].second);
        answ = answ > 0 ? L'>' : answ < 0 ? L'<' : L'=';
        if ((wchar_t)answ == arr[i].result)
            printf("Success!\n");
        else
            printf("\n\n______________Wrong answer!______________\n\n");
    }

}

void testing_getNumberLines()
{
    const char* temp_file_name = "count_line_test.txt";
    FILE* file = fopen(temp_file_name, "w");
    Assert_c(file != NULL);
    if (file == NULL)
        return;
    int number_of_lines = 0;
    for (int i = 0; i < 50; i++)
    {
        printf("Add to count_line_test.txt new line\n");
        fprintf(file, "%d\n", i + 1);
        fflush(file);
        printf("Total lines: %d\n", i + 2);

        printf("Try to count lines by getNumberLines(...)\n");
        number_of_lines = getNumberLines("count_line_test.txt");
        printf("Number of counted lines: %d\n", number_of_lines);
        if (number_of_lines == i + 2)
            printf("Success!\n");
        else
            printf("\n\n______________Wrong answer!______________\n\n");
    }
    fclose(file);
    if (!remove(temp_file_name))
        printf("Temp file is deleted successfully!\n");
    else
        printf("We can't delete temp file!\n");

}