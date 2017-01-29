
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
    counter--;

  for (loopcounter = 0; loopcounter < 10; ++loopcounter)
  {
    counter++;
  }
}

void tdd_whilefixer(void)
{
  int whilecounter;
  int counter;

  while (1)
    whilecounter--;

  while (1)
    whilecounter++;

  while (1)
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
    a0++;

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
    a1++;
  else if (a1 == 0)
    b1--;
  else
  {

  }

  if (a1 > 10)
    a1++;

  if (b1 < 10)
    a1++;
  else if (a1 == 0)
  {
    b1--;
    if (a1 < 10)
      a1--;
    else if (b1 > 10)
      b1++;
  }
  else
  {

  }

}

void tdd_switchfixer_switchdffixer(void)
{
  int level = 10U;
  int a;
  int b;

  switch (level) {
  case 10U: {
    level++;
    level--;
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

void tdd_ifconstswapper(void)
{
  int a10;

  if (a10 = 10)
  {
    /*blah blah*/
  }

  if (a10 == MACRO1)
  {

  }

  if (MACRO1 == a10)
  {

  }
}
/**********************************************************************************************************************/
/**********************************************************************************************************************/
/*last line's intntionally left blank.*/
