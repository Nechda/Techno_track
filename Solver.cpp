#include "Solver.h"
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>


#define NDEBUG
#include <assert.h>


//-------------------------------------------------------------------------------------
//! \brief Compares value with 0 in current accuracy
//!
//! \param [in]   t         Rounding variable
//! \param [in]   accuracy  Accuracy of rounding to zero
//!
//! \return Next to a variable with zero or not.
//!
//! \note Accuracy should be gain than DBL_MIN constant.
//!       
//-------------------------------------------------------------------------------------
static inline bool isZero(const double t, const double accuracy)
{
	assert(std::isfinite(t));
	assert(accuracy > DBL_MIN);
	return fabs(t) < accuracy ? 1 : 0;
}

//-------------------------------------------------------------------------------------
//! \brief Solves a linear equation ax + b = 0
//!
//! \param [in]   a         a-coefficient
//! \param [in]   b         b-coefficient
//! \param [out]  x         Pointer to the root
//! \param [in]   accuracy  Accuracy of rounding to zero, by default equals to 1e-7
//!
//! \return Number of roots.
//! 
//! \note Function demands allocated memory for pointer x.
//!       In case of infinite number of roots, returns SE_INFTY.
//!       Accuracy should be gain than DBL_MIN constant.
//!       In case of error, function returns SE_HAVE_EERRORS. The code of error stored in errno variable.
//! Example of usage:
//! \code
//!int main()
//!{
//!	printf("Enter the coefficients of linear equation ax+b = 0\n");
//!	double a = 0, b = 0;
//!	scanf("%lg %lg", &a, &b);
//!
//!	double x = 0;
//!	int nRoots = SolveLinear(a, b, &x, 1e-6);
//!
//!	switch (nRoots)
//!	{
//!		case 0:
//!			printf("There aren't any roots\n");
//!			break;
//!		case 1:
//!			printf("There is a root: x = %.4f\n", x1);
//!			break;
//!		case SE_INFTY:
//!			printf("Any number is a root of current equation.\n");
//!			break;
//!		default:
//!			printf("Strange number of roots... \n");
//!			break;
//!	}
//!
//!	return 0;
//!}
//! \endcode
//!    

//-------------------------------------------------------------------------------------
int SolveLinear(const double a, const double b, double* x, double accuracy)
{


	assert(std::isfinite(a));
	assert(std::isfinite(b));
	assert(std::isfinite(accuracy));
	assert(x != NULL);
	assert(accuracy > DBL_MIN);

#ifdef NDEBUG
	if (!std::isfinite(a) || !std::isfinite(b))
	{
		errno = SE_ERR_INVALID_INPUT_DATA;
		return SE_HAVE_EERRORS;
	}
	if (!std::isfinite(accuracy) || accuracy < DBL_MIN)
	{
		errno = SE_ERR_INVALID_ACCURACY;
		return SE_HAVE_EERRORS;
	}
	if (x == NULL)
	{
		errno = SE_ERR_NULL_POINTER;
		return SE_HAVE_EERRORS;
	}
#endif


	*x = 0;

	if (isZero(a, accuracy))
		return isZero(b, accuracy) ? SE_INFTY : 0;

	*x = -b / a;
	return 1;
}

//-------------------------------------------------------------------------------------
//! \brief Solves a square equation ax^2 + bx + c = 0
//!
//! \param [in]   a         a-coefficient
//! \param [in]   b         b-coefficient
//! \param [in]   c         c-coefficient
//! \param [out]  x_1       Pointer to the 1st root
//! \param [out]  x_2       Pointer to the 2nd root
//! \param [in]   accuracy  Accuracy of rounding to zero, by default equals to 1e-7
//!
//! \return Number of roots
//! 
//! \note Function demands allocated memory for both pointers x_1,x_2.
//!       In case of infinite number of roots, returns SE_INFTY.
//!       Accuracy should be gain than DBL_MIN constant.
//!       In case of error, function returns SE_HAVE_EERRORS. The code of error stored in errno variable.
//!
//! Example of usage:
//! \code
//!int main()
//!{
//!	printf("Enter the coefficients of square equation ax^2+bx+c = 0\n");
//!	double a = 0, b = 0, c = 0;
//!	scanf("%lg %lg %lg", &a, &b, &c);
//!
//!	double x1 = 0, x2 = 0;
//!	int nRoots = SolveSquare(a, b, c, &x1, &x2, 1e-6);
//!
//!	switch (nRoots)
//!	{
//!		case 0:
//!			printf("There aren't any roots\n");
//!			break;
//!		case 1:
//!			printf("There is a root: x_1 = %.4f\n", x1);
//!			break;
//!		case 2:
//!			printf("There is two roots: x_1 = %.4f, x_2 = %.4f \n", x1, x2);
//!			break;
//!		case SE_INFTY:
//!			printf("Any number is a root of current equation.\n");
//!			break;
//!		default:
//!			printf("Strange number of roots... \n");
//!			break;
//!	}
//!
//!	return 0;
//!}
//! \endcode
//!       
//-------------------------------------------------------------------------------------
int SolveSquare(const double a, const double b, const double c, double* x_1, double* x_2, double accuracy)
{
	assert(std::isfinite(a));
	assert(std::isfinite(b));
	assert(std::isfinite(c));
	assert(std::isfinite(accuracy));
	assert(x_1 != x_2);
	assert(x_1 != NULL);
	assert(x_2 != NULL);
	assert(accuracy > DBL_MIN);

#ifdef NDEBUG
	if (!std::isfinite(a) || !std::isfinite(b) || !std::isfinite(c))
	{
		errno = SE_ERR_INVALID_INPUT_DATA;
		return SE_HAVE_EERRORS;
	}
	if (!std::isfinite(accuracy) || accuracy < DBL_MIN)
	{
		errno = SE_ERR_INVALID_ACCURACY;
		return SE_HAVE_EERRORS;
	}
	if (x_1 == NULL || x_2 == NULL)
	{
		errno = SE_ERR_NULL_POINTER;
		return SE_HAVE_EERRORS;
	}
	if (x_1 == x_2)
	{
		errno = SE_ERR_SAME_POINTERS;
		return SE_HAVE_EERRORS;
	}
#endif

	*x_1 = 0;
	*x_2 = 0;

	if (isZero(a, accuracy))
		return SolveLinear(b, c, x_1, accuracy);

	if (isZero(c, accuracy))
	{
		SolveLinear(a, b, x_2, accuracy);
		return isZero(*x_1 - *x_2, accuracy) ? 1 : 2;
	}

	double D = b*b;
	assert(std::isfinite(D));
	D -= 4 * a*c;
	assert(std::isfinite(D));

#ifdef NDEBUG
	if (!std::isfinite(D))
	{
		errno = SE_ERR_DISCRIMINANT_FAILED;
		return SE_HAVE_EERRORS;
	}
#endif

	if (D > accuracy)
	{
		D = sqrt(D);
		*x_1 = *x_2 = -b;

		*x_1 += D;
		*x_2 -= D;

		*x_1 /= 2 * a;
		*x_2 /= 2 * a;

		return 2;
	}

	if (isZero(D, accuracy))
	{
		*x_1 = -b / (2 * a);
		return 1;
	}

	if (D < accuracy)
		return 0;
}