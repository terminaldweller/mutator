
/*first line intentionally left blank.*/
/*********************************************************************************************************************/
int crappyint;

/*********************************************************************************************************************/
/*inclusion directives*/
#include "testFuncs3.h"
#include <complex.h>
#include <string.h>
/*********************************************************************************************************************/
/*globals*/
int intarray3[3][2] = MACRO1;
int intarray4[3][2] = MACRO2;
int answer = ANSWER;
float floatbitaccess;

/*********************************************************************************************************************/
void tddfunc1(void) {
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

void tddfunc2(void) {
	static int staticint1;

	unsigned short int port = 0x5aU;
	unsigned short int result_8;
	// uint16_t result_16;
	// uint16_t mode;
	result_8 = (~port) >> 4;
}

void tddfunc3(void) {
	int a = 1;

	READ_TIME_32();
	READ_TIME_33();
	READ_TIME_34();
}

int *tddfunc4(void) {
	int localauto;
	return (&localauto);
}

void tddfunc5(void) {
	int arrint[4];
	arrint[0] = 052;
	arrint[1] = 067;
	arrint[2] = 100;
	arrint[3] = 786;
}

/*12.12 tdd*/
void tddfunc6(void) {
	float floatbitaccess;
	int *pointertofloat;
	int apartoffloat;
	/*@DEVI-you cannot get away with this since c90 and c99 both specify that
	 * operands to & need to both be of integral types.*/
	/*embedded compilers might not necessarily be ISO-C-complete so apparently this
	might be actually viable in some cases but
	if the clang parser won't let this through, i can't analyze it so we can't check
	for this.*/
#if 0
	apartoffloat = floatbitaccess & 0x000000FF;
#endif
	/*this results in a bticast castexpr which we are already tagging,clang does
	too, so im not overly concerned with this one
	though its fairly east to implement.*/
	pointertofloat = &floatbitaccess;

	/*we could literally just tag all unions having float members as possible
	 * violations.*/
	union aunion {
		float member1;
		union member2 {
			unsigned char member3;
			unsigned char member4;
			unsigned char member5;
			unsigned char member6;
		} member22;
	} aunionproto;

	aunionproto.member1 = floatbitaccess;
	apartoffloat = aunionproto.member22.member5 & 0x000000FF;
}

int externfunc(int arg) { return arg; }

void testfunc7(void) {
	unsigned char chararroct[7U];

	chararroct[0U] = '\100';
	chararroct[1U] = '\109';
	chararroct[2U] = 010;
	chararroct[3U] = 055;
	chararroct[4U] = 125;
	chararroct[5U] = '\x12';
}

void tddfunc8(void) {
	// asm("add %al, (%rax)");

	/*for 20.2*/
	int printf;
}

/*********************************************************************************************************************/
/*last line intnetionally left blank.*/
