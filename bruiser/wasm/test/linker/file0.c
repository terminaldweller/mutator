#include <stdio.h>
#include <inttypes.h>
#include "file1.c"

#define CODE_SECTION_0_0 "constant_0_0"
#define CODE_SECTION_0_1 "constant_0_1"
#define CODE_SECTION_0_2 "constant_0_2"
#define CODE_SECTION_0_3 "constant_0_3"
#define CODE_SECTION_0_4 "constant_0_4"

int g_int_0_0 = 0;
int g_int_0_1 = 1;

int dummy_f_0_0(int n) {
  return n*4;
}

int main (int argc, char** argv) {
  return 123;
}
