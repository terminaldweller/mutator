
/*first line's intentionally left blank.*/
/**********************************************************************************************************************/
/*inclusion directives*/
#include "auto-refac-tdd01.h"
/**********************************************************************************************************************/
/**************************************************MACROS & DEFS*******************************************************/
#define MACRO1 100U

/**********************************************************************************************************************/
/******************************************************Globals*********************************************************/


/**********************************************************************************************************************/
/***********************************************Function Definitions***************************************************/
void tdd_forfixer(void)
{
  int loopcounter;
  int counter;

  for (loopcounter = 0; loopcounter < 10; ++loopcounter)
    counter;

  for (loopcounter = 0; loopcounter < 10; ++loopcounter)
    counter--       ;

  for (loopcounter = 0; loopcounter < 10; ++loopcounter)
  {
    counter++;
  }
}

void tdd_whilefixer(void)
{
  int whilecounter;

  while (true)
    whilecounter;

  while (true)
    whilecounter++     ;

  while (true)
  {
    counter++;
  }
}

void tdd_ifelsefixer(void)
{
  int a0;
  int b0;

  if (0 == a0)
  {
    /*something*/
  }
  else
    a0++;

  if (0 == a0)
  {
    /*something*/
  }
  else
    a0++     ;

  if (0 == a0)
  {
    /*something*/
  }
  else
  {
    a0++;
  }

  if (0 == a0)
  {
    if (0 == b0)
    {

    }
    else
      b0++;
  }
  else
    a0++;

}

void tdd_iffixer(void)
{
  int a1;
  int b1;

  if (a1 > 10)
    a1++;

  if (b1 < 10)
    a1++  ;
  else if (a1 == 0)
    b1--      ;
  else
  {

  }

  if (a1 > 10)
    a1++;

  if (b1 < 10)
    a1++  ;
  else if (a1 == 0)
  {
    b1--      ;
    if (a1 < 10)
      a1--;
    else if (b1 > 10)
      b1++;
  }
  else
  {

  }

}

void tdd_ifconstswapper(void)
{
  int a;

  if (a = 10)
  {
    /*blah blah*/
  }

  if (a == MACRO1)
  {

  }

  if (MACRO1 == a)
  {

  }
}
/**********************************************************************************************************************/
/**********************************************************************************************************************/
/*last line's intntionally left blank.*/
