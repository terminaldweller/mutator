
#include "testFuncs3.h"
#include <complex.h>

void tddfunc1 (void)
{
	double complex z1 = 10.0 + 10.0 * I;
	double complex z2 = 1.0 - 4.0 * I;

	float complex z3;

	int complex iz1;
	long int complex iz2;
	unsigned int complex iz3;
	signed int complex iz4;
	signed int n1;
	unsigned int n2;

	z3 = z2;
	z2 = z3;

	iz1 = iz2;
	iz2 = iz1;

	iz3 = iz4;
	iz2 = iz3;

	n1 = n2;
}