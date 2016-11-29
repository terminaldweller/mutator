
#include "testFuncs2.h"

static void test33(void)
{
	short int a;
	long int b;

	b = (int)a;
}

void testfunc1(void)
{
	unsigned char a;
	unsigned char b;

	b =  a;
}

testfunc2(void)
{
	int a;
	int b;
	int sum;

	sum = a + b;
}

void testfunc3()
{
	int a;
	int b;
	int c;

	/*do stuff*/
}

int testfunc6(void)
{
	int a;
	int b;
	//return ();
}

int testfunc7(void)
{
	int a;
	int b;
	//return;
}

int testfunc8(void)
{
	int a[10];
	int b[10];

	int* p;

	p = &a[0];

	int i;
	int sum;
	int sum2;
	int garbage;
	int garbage2;

	for (i = 0; i < 10; ++i)
	{
		sum += *(a + i);
		sum2 += *(p + i);
		//garbage = *(a - b);
	}

	for (i = 10; i < 1; i++)
	{
		sum += *(a - i);
		sum2 += *(p - i);
		//garbage2 = *(p - a);
	}
}