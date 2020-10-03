#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include "Sorter.h"
#include "Tester.h"




int main()
{
    /*
#ifdef NDEBUG
    testing_compare_wstr();
    testing_getNumberLines();
#endif
*/

    sortFromFile("origin.txt", "remake1.txt", Direct);
    sortFromFile("origin.txt", "remake2.txt", Inverse);
    poermGenerator("origin.txt", "AbAbCCddEffEgg", "poem.txt", "AbAbCCddEffEgg",10);

    std::system("pause");
    return 0;
}