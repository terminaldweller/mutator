
/* the first line's been intentionally left blank.*/

/*include*/
/**********************************************************************************************************************/
#include "testFuncs1.h"
#if 0
#include <string.h>
#endif
/**************************************************MACROS &
 * DEFS*******************************************************/
/**********************************************************************************************************************/
#define LUPO 1U
#define LOCO 2U

typedef unsigned int ut_int;
typedef signed int t_int;
typedef unsigned char BYTE;
/*5.3:clang doesnt allow typedef redifinition*/
#if 0
typedef const int dummytypedef;
#endif

const unsigned int shift = 10U;

/******************************************************Globals*********************************************************/

/**********************************************************************************************************************/
void test(void);
void test2(void);
void test11(void);

double badboy = 0.0;

/**********************************************************************************************************************/

/*testing for simple statement coverage*/
void testFuncStatementsinmple(void) {
  int int1 = 1U;
  int int2 = 2U;
  int int3 = 3U;

  int1 = int2 + int3;
  int2 = int1 + int2;
  int3 = int2 + int3;
  im_a_minion_01();
  im_a_minion_02();
  im_a_minion_03();
}

/*testing for complex statement coverage*/
void testFuncStatementComplexIf(void) {
  int int1 = 1U;
  int int2 = 2U;
  int int3 = 3U;
  int int4 = 4U;

  if (int4 > int3)
    if (int3 > int2)
      if (int2 > int1)
        if (int2 > int1)
          im_a_minion_02();
}

/*testing to see whether the tool recognizes not covering statements*/
void testFuncStatementNotCoverage(void) {
  int int1 = 1U;
  int int2 = 2U;
  int int3 = 3U;
  int int4 = 4U;

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
  int i = 1U;

  for (i = 1; i < 10; i++) {
    im_a_minion_01();
  }

  i = 1U;

  for (i = 1; i < 10; i++) {
    if (1U == (i % 2U)) {
      im_a_minion_01();
    }
  }
}

/*verifying while loops*/
void testFuncLoopWhile(void) {
  int i = 20U;

  while (i > 10U) {
    i = i - 1U;
  }

  i = 20;

#if (TRUE == INF_LOOP)
  while (20U == i) {
    im_a_minion_03();
  }
#endif
}

/*verifying the correct handling of continue*/
void testFuncContinue(void) {
  int i = 1U;

  for (i = 1U; i < 20U; i++) {
    if (i < 19U) {
      continue;
    }
    im_a_minion_01();
  }
}

/*verifying the correct handling of break*/
void testFuncBreak(void) {
  int i = 1U;

  for (i = 1U; i < 10U; i++) {
    if (i > 0U) {
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
int testFuncNotReturn(int a, int b) {
  int sum = 0U;

  sum = a + b;

  if (10U == sum) {
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
  int a = 1U;
  int b = 2U;
  int c = 3U;
  int d = 4U;

  a = a + b;
  b = b + c;
  c = c + d;
  d = d + a;
}

/*checking whether multiple statements are handled correctly on the same line*/
void testFuncMultipleStatementNot(void) {
  int a = 10U;

  if (a < 10U) {
    im_a_minion_01();
  } else {
    im_a_minion_02();
  }

  testFuncMultipleStatementNot();
}

/*checking how compiler optimizations may affect the coverage reported-killed
 * assignment elimination*/
void testFuncCompOpti1(void) {
  int a = 1U;
  int b = 2U;
}

/*checking how compiler optimizations may affect the coverage reported-common
 * subexpression optimization*/
void testFuncCompOpti2(void) {
  int a = 1U;
  int b = 1U;

  if (0U == (((a * b) + (a / b) - a) - b)) {
    im_a_benign_func();
  }

  if (0U == (((a * b) + (a / b) - a) - b)) {
    im_a_mlaign_func();
  }
}

/*checking how compiler optimizations may affect the coverage reported-loop
 * invariant optimization*/
void testFuncCompOpti3(void) {
  int i = 1U;
  int counter = 0U;
  int a = 0U;
  int b = 10U;
  int sum = 0U;

  for (i = 1U; i < 100U; i++) {
    counter = counter + 1U;
    sum = a + b;
  }

  sum = sum * 2U;
}

/*checking how compiler optimizations may affect the coverage reported-dead code
 * optimization*/
void testFuncCompOpti4(void) {
  im_a_minion_01();
  im_a_minion_02();
  im_a_minion_03();
}

/*testing if declarative statements that have a run-time footprint are covered
 * by statement coverage.*/
void testFuncStatementDecl1(void) {
  int declaration1 = 1001U;
  int declaration2 = 666U;
}

/*testing to see whether statement coverage covers the return.*/
int testFuncStatementInt1(int int1, int int2) {
  int sum = 0U;
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
void testFuncMultiInstantiation (int level)
{
  switch (level)
  {
  case 10U:
    babeFunk(20);
    break;
  case 20U:
    babeFunk(10);
    break;
  case 30U:
    babeFunk(5);
    break;
  case 40U:
    im_a_dummy();
    break;
  case 50U:
    im_a_dummy();
    break;
  case 60U:
    im_a_dummy();
    break;
  case 70U:
    im_a_dummy();
    break;
  case 80U:
    im_a_dummy();
    break;
  case 90U:
    im_a_dummy();
    break;
  default:
    im_a_dummy();
    break;
  }
}
#endif

/* this function will check how the tool handles the "?" operator*/
void testFuncQMark(int int1, int int2) {
  (int1 > int2) ? im_a_minion_01() : im_a_minion_02();
}

/* checking how the tool handles calling a function that returns boolean*/
void testFuncCallBool(void) {
  int local1 = 0U;
  int local2 = 0U;
  local1 = testFuncStatementbool1(1U, 0U);
  local2 = testFuncStatementbool2(1U, 0U);
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
inline void babeFunk(int entry) {
  if (10U > entry) {
    im_a_minion_01();
  } else if (10U == entry) {
    im_a_minion_02();
  } else {
    im_a_minion_03();
  }
}
#endif

#if (HCS12 == COMPILER)
#pragma INLINE
void babeFunk(int entry) {
  if (10U > entry) {
    im_a_minion_01();
  } else if (10U == entry) {
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
  int i = 0U;
  int j = 0U;
  int a, b;
  int c, d;

  for (;;) {
    a = b;
  }

  for (i = 1; i < 100U; i++)
    b++;

  while (a > 10U)
    im_a_minion_01();

  while (a == 90U) {
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
  int a = 0U;
  int b = 10U;
  int c, d;

  if (a == b) {
    /*do something*/
  } else
    b = a;

  if (20U == a) {
    /*kill all mutants.*/
  }

  if (20U == a) {
    /*do somethin good*/
    a = b;
  } else if (30U == a) {
    /*do something else*/
    b = a;
  }

  if (10U == a) { /*die*/
  } else if (15U == a)
    a = b;
  else if (17U == a)
    a = 10000000U;
  else if (19U == a)
    a = 50U;

  if (b == a) {
    if (10U == b) {
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

  int level = 10U;

  switch (level) {
  case 10U: {
    level++;
    test();
    break;
  }
  case 20U: {
    level = 10000U;
    break;
  }
  case 30U: {
    level++;
    break;
  }
  case 40U: {
    level++;
    break;
  }
  case 50U: {
    level++;
    break;
  }
  case 60U: {
    level = 1000U;
    break;
  }
  case 70U: {
    level++;
    break;
  }
  case 80U: {
    level++;
    break;
  }
  case 90U: {
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
      level = 10U;
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
void test3 (int a, int, ...)
{

}
#endif

test3() {
  int a;
  int b;
}

double test4(int a, int b, double c) { return a + b + c; }

void test5(void) {
  int i = 0;
  if (test4) {
    i++;
  }
}

void test6(void) {
  int a[100];
  int *p = a;

  int i = 0;

  for (i = 0; i < 100; ++i) {
    a[i] = i;
  }
}

void test7(void) {
  double a[100];

  double b[100];

  char c[100];

  double *pointer;

  double *pointerb;

  char *pointerc;

  pointerb = b;

  pointer = a;

  pointerc = c;

  if (a - b >= a) {
  } else if (b - a < a) {
  }

  if (a < b) {
  }

  if (pointer < pointerb) {
  }

  if (pointer > pointerb) {
  }

  if (pointerb <= pointer) {
  }

  if (pointer >= pointerb) {
  }

  if (pointer < pointerc) {
  }

  int i = 0;

  for (i = 0; i < 50; i++) {
    *(a + i) = i;
  }
}

void test8(void) { union uni uni2; }

void test9(void) {
  /*im a comment*/
  // im also a comment
}

void test10(void) {
  struct {
    unsigned int r1 : 1;
    unsigned int r2 : 2;
  } reg1;

  struct {
    signed int r3 : 1;
    signed int r4 : 15;
  } reg2;

  struct {
    char a : 8;
    signed int r5 : 14;
    unsigned int r6 : 5;
    ut_int r7 : 32;
    BYTE r8 : 8;
  } reg3;
}

void test11(void) { /*empty*/
}

double test12(double a) { return a * 2; }

int test13(void) {
  static int a;
  int b;

  return (a * b);
}

void test14(void) {
  int arr[3][2] = {{2, 3}, {4, 5}, {6, 7}};

  int arr2[4][4] = {0};

  int arr3[2][3];

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

short int test16(int a, int b) {
  short int sum;
  short unsigned int sumus;
  sum = a + b;
  sumus = a + b;

  return sum;
}

void test17(void) {
  int a = 100;
  int b = 1000;
  long int longa;
  a = sizeof(b = 2000U);
}

void test18(void) {
  int a;
  int b;
  int c;
  unsigned char d;

  if (c && (a = b)) {
    /*yada /*yada*/
  }

  if ((c = a) || a) {
    /*yada*/
  }
  if (c && (a || c)) {
  }

  d = c && (a = c);
}

void test19(void) {
  const int a = 100;

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
  signed int a;
  unsigned int b;
  signed char c;
  unsigned char d;
  t_int e;
  ut_int f;

  b = b >> 2;
  a = a << 3U;
  b = b ^ 2U;
  c = c & 6U;
  d = d | 2U;
  e = e >> 2U;
  f = f << 4U;
}

void test21(void) {
  unsigned int a;
  a = a >> 44U;
  a = a >> shift;
  a <<= 10U;
  a << 45U;
}

void test22(void) {
  unsigned int a;
  int b;
  t_int c;
  ut_int d;

  b = -a;
  b = -c;
  b = -d;
}

void test23(void) {
  int a, b, c;

  a = b, c = a;
}

void test24(void) {
  int a;
  int b;
  int c;

  c = ++a - b--;
  c = a++;
  b++;
  --a;
}

void test25(void) {
  int a;
  int b;
  int c;

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
  int i;
  int j;
  int k;
  int flag = 0;
  int counter = 0;

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

int test29(int a) {
  goto loco;

loco:
  return a;
}

void test30(void) {
  int a;
  int b;
  int c;

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

int test31(void) {
  int a;

  if (a == 1)
    return 2;
  else if (a == 2)
    return 3;
  else
    return 1;
}

void test32(void) {
  int a;

  switch (a == 1) {
  case 1:
    a++;
  case 0:
    a--;
  }
}

void test33(void) {
  short int a;
  long int b;

  b = (int)a;
}

void test34(void) {
  int i, a;

  i >= 3;

  for (;; i++) {
    a++;
  }

  ; /*yadayada*/
  ; /*:P*/

  ;
  int b;

  int ut_int = 0;

  test33();
}

#if 0
void malloc (void)
{

}
#endif

/**********************************************************************************************************************/
/*the last line's been intentionally left blank.*/
