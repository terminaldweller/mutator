
/*the first line's been intentionally left blank.*/
/***********************************************************************************************************/
/*defines*/
#ifndef TEST1_H
#define TEST1_H

#define FALSE 0
#define TRUE 1

#include "msepoly.h"
#include "mseprimitives.h"
#include <functional>

/***********************************************Global******************************************************/
typedef mse::tdp_variant<mse::CInt, double, mse::mstd::array<char, 20>> uni;
extern uni uni1;

//extern mse::CInt eldiablo = 0;

extern mse::TAnyRandomAccessIterator<mse::CInt> cccc;

//extern cucu;
mse::CInt yetanotherheadervar;

double yetanotherdoubleinsideaheader;
/***********************************************************************************************************/
/*choose the compiler*/
#define COMPILER  GCC

#define HCS12 1U
#define MPC   2U
#define RL78  3U
#define GCC   4U
#define CLANG 5U

/***********************************************************************************************************/
/*options*/

/*this option sets the infinite while loop to see how your tool reacts to it.*/
/*setting it to ON enables it. setting it to anything else disables it but go with OFF for good taste.*/
/*setting this option to ON will practically disable the rest of the tests.*/
#define INF_LOOP FALSE

/*turn on the testing of inline functions*/
#define INLINE FALSE

/***********************************************************************************************************/
/*the compiler's gonna be needing some sorta pragmas to handle some keywords correctly or altogether.*/
/*you need pragmas for inlining, boolean, etc.*/
/*if your compiler needs em, put em here.*/




/***********************************************************************************************************/
#if (HCS12 == COMPILER)
#define blreplacement unsigned char
#endif
#if (MPC == COMPILER)
#define blreplacement unsigned char
#endif
#if (RL78 == COMPILER)
#define blreplacement unsigned char
#endif
#if (GCC == COMPILER)
#define blreplacement unsigned char
#endif

/***********************************************************************************************************/
/*all the test function prototypes are here.*/
void testFuncStatementsinmple (void);
void testFuncStatementComplexIf (void);
void testFuncStatementNotCoverage (void);
void testFuncLoopFor (void);
void testFuncLoopWhile (void);
void testFuncContinue (void);
void testFuncBreak (void);
void testFuncGoto (void);
mse::CInt testFuncNotReturn (mse::CInt a, mse::CInt b);
void testFuncMultiLineStatement (void);
void testFuncMultipleStatement (void);
void testFuncMultipleStatementNot (void);
void testFuncCompOpti1 (void);
void testFuncCompOpti2 (void);
void testFuncCompOpti3 (void);
void testFuncCompOpti4 (void);
void testFuncStatementDecl1 (void);
mse::CInt testFuncStatementInt1 (mse::CInt int1, mse::CInt int2);
blreplacement testFuncStatementbool1 (blreplacement bool1 , blreplacement bool2);
blreplacement testFuncStatementbool2 (blreplacement bool1 , blreplacement bool2);
void testFuncStatementDecision1 (blreplacement decision1 , blreplacement decision2);
void testFuncShortCircuit (blreplacement bool1 , blreplacement bool2);
void testFuncMCDC1 (blreplacement decision1 , blreplacement decision2);
#if (TRUE == INLINE)
void testFuncMultiInstantiation (mse::CInt level);
#endif
void testFuncQMark (mse::CInt int1, mse::CInt int2);
void testFuncCallBool (void);

static void im_a_mlaign_func (void);
static void im_a_benign_func (void);
void im_a_dummy (void);
void im_a_minion_01 (void);
void im_a_minion_02 (void);
static void im_a_minion_03 (void);

#if (TRUE == INLINE)
#if (MPC == COMPILER)
inline void babeFunk (mse::CInt entry);
#endif

#if (HCS12 == COMPILER)
void babeFunk (mse::CInt entry);
#endif
#endif

//test3();
double test4 (mse::CInt aa, mse::CInt bb, double cc);
void test5(void);
void test6(void);
void test7(void);
void test8(void);
void test10(void);
mse::CInt test13(void);
void test15(void);
void test17(void);
void test19(void);
void test20(void);
void test21(void);
void test22(void);
void test23(void);
void test24(void);
void test25(void);
void test26(void);
void test27(void);
void test28(void);
mse::CInt test29(mse::CInt a);
mse::CInt test31(void);

void headerfund(void)
{
  mse::CInt a;
  mse::CInt b;
  mse::CInt sum;
  sum = a + b;
}

template <typename _Ty> using nra_iter = mse::TNullableAnyRandomAccessIterator<_Ty>;

void testfunc9999(void)
{
	mse::TNullableAnyPointer<mse::TNullableAnyPointer<mse::CInt> > p1;
	mse::TNullableAnyPointer<mse::TNullableAnyPointer<mse::TNullableAnyPointer<mse::CInt> > > p2;

  struct s
  {
		mse::TNullableAnyPointer<mse::CInt> sp1;
		mse::TNullableAnyPointer<mse::TNullableAnyPointer<mse::CInt> > sp2;
		mse::TNullableAnyPointer<mse::TNullableAnyPointer<mse::TNullableAnyPointer<mse::CInt> > > sp3;
  };

  mse::TNullableAnyPointer<struct s> ps1;
  /*these two should not be tagged by 18.1 since they are pomse::CInters to an incomplete type. the pomse::CInter is a complete type.*/
  mse::TNullableAnyPointer<mse::TNullableAnyPointer<struct s> > ps2;
  mse::TNullableAnyPointer<mse::TNullableAnyPointer<mse::TNullableAnyPointer<struct s> > > ps3;

  //INTPTR *const* const fedupp1;

  /*
  int ** (*pfunc1)();
  int ** (**pfunc2)();
  int ** (***pfunc3)();
  int *** (**pfunc4)();
  int ** (**pfunc5)(int**, int**);
  int ** (**pfunc6)(int**, int***);
  */
  //template <typename _Ty> using nra_iter = mse::TNullableAnyRandomAccessIterator<_Ty>;
  nra_iter<std::function<nra_iter<nra_iter<mse::CInt> >(void)> > pfunc1;
  nra_iter<nra_iter<std::function<nra_iter<nra_iter<mse::CInt> >(void)> > > pfunc2;
  nra_iter<nra_iter<nra_iter<std::function<nra_iter<nra_iter<mse::CInt> >(void)> > > > pfunc3;
  nra_iter<nra_iter<std::function<nra_iter<nra_iter<nra_iter<mse::CInt> > >(void)> > > pfunc4;
  nra_iter<nra_iter<std::function<nra_iter<nra_iter<mse::CInt> >(nra_iter<nra_iter<mse::CInt> >, nra_iter<nra_iter<mse::CInt> >)> > > pfunc5;
  nra_iter<nra_iter<std::function<nra_iter<nra_iter<mse::CInt> >(nra_iter<nra_iter<mse::CInt> >, nra_iter<nra_iter<nra_iter<mse::CInt> > >)> > > pfunc6;
}

#if 0
void malloc (void);
#endif

#endif
/***********************************************************************************************************/
/*ive been intentionally left blank. dont touch me.*/
