
/*first line intentionally left blank*/
/*********************************************************************************************************************/
/*inclusion directives*/
#include "testFuncs2.h"
#if 0
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <signal.h>
#include <time.h>
#endif
/*********************************************************************************************************************/
/*macro and definitions*/
typedef const* INTPTR;

#define XMACRO Y

#define ABSOLUTE(X) (((X) >= 0) ? (X) : -(X))
#define ABSOLUTE2(X) ((X >= 0) ? X : -X)
#define ABSOLUTE3(XMACRO) (((XMACRO) <= 0) ? (XMACRO) : -(XMACRO))
#define MINUS(X,Y) ((X) - (Y))

#define LOCOLUPO
#undef LOCOLUPO
/*********************************************************************************************************************/
/*Globals*/

/*********************************************************************************************************************/
/*functions go here.*/
static void test33(void)
{

	short int a;
	long int b;

	MINUS(a, b);
	//MINUS(a);

	b = (int)a;
}

void testfunc1(void)
{
	unsigned char a;
	unsigned char b;

	b =  a;
}

#if CRAZYMACRO < 0
#endif

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

	int abcdefghijklmnopqrstuvwxyz1234567890;
	int abcdefghijklmnopqrstuvwxyz12345678902;

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

void testfunc9(void)
{
	int** p1;
	int*** p2;

	struct
	{
		int* sp1;
		int** sp2;
		int*** sp3;
	};

	struct s* ps1;
	struct s** ps2;
	struct s*** ps3;

	INTPTR *const* const fedupp1;

	int ** (*pfunc1)();
	int ** (**pfunc2)();
	int ** (***pfunc3)();
	int *** (**pfunc4)();
	int ** (**pfunc5)(int**, int**);
	int ** (**pfunc6)(int**, int***);
}

void testfunc10 (int ** (**p5)(int**, int**), int ** (**p6)(int**, int***))
{

}
/*********************************************************************************************************************/
/*last line intentionally left blank.*/
