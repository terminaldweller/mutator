
#include "testFuncs3.h"
#include <complex.h>


int intarray3[3][2] = MACRO1;
int intarray4[3][2] = MACRO2;
int answer = ANSWER;

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

void tddfunc2(void)
{
	static int staticint1;

	unsigned short int port = 0x5aU;
	unsigned short int result_8;
	//uint16_t result_16;
	//uint16_t mode;
	result_8 = (~port) >> 4;
}

void tddfunc3(void)
{
	int a = 1;

	READ_TIME_32();
	READ_TIME_33();
	READ_TIME_34();
}