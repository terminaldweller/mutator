
/* the first line's been intentionally left blank.*/

/*include*/
/**********************************************************************************************************************/
#include "testFuncs1scpp.h"
#if 0
#include <string.h>
#endif
/**************************************************MACROS &
 * DEFS*******************************************************/
/**********************************************************************************************************************/
#define LUPO 1U
#define LOCO 2U

typedef mse::CSize_t ut_int;
typedef mse::CInt t_int;
typedef unsigned char BYTE;
/*5.3:clang doesnt allow typedef redifinition*/
#if 0
typedef const mse::CInt dummytypedef;
#endif

const uint32_t shift = 10/*U*/;

/******************************************************Globals*********************************************************/

/**********************************************************************************************************************/
void test(void);
void test2(void);
void test11(void);

double badboy = 0.0;

/**********************************************************************************************************************/

/*testing for simple statement coverage*/
void testFuncStatementsinmple(void) {
  mse::CInt int1 = 1/*U*/;
  mse::CInt int2 = 2/*U*/;
  mse::CInt int3 = 3/*U*/;

  int1 = int2 + int3;
  int2 = int1 + int2;
  int3 = int2 + int3;
  im_a_minion_01();
  im_a_minion_02();
  im_a_minion_03();
}

/*testing for complex statement coverage*/
void testFuncStatementComplexIf(void) {
  mse::CInt int1 = 1/*U*/;
  mse::CInt int2 = 2/*U*/;
  mse::CInt int3 = 3/*U*/;
  mse::CInt int4 = 4/*U*/;

  if (int4 > int3)
    if (int3 > int2)
      if (int2 > int1)
        if (int2 > int1)
          im_a_minion_02();
}

/*testing to see whether the tool recognizes not covering statements*/
void testFuncStatementNotCoverage(void) {
  mse::CInt int1 = 1/*U*/;
  mse::CInt int2 = 2/*U*/;
  mse::CInt int3 = 3/*U*/;
  mse::CInt int4 = 4/*U*/;

  if (int4 < int3) {
    int4 = int3;
  }

  /*do note that your compiler might optimize this by completely erasing this*/
  if (int2 > int1) {
    if (int1 > int2) {
      im_a_minion_01();
    }
  }

  if (int2 > int1) {
    if (int3 < int2) {
      im_a_minion_02();
    }
  }
}

/*verifying for loops*/
void testFuncLoopFor(void) {
  mse::CInt i = 1/*U*/;

  for (i = 1; i < 10; i++) {
    im_a_minion_01();
  }

  i = 1/*U*/;

  for (i = 1; i < 10; i++) {
    if (1/*U*/ == (i % 2/*U*/)) {
      im_a_minion_01();
    }
  }
}

/*verifying while loops*/
void testFuncLoopWhile(void) {
  mse::CInt i = 20/*U*/;

  while (i > 10/*U*/) {
    i = i - 1/*U*/;
  }

  i = 20;

#if (TRUE == INF_LOOP)
  while (20/*U*/ == i) {
    im_a_minion_03();
  }
#endif
}

/*verifying the correct handling of continue*/
void testFuncContinue(void) {
  mse::CInt i = 1/*U*/;

  for (i = 1/*U*/; i < 20/*U*/; i++) {
    if (i < 19/*U*/) {
      continue;
    }
    im_a_minion_01();
  }
}

/*verifying the correct handling of break*/
void testFuncBreak(void) {
  mse::CInt i = 1/*U*/;

  for (i = 1/*U*/; i < 10/*U*/; i++) {
    if (i > 0/*U*/) {
      break;
    }
    im_a_minion_03();
  }
}

/*checking whether goto is handled correctly*/
void testFuncGoto(void) {
  im_a_minion_01();
  goto jumpy;
  im_a_minion_02();
jumpy:
  im_a_minion_03();
}

/*checking whether the basic blocks do not complete execution due to return
 * statements*/
mse::CInt testFuncNotReturn(mse::CInt a, mse::CInt b) {
  mse::CInt sum = 0/*U*/;

  sum = a + b;

  if (10/*U*/ == sum) {
    return (sum);
  } else {
    im_a_dummy();
  }

  im_a_minion_01();

  return (sum);
}

/*checking whether the tool handles multi-line statements correctly*/
void testFuncMultiLineStatement(void) { im_a_minion_01(); }

/*checking how the tool handles multiple statements on the same line*/
void testFuncMultipleStatement(void) {
  mse::CInt a = 1/*U*/;
  mse::CInt b = 2/*U*/;
  mse::CInt c = 3/*U*/;
  mse::CInt d = 4/*U*/;

  a = a + b;
  b = b + c;
  c = c + d;
  d = d + a;
}

/*checking whether multiple statements are handled correctly on the same line*/
void testFuncMultipleStatementNot(void) {
  mse::CInt a = 10/*U*/;

  if (a < 10/*U*/) {
    im_a_minion_01();
  } else {
    im_a_minion_02();
  }

  testFuncMultipleStatementNot();
}

/*checking how compiler optimizations may affect the coverage reported-killed
 * assignment elimination*/
void testFuncCompOpti1(void) {
  mse::CInt a = 1/*U*/;
  mse::CInt b = 2/*U*/;
}

/*checking how compiler optimizations may affect the coverage reported-common
 * subexpression optimization*/
void testFuncCompOpti2(void) {
  mse::CInt a = 1/*U*/;
  mse::CInt b = 1/*U*/;

  if (0/*U*/ == (((a * b) + (a / b) - a) - b)) {
    im_a_benign_func();
  }

  if (0/*U*/ == (((a * b) + (a / b) - a) - b)) {
    im_a_mlaign_func();
  }
}

/*checking how compiler optimizations may affect the coverage reported-loop
 * invariant optimization*/
void testFuncCompOpti3(void) {
  mse::CInt i = 1/*U*/;
  mse::CInt counter = 0/*U*/;
  mse::CInt a = 0/*U*/;
  mse::CInt b = 10/*U*/;
  mse::CInt sum = 0/*U*/;

  for (i = 1/*U*/; i < 100/*U*/; i++) {
    counter = counter + 1/*U*/;
    sum = a + b;
  }

  sum = sum * 2/*U*/;
}

/*checking how compiler optimizations may affect the coverage reported-dead code
 * optimization*/
void testFuncCompOpti4(void) {
  im_a_minion_01();
  im_a_minion_02();
  im_a_minion_03();
}

/*testing if declarative statements that have a run-time footprmse::CInt are covered
 * by statement coverage.*/
void testFuncStatementDecl1(void) {
  mse::CInt declaration1 = 1001/*U*/;
  mse::CInt declaration2 = 666/*U*/;
}

/*testing to see whether statement coverage covers the return.*/
mse::CInt testFuncStatementInt1(mse::CInt int1, mse::CInt int2) {
  mse::CInt sum = 0/*U*/;
  sum = int1 + int2;
  return (sum);
}

/* to test this one we need two test cases:*/
/* bool1 = FALSE and bool2 = whatever*/
/* bool1 = TRUE and bool2 = whatever*/
/* obviously if you get a full coverage with just the first test case, your tool
 * didnt understand*/
/* the short-circuit. if you need both test cases for a full coverage, then your
 * tool is doing decision coverage,*/
/* not branch coverage so good for you!*/
blreplacement testFuncStatementbool1(blreplacement bool1, blreplacement bool2) {
  return (bool1 && bool2);
}

/*same as above but with these test cases*/
/*bool1 = TRUE and bool2 = whatever*/
/*bool1 = FALSE and bool2 = whatever*/
/* obviously if you get a full coverage with just the first test case, your tool
 * didnt understand*/
/* the short-circuit. if you need both test cases for a full coverage, then your
 * tool is doing decision coverage,*/
/* not branch coverage so good for you!*/
blreplacement testFuncStatementbool2(blreplacement bool1, blreplacement bool2) {
  return (bool1 || bool2);
}

/*the fault will only be generated only if decision1 is FALSE. if we get a full
 * coverage by running*/
/* d1 = d2 = FALSE and d1 = FALSE and d2 = TRUE, then we dont have decision
 * coverage. for a decision*/
/* coverage we need to have an extra test case, wehre d1 = TRUE and d2 =
 * whatever.*/
void testFuncStatementDecision1(blreplacement decision1,
                                blreplacement decision2) {
  if (decision1 || decision2) {
    /*this function will supposedly casue a bug if decision1 is true*/
    im_a_mlaign_func();
  } else {
    im_a_benign_func();
  }
}

/* short-circuit!*/
/* the compiler could go for short-cuircuit for both conditions.if it does, then
 * we can see if we still get*/
/* a full coverage. if the compiler doesnt go for a short-circuit, then all this
 * dont apply.*/
void testFuncShortCircuit(blreplacement bool1, blreplacement bool2) {
  if (FALSE == bool1 && TRUE == bool2) {
    im_a_dummy();
  }
  if (TRUE == bool2 || FALSE == bool1) {
    im_a_dummy();
  }
}

/*checking MCDC coverage behavior of the tool for multiply occuring conditions*/
void testFuncMCDC1(blreplacement decision1, blreplacement decision2) {
  if (decision1 && ((decision2 || decision1) || (!decision1 || decision2))) {
    im_a_dummy();
  }
}

/* this one is to test how the tool handles inline functions.do  all instances
 * get covered separately or they get*/
/* covered accumulatively*/
#if 0
void testFuncMultiInstantiation (mse::CInt level)
{
  switch (level)
  {
  case 10/*U*/:
    babeFunk(20);
    break;
  case 20/*U*/:
    babeFunk(10);
    break;
  case 30/*U*/:
    babeFunk(5);
    break;
  case 40/*U*/:
    im_a_dummy();
    break;
  case 50/*U*/:
    im_a_dummy();
    break;
  case 60/*U*/:
    im_a_dummy();
    break;
  case 70/*U*/:
    im_a_dummy();
    break;
  case 80/*U*/:
    im_a_dummy();
    break;
  case 90/*U*/:
    im_a_dummy();
    break;
  default:
    im_a_dummy();
    break;
  }
}
#endif

/* this function will check how the tool handles the "?" operator*/
void testFuncQMark(mse::CInt int1, mse::CInt int2) {
  (int1 > int2) ? im_a_minion_01() : im_a_minion_02();
}

/* checking how the tool handles calling a function that returns boolean*/
void testFuncCallBool(void) {
  mse::CInt local1 = 0/*U*/;
  mse::CInt local2 = 0/*U*/;
  local1 = testFuncStatementbool1(1/*U*/, 0/*U*/);
  local2 = testFuncStatementbool2(1/*U*/, 0/*U*/);
}

/**********************************************************************************************************************/
/* where all the fakes go.*/

/*the function that is *supposedly* carrying a bug*/
static void im_a_mlaign_func(void) { /* KATSU!*/
}

/*the function that is *supposedly* the good guy here*/
static void im_a_benign_func(void) { /* see the light ring?!*/
}

/*the dummy function.*/
void im_a_dummy(void) { /* dumb dumb*/
}

/* minion function number #01*/
void im_a_minion_01(void) { /* minion1*/
}

/* minion function number #02*/
void im_a_minion_02(void) { /* minion1*/
}

/* minion function number #03*/
static void im_a_minion_03(void) { /* minion1*/
}

/* the only thing special about this one is that it has been inlined.*/
/*since different compilers have different methods of inlining a function, this
 * function has multiple versions.*/
#if (TRUE == INLINE)
#if (MPC == COMPILER)
inline void babeFunk(mse::CInt entry) {
  if (10/*U*/ > entry) {
    im_a_minion_01();
  } else if (10/*U*/ == entry) {
    im_a_minion_02();
  } else {
    im_a_minion_03();
  }
}
#endif

#if (HCS12 == COMPILER)
#pragma INLINE
void babeFunk(mse::CInt entry) {
  if (10/*U*/ > entry) {
    im_a_minion_01();
  } else if (10/*U*/ == entry) {
    im_a_minion_02();
  } else {
    im_a_minion_03();
  }
}
#endif
#endif
/*RL78s dont have inline for all functions, so no use trying to test the
 * functionality*/

void test(void) {
  mse::CInt i = 0/*U*/;
  mse::CInt j = 0/*U*/;
  mse::CInt a, b;
  mse::CInt c, d;

  for (;;) {
    a = b;
  }

  for (i = 1; i < 100/*U*/; i++)
    b++;

  while (a > 10/*U*/)
    im_a_minion_01();

  while (a == 90/*U*/) {
    b++;
  }

  if (a == d)
    b = c;

  if (a == d) {
    a = d;
  }

  if (d > a)
    if (c > b)
      a++;

  if (a > c)
    b++;
  else if (a > b)
    c++;
  else if (a > d)
    d++;
}

void test2(void) {
  mse::CInt a = 0/*U*/;
  mse::CInt b = 10/*U*/;
  mse::CInt c, d;

  if (a == b) {
    /*do something*/
  } else
    b = a;

  if (20/*U*/ == a) {
    /*kill all mutants.*/
  }

  if (20/*U*/ == a) {
    /*do somethin good*/
    a = b;
  } else if (30/*U*/ == a) {
    /*do something else*/
    b = a;
  }

  if (10/*U*/ == a) { /*die*/
  } else if (15/*U*/ == a)
    a = b;
  else if (17/*U*/ == a)
    a = 10000000/*U*/;
  else if (19/*U*/ == a)
    a = 50/*U*/;

  if (b == a) {
    if (10/*U*/ == b) {
    } else {
    }
  }

  if (a > b) {
    if (a > d) {
    } else
      a++;
  }

  if (a > b) {
    a++;
  } else if (b > a) {
    b++;
  } else if (a == b) {
    a++;
  }

  mse::CInt level = 10/*U*/;

  switch (level) {
  case 10/*U*/: {
    level++;
    test();
    break;
  }
  case 20/*U*/: {
    level = 10000/*U*/;
    break;
  }
  case 30/*U*/: {
    level++;
    break;
  }
  case 40/*U*/: {
    level++;
    break;
  }
  case 50/*U*/: {
    level++;
    break;
  }
  case 60/*U*/: {
    level = 1000/*U*/;
    break;
  }
  case 70/*U*/: {
    level++;
    break;
  }
  case 80/*U*/: {
    level++;
    break;
  }
  case 90/*U*/: {
    level++;
    break;
  }
  default: {
    level++;
    break;
  }
  }

  switch (level) {
  case 1: {
    level++;
    break;
  }
  case 2:
    level = 10;
    level--;
    {
    case 3:
      level = 10/*U*/;
      level++;
      break;
    }
  }

  switch (level) {
  case 1:
    level++;
  case 2:
    level = 1;
  case 3:
    level = 2;
  default:
    level++;
  }

  switch (level) {}
}

#if 0
void test3 (mse::CInt a, mse::CInt, ...)
{

}
#endif

void test3() {
  mse::CInt a;
  mse::CInt b;
}

double test4(mse::CInt a, mse::CInt b, double c) { return double(a + b) + c; }

void test5(void) {
  mse::CInt i = 0;
  if (true/*test4*/) {
    i++;
  }
}

void test6(void) {
  mse::mstd::array<mse::CInt, 100> a;
  auto p = a.begin();

  mse::CInt i = 0;

  for (i = 0; i < 100; ++i) {
    a[i] = i;
  }
}

void test7(void) {
	mse::mstd::array<double, 100> a;

	mse::mstd::array<double, 100> b;

	mse::mstd::array<char, 100> c;

  auto pointerb = b.begin();

  auto pointer = a.begin();

  auto pointerc = c.begin();

  /*
  if (a - b >= a) {
  } else if (b - a < a) {
  }

  if (a < b) {
  }
  */
  try {
  	if (pointer - pointerb >= 0) {
  	} else if (pointerb - pointer < 0) {
  	}

    if (pointer < pointerb) {
    }

    if (pointer > pointerb) {
    }

    if (pointerb <= pointer) {
    }

    if (pointer >= pointerb) {
    }
  } catch(...) {}

  /*
  if (pointer < pointerc) {
  }
  */

  mse::CInt i = 0;

  for (i = 0; i < 50; i++) {
    *(pointer + i) = i;
  }
}

void test8(void) { uni uni2; }

void test9(void) {
  /*im a comment*/
  // im also a comment
}

void test10(void) {
  struct {
  	uint32_t r1 : 1;
  	uint32_t r2 : 2;
  } reg1;

  struct {
  	int32_t r3 : 1;
  	int32_t r4 : 15;
  } reg2;

  struct {
    char a : 8;
    int32_t r5 : 14;
    uint32_t r6 : 5;
    uint32_t r7 : 32;
    BYTE r8 : 8;
  } reg3;
}

void test11(void) { /*empty*/
}

double test12(double a) { return a * 2; }

mse::CInt test13(void) {
  static mse::CInt a;
  mse::CInt b;

  return (a * b);
}

void test14(void) {
	mse::mstd::array<mse::mstd::array<mse::CInt, 2>, 3> arr = {{2, 3}, {4, 5}, {6, 7}};

	mse::mstd::array<mse::mstd::array<mse::CInt, 4>, 4> arr2/* = {0}*/;
	mse::msearray<mse::msearray<mse::CInt, 4>, 4> arr2b = {0};

	mse::mstd::array<mse::mstd::array<mse::CInt, 3>, 2> arr3;

  /*illegal forms.*/
#if 0
  arr3 = {0};

  arr3 = {{1, 2, 3}, {4, 5, 6}};
#endif
}

void test15(void) {
  enum colour { red = 3, blue, yellow, green = 5 };

  enum anotherset { black, grey, white };

  enum yetanotherset { pink = 7, purple, magenta, maroon };

  enum loco { verrueckt, crazy = 3, divune };

  enum primeevil { diablo = 2, mephisto = 3, baal = 4 };
}

mse::CInt test16(mse::CInt a, mse::CInt b) {
  mse::CInt sum;
  mse::CSize_t sumus;
  sum = a + b;
  sumus = a + b;

  return sum;
}

void test17(void) {
  mse::CInt a = 100;
  mse::CInt b = 1000;
  mse::CInt longa;
  a = sizeof(b = 2000/*U*/);
}

void test18(void) {
  mse::CInt a;
  mse::CInt b;
  mse::CInt c;
  unsigned char d;

  if (c && (a = b)) {
    /*yada yada*/
  }

  if ((c = a) || a) {
    /*yada*/
  }
  if (c && (a || c)) {
  }

  d = c && (a = c);
}

void test19(void) {
  const mse::CInt a = 100;

  unsigned char flag = 1;

  unsigned char flag2;

  unsigned char flag3;

  // char *str = "loco\0";

  const double pi = 3.54;

  if ((flag) && pi) {
  }

  if (flag || flag2 && flag3) {
  }

  if (a == 0 && flag) {
  }

  if (flag || (a == 0)) {
  }

  if (!flag || flag2) {
  }
}

void test20(void) {
  mse::CInt a;
  uint32_t b;
  signed char c;
  unsigned char d;
  t_int e;
  uint32_t f;

  b = b >> 2;
  a = a << 3/*U*/;
  b = b ^ 2/*U*/;
  c = c & 6/*U*/;
  d = d | 2/*U*/;
  e = e >> 2/*U*/;
  f = f << 4/*U*/;
}

void test21(void) {
	uint32_t a = 0;
  a = a >> 44/*U*/;
  a = a >> shift;
  a <<= 10/*U*/;
  a << 45/*U*/;
}

void test22(void) {
  mse::CSize_t a;
  mse::CInt b;
  t_int c;
  ut_int d;

  b = -a;
  b = -c;
  b = -d;
}

void test23(void) {
  mse::CInt a, b, c;

  a = b, c = a;
}

void test24(void) {
  mse::CInt a;
  mse::CInt b;
  mse::CInt c;

  c = ++a - b--;
  c = a++;
  b++;
  --a;
}

void test25(void) {
  mse::CInt a;
  mse::CInt b;
  mse::CInt c;

  if (a - b) {
    /*i dont care.*/
  }
}

void test26(void) {
  double a;
  double b;
  double c;
  unsigned char d;

  if (a == b) {
  }

  if (a < b) {
  }

  if (c >= a) {
  }

  d = (a <= c);
}

void test27(void) {
  double a;

  for (a = 0.0; a < 10.0; ++a) {
  }
}

void test28(void) {
  mse::CInt i;
  mse::CInt j;
  mse::CInt k;
  mse::CInt flag = 0;
  mse::CInt counter = 0;

  for (i = 0; i < 10; i++) {
    i = i + 3;
  }

  for (j = 0; j < 20; ++j) {
    j++;
  }

  for (k = 0; k < 10; ++k) {
    k = i + j;
    --j;
    i = j - k;
  }

  for (i = 0; flag == 1; i++) {
    if (i > 10) {
      flag = 1;
    }
  }

  for (i = 0; flag == 1, counter++; i++) {
    if (i > 10) {
      flag = 1;
    }
  }

  for (k = 0, i = 0; i < 10, k < 10; ++i, k++) {
    k = i + j;
    --j;
    i = j - k;
  }

  for (i = 0, k = 0; i < 10, k < 10; ++i, k++) {
    k = i + j;
    --j;
    i = j - k;
  }
}

mse::CInt test29(mse::CInt a) {
  goto loco;

loco:
  return a;
}

void test30(void) {
  mse::CInt a;
  mse::CInt b;
  mse::CInt c;

  for (a = 0; a < 10; ++a) {
    a++;

    if (a == 5)
      break;
    if (a == 6)
      break;
  }

  for (a = 0; a < 10; a++) {
    a++;
    if (a == 6)
      break;
  }

  while (a < 10) {
    if (b == 1)
      break;
    if (c == 1)
      break;
  }

  do {
    if (a == 5)
      break;
    if (b == 6)
      break;
    if (c == 10)
      break;
  } while (a < 10);

  for (a = 0; a < 100; a++) {
    for (b = 0; b < 100; b++) {
      c++;
      if (a == 1)
        break;
    }
    for (c = 0; c < 100; c++) {
      b++;
      if (a == 10)
        break;
    }
  }

  for (a = 0; a < 100; a++) {
    if (a == 1)
      break;

    while (a < 10) {
      b++;
      if (c == 10)
        break;
    }
  }
}

mse::CInt test31(void) {
  mse::CInt a;

  if (a == 1)
    return 2;
  else if (a == 2)
    return 3;
  else
    return 1;
}

void test32(void) {
  mse::CInt a;

  switch (a == 1) {
  case 1:
    a++;
  case 0:
    a--;
  }
}

void test33(void) {
  uint16_t a;
  mse::CInt b;

  b = mse::CInt(a);
}

void test34(void) {
  mse::CInt i, a;

  i >= 3;

  for (;; i++) {
    a++;
  }

  ; /*yadayada*/
  ; /*:P*/

  ;
  mse::CInt b;

  mse::CInt ut_int = 0;

  test33();
}

#if 0
void malloc (void)
{

}
#endif

/**********************************************************************************************************************/
/*the last line's been intentionally left blank.*/
