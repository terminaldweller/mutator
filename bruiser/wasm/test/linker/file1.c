#include <stdio.h>
#include <inttypes.h>
#if 1
void print(void) {
  printf("external symbol");
}
#endif

#define CODE_SECTION_1_0 "constant_1_0"
#define CODE_SECTION_1_1 "constant_1_1"
#define CODE_SECTION_1_2 "constant_1_2"
#define CODE_SECTION_1_3 "constant_1_3"
#define CODE_SECTION_1_4 "constant_1_4"

int g_int_1_0 = 10;
int g_int_1_1 = 11;

int dummy_f_1_0(int n) {
  if (n >= 1) return dummy_f_1_0(n-1) * n;
  else return 1;
}

int dymmy_f_1_1(int n) {
  return n*2;
}
