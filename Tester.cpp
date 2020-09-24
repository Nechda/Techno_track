//-------------------------------------------------------------------------------------
//!\file
//!\brief This file contains function for testing SolveLinear & SolveSquare
//-------------------------------------------------------------------------------------

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include "Tester.h"
#include "Solver.h"



//-------------------------------------------------------------------------------------
//! \brief Shows detailized information about error code
//-------------------------------------------------------------------------------------
void SE_DecodeError()
{
    printf("Decoded error: ");
    fflush(stdout);
    switch (errno)
    {
    case SE_ERR_INVALID_INPUT_DATA:
        printf("INVALID INPUT DATA\n");
        break;
    case SE_ERR_NULL_POINTER:
        printf("NULL POINTER\n");
        break;
    case SE_ERR_SAME_POINTERS:
        printf("SAME POINTERS\n");
        break;
    case SE_ERR_INVALID_ACCURACY:
        printf("INVALID ACCURACY\n");
        break;
    case SE_ERR_DISCRIMINANT_FAILED:
        printf("CAN'T EXPRESS DISCRIMINANT\n");
        break;
    default:
        printf("undefined error code...\n");
        break;
    }
}

//-------------------------------------------------------------------------------------
//! \brief Structure for SolveLinear function's input data 
//-------------------------------------------------------------------------------------
struct Input_SL 
{
    double a;
    double b;
    double* x;
    double accuracy;
};

//-------------------------------------------------------------------------------------
//! \brief Structure for SolveLinear function's ouput data 
//-------------------------------------------------------------------------------------
struct Output_SL
{
    int nRoots;
    int error;
    double root;
};

//-------------------------------------------------------------------------------------
//! \brief Structure for SolveLinear function's input&output data 
//-------------------------------------------------------------------------------------
struct Collection_SL
{
    Input_SL input;
    Output_SL output;
};


//-------------------------------------------------------------------------------------
//! \brief Compare structures
//-------------------------------------------------------------------------------------
bool static inline isEqual(const Output_SL a, const Output_SL b)
{
    return  a.error == b.error &&
            a.nRoots == b.nRoots &&
            fabs(a.root - b.root) < SE_ACCURACY ?
            1 : 0;
}

//-------------------------------------------------------------------------------------
//! \brief Tester for SolveLinear function
//-------------------------------------------------------------------------------------
void Testing_SolveLinear()
{
    double x = 0;
    int nRoots = 0;
    Collection_SL data[] = {
        {{ 1,                    1,            &x,        SE_ACCURACY },      {    1,                  0,                           -1 }},
        {{ 0,                    1,            &x,        SE_ACCURACY },      {    0,                  0,                            0 }},
        {{ SE_ACCURACY/2.0,      0,            &x,        SE_ACCURACY },      {    SE_INFTY,           0,                            0 }},
        {{ 1,                    NAN,          &x,        SE_ACCURACY },      {    SE_HAVE_EERRORS,    SE_ERR_INVALID_INPUT_DATA,    0 }},
        {{ 1,                    1,            NULL,      SE_ACCURACY },      {    SE_HAVE_EERRORS,    SE_ERR_NULL_POINTER,          0 }},
        {{ 1,                    1,            &x,        0           },      {    SE_HAVE_EERRORS,    SE_ERR_INVALID_ACCURACY,      0 }}
    };

    printf("========Testing function \"SolveLinear========\n");

    Output_SL answ;
    for (int i = 0; i < sizeof(data) / sizeof(Collection_SL); i++)
    {
        printf("Start test %d\n", i + 1);
        printf("In: a = %.4f, b = %.4f, \&x = 0x%x, accuracy = %e\n",
                data[i].input.a, data[i].input.b, data[i].input.x, data[i].input.accuracy);

        nRoots = SolveLinear(
                data[i].input.a, data[i].input.b, data[i].input.x, data[i].input.accuracy);
        answ.nRoots = nRoots;
        answ.error = nRoots == SE_HAVE_EERRORS ? errno : 0;
        answ.root = x;

        printf("Our     out: nRoots = %d, error = %d, root = %.4f\n",
                answ.nRoots, answ.error, answ.root);
        printf("Correct out: nRoots = %d, error = %d, root = %.4f\n",
                data[i].output.nRoots, data[i].output.error, data[i].output.root);

        if (isEqual(answ, data[i].output))
        {
            if (answ.error)
                SE_DecodeError();
            printf("Test %d complited successful!\n\n", i + 1);
        }
        else
            printf("We have problems...\n\n");
    }
}


//-------------------------------------------------------------------------------------
//! \brief Structure for SolveSquare function's input data 
//-------------------------------------------------------------------------------------
struct Input_SS
{
    double a;
    double b;
    double c;
    double* x1;
    double* x2;
    double accuracy;
};

//-------------------------------------------------------------------------------------
//! \brief Structure for SolveSquare function's ouput data 
//-------------------------------------------------------------------------------------
struct Output_SS
{
    int nRoots;
    int error;
    double root1;
    double root2;
};


//-------------------------------------------------------------------------------------
//! \brief Structure for SolveSquare function's input&output data 
//-------------------------------------------------------------------------------------
struct Collection_SS
{
    Input_SS input;
    Output_SS output;
};

//-------------------------------------------------------------------------------------
//! \brief Compare structures
//-------------------------------------------------------------------------------------
bool static inline isEqual(Output_SS a, Output_SS b)
{
    return  a.error == b.error &&
            a.nRoots == b.nRoots &&
            fabs(a.root1 - b.root1) < SE_ACCURACY &&
            fabs(a.root2 - b.root2) < SE_ACCURACY ?
            1 : 0;
}

//-------------------------------------------------------------------------------------
//! \brief Tester for SolveSquare function
//-------------------------------------------------------------------------------------
void Testing_SolveSquare()
{
    double x1 = 0, x2 = 0;
    int nRoots = 0;
    Collection_SS data[] = {
        {{    1,                    2,                1,    &x1,&x2,    SE_ACCURACY },    { 1,                  0,                           -1,0}},
        {{    1,                    0,                0,    &x1,&x2,    SE_ACCURACY },    { 1,                  0,                            0,0}},
        {{    0,                    0,                0,    &x1,&x2,    SE_ACCURACY },    { SE_INFTY,           0,                            0,0}},
        {{    SE_ACCURACY/2.0,      0,                0,    &x1,&x2,    SE_ACCURACY },    { SE_INFTY,           0,                            0,0}},
        {{    1,                    2,                3,    NULL,NULL,  SE_ACCURACY },    { SE_HAVE_EERRORS,    SE_ERR_NULL_POINTER,          0,0}},
        {{    1,                    NAN,              3,    &x1,&x2,    SE_ACCURACY },    { SE_HAVE_EERRORS,    SE_ERR_INVALID_INPUT_DATA,    0,0}},
        {{    1,                    2,                3,    &x1,&x1,    SE_ACCURACY },    { SE_HAVE_EERRORS,    SE_ERR_SAME_POINTERS,         0,0}},
        {{    1,                    2,                3,    &x1,&x2,    0           },    { SE_HAVE_EERRORS,    SE_ERR_INVALID_ACCURACY,      0,0}},
        {{    1,                    DBL_MAX/2.0,      1,    &x1,&x2,    SE_ACCURACY },    { SE_HAVE_EERRORS,    SE_ERR_DISCRIMINANT_FAILED,   0,0}}
    };

    printf("========Testing function \"SolveSquare========\n");

    Output_SS answ;
    for (int i = 0; i < sizeof(data) / sizeof(Collection_SS); i++)
    {
        printf("Start test %d\n", i + 1);
        printf("In: a = %.4f, b = %.4f, c = %.4f, \&x1 = 0x%x, \&x2 = 0x%x, accuracy = %e\n", 
                data[i].input.a, data[i].input.b, data[i].input.c, data[i].input.x1, data[i].input.x2, data[i].input.accuracy);

        nRoots = SolveSquare(
                data[i].input.a, data[i].input.b, data[i].input.c, data[i].input.x1, data[i].input.x2, data[i].input.accuracy);
        answ.nRoots = nRoots;
        answ.error = nRoots == SE_HAVE_EERRORS ? errno : 0;
        answ.root1 = x1;
        answ.root2 = x2;

        printf("Our     out: nRoots = %d, error = %d, root1 = %.4f, root2 = %.4f\n",
                answ.nRoots, answ.error, answ.root1, answ.root2);
        printf("Correct out: nRoots = %d, error = %d, root1 = %.4f, root2 = %.4f\n",
                data[i].output.nRoots, data[i].output.error, data[i].output.root1, data[i].output.root2);

        if (isEqual(answ, data[i].output))
        {
            if (answ.error)
                SE_DecodeError();
            printf("Test %d complited successful!\n\n", i + 1);
        }
        else
            printf("We have problems...\n\n");
    }
}