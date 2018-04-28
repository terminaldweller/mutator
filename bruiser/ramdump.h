
/***************************************************Project Mutator****************************************************/
/*first line intentionally left blank.*/
/*bruiser's ram dump module*/
/*Copyright (C) 2018 Farzad Sadeghi

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
/***********************************************************************************************************/
#ifndef RAMDUMP_H
#define RAMDUMP_H
#include <inttypes.h>
#include <stdio.h>
#ifdef __cplusplus
extern "C" {
#endif
void dump_memory_region(FILE* pMemFile, uint64_t start_address, uint64_t length);
void dump_ram(unsigned int pid, FILE* out_file);
#ifdef __cplusplus
}
#endif
#endif
/***************************************************Project Mutator****************************************************/
/*last line intentionally left blank.*/

