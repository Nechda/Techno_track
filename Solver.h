#pragma once

const int SE_INFTY = -1; /// Constant which returns when equation has infinite many roots.
const int SE_HAVE_EERRORS = -2; /// Returns when there is errors.
const double SE_ACCURACY = 1e-7; /// Default accuracy for all expressions.

const int SE_ERR_INVALID_INPUT_DATA = -1; /// Returns when one of the coficcients is NAN or INF. 
const int SE_ERR_NULL_POINTER = -2; /// Returns when NULL is adress for writing answer.
const int SE_ERR_SAME_POINTERS = -3;/// Returns when different variables have same pointers.
const int SE_ERR_INVALID_ACCURACY = -4; /// Returns when acurracy lower than DBL_MIN or equal NAN, INF
const int SE_ERR_DISCRIMINANT_FAILED = -5; /// Returns when have problems in calculating the discriminant

int SolveLinear(const double a, const double b, double* x, double accuracy = SE_ACCURACY);
int SolveSquare(const double a, const double b, const double c, double* x_1, double* x_2, double accuracy = SE_ACCURACY);