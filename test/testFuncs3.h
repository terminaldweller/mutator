
#if 1
#include <assert.h>
#include <complex.h>
#include <ctype.h>
#include <errno.h>
#include <fenv.h>
#include <float.h>
#include <inttypes.h>
#include <iso646.h>
#include <limits.h>
#include <locale.h>
#include <math.h>
#include <setjmp.h>
#include <signal.h>
#include <stdalign.h>
#include <stdarg.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <string.h>
#include <tgmath.h>
#include <time.h>
#include <uchar.h>
#include <wchar.h>
#include <wctype.h>
#endif

#define MACRO1 {{1,2},{3,4},{5,6}}

#define READ_TIME_32() \
do { \
} while (0)

#define READ_TIME_33() \
do { \
	while(a<10)\
		{}\
} while (0)

#define READ_TIME_34() \
do { \
	while(a>10)\
		{}\
} while (a < 10)

#define ANSWER (17U)
