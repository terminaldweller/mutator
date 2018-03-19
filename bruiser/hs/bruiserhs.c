
/***************************************************Project Mutator****************************************************/
/*first line intentionally left blank.*/
/*bruiser's lua asmrewriter implementation for jump tables*/
/*Copyright (C) 2018 Farzad Sadeghi

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.*/
/**********************************************************************************************************************/
#include <HsFFI.h>
#ifdef __GLASCOW_HASKELL__
#include "Safe_stub.h"
extern void __stginit_Safe(void);
#endif
#include "bruiserhs.h"
#include <stdio.h>

#pragma weak main
int main(int argc, char** argv) {
  int i;
  hs_init(&argc, &argv);
#ifdef __GLASCOW_HASKELL__
  hs_add_root(__stginit_Safe);
#endif

  i = fibonacci_hs(42);
  printf("Fibonnaci:%d\n", i);

  hs_exit();
  return 0;
}
/**********************************************************************************************************************/
/*last line intentionally left blank*/

