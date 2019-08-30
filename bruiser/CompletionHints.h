
/***************************************************Project Mutator****************************************************/
//-*-c++-*-
/*first line intentionally left blank.*/
/*the header for bruiser's hints and auto-completions*/
/*Copyright (C) 2017 Farzad Sadeghi

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 3
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.*/
/*code structure inspired by Eli Bendersky's tutorial on Rewriters.*/
/**********************************************************************************************************************/
/*inclusion guard*/
#ifndef COMPLETION_SUGGESTIONS_H
#define COMPLETION_SUGGESTIONS_H
/**********************************************************************************************************************/
/*included modules*/
/*project headers*/
#include "./linenoise/linenoise.h"
/*standard library headers*/
/*clang headers*/
/*llvm headers*/
/**********************************************************************************************************************/
/*using*/
/**********************************************************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

#define NELEMS(x) (sizeof(x)/sizeof(x[0]))

typedef struct{
  size_t begin;
  size_t end;
}word_pos_t;

void shell_completion(const char* buf, linenoiseCompletions* lc, size_t pos);
char* shell_hint(const char* buf, int* color, int* bold);
/**********************************************************************************************************************/
#ifdef __cplusplus
}
#endif

#endif
/*last line intentionally left blank*/

