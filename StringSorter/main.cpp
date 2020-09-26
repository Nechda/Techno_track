#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include "Sorter.h"
#include "Tester.h"


int main()
{
#ifndef NDEBUG
    testing_compare_wstr();
    testing_getNumberLines();
#endif

    sortStringsFromFile("origin.txt", "remake.txt");
    std::system("pause");
    return 0;
}