//-------------------------------------------------------------------------------------
//!\file
//!\brief The main file of project
//!
//!
//!This file consists main functions which demands for solving square equation.
//-------------------------------------------------------------------------------------

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include "Solver.h"
#include "Tester.h"

#define NDEBUG


int main()
{
    #ifdef NDEBUG
        Testing_SolveLinear();
        Testing_SolveSquare();
    #endif

    
    printf("Enter the coefficients of square equation ax^2+bx+c = 0\n");
    printf("Example: 10.4 0.2 5.95\n");
    printf("Coefficients: "); 
    fflush(stdout);

    double a = 0, b = 0, c = 0;
    scanf("%lg %lg %lg", &a, &b, &c);

    double x1 = 0, x2 = 0;
    int nRoots = SolveSquare(a, b, c, &x1, &x2, 1e-6);

    switch (nRoots)
    {
    case 0:
        printf("There aren't any roots\n");
        break;
    case 1:
        printf("There is a root: x_1 = %.4f\n", x1);
        break;
    case 2:
        printf("There is two roots: x_1 = %.4f, x_2 = %.4f \n", x1, x2);
        break;
    case SE_INFTY:
        printf("Any number is a root of current equation.\n");
        break;
    case SE_HAVE_EERRORS:
        SE_DecodeError();
        break;
    default:
        printf("DED\n");
        break;
    }
    
    std::system("pause");
    return 0;
}