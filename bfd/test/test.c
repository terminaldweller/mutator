#include "stdio.h"

int myfunc1(void) {return 1;}
int myfunc2(void) {return 2;}
int myfunc3(void) {return 3;}
int myfunc4(void) {return 4;}
int myfunc5(void) {return 5;}
int myfunc6(void) {return 6;}
int add2(int a, int b) {return a + b;}
int sub2(int a, int b) {return a - b;}
double adddouble(double a, double b) {return a+b;}
double subdouble(double a, double b) {return a-b;}
double triple(double a, double b, double c) {return a+b+c;}
int quad(int a, int b, int c, int d) {return add2(a,b) + add2(c,d);}
const char* passthrough(const char* a) {return a;}
void ext_1(void) {printf("%s", "hey there sleepy-head.\n");}

int myvar1 = 1;
int myvar2 = 2;
int myvar3 = 3;
int myvar4 = 4;

int glob(void) {return myvar1+myvar2+myvar3+myvar4;}

int main(int argc, char** argv) {
  int sum;
  sum = add2(10, 20);
  printf("i live!\n");
  int res =  sub2(20, 10);
  ext_1();
  return quad(1,2,3,4);
}
