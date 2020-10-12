#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <clocale>
#include "Sorter.h"
#include "Tester.h"
#include "Converter.h"


int main()
{
#ifdef NDEBUG
    testing_converter();
    testing_compare_wstr();
#endif
    sortFromFile("origin.txt", "remake1.txt", Direct);
    sortFromFile("origin.txt", "remake2.txt", Inverse);
    poermGenerator("origin.txt", "AbAbCCddEffEgg", "poem.txt", "AbAbCCddEffEgg",4);

    std::system("pause");
    return 0;
}