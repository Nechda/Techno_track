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
    sort_FromFile("origin.txt", "remake1.txt",Direct);
    sort_FromFile("origin.txt", "remake2.txt",Inverse);
    std::system("pause");
    return 0;
}