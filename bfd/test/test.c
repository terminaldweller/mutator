#include "stdio.h"

int myfunc1(void) {return 1;}
int myfunc2(void) {return 2;}
int myfunc3(void) {return 3;}
int myfunc4(void) {return 4;}
int myfunc5(void) {return 5;}
int myfunc6(void) {return 6;}
int myfunc7(int a, int b) {return a + b;}

int myvar1 = 1;
int myvar2 = 2;
int myvar3 = 3;
int myvar4 = 4;

int main(int argc, char** argv) {
  int sum;
  sum = myfunc7(10, 20);
  printf("i live!\n");
  return myfunc7(10, 20);
}
