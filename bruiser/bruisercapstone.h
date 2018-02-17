
/***************************************************Project Mutator****************************************************/
/*first line intentionally left blank.*/
/*bruiser's capstone side for rewriting xobjects*/
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
#include <capstone/capstone.h>
#include <keystone/keystone.h>
#include <stdint.h>
/**********************************************************************************************************************/
#ifndef BRUISER_CAPSTONE_H
#define BRUISER_CAPSTONE_H

#ifdef __cplusplus
extern "C" {
#endif

uint32_t get_textsection_length(void);
uintptr_t get_symbol_rt_address(const char* symbol_name);
void int2byte(int value, uint8_t* ret_value, size_t size);
void leb128_encode_s(int32_t value, uint8_t* ret_value, size_t size);
void leb128_encode_u(uint32_t value, uint8_t* ret_value, size_t size);
void leb128_decode_s(int32_t value, uint8_t* ret_value, size_t size);
void leb128_decode_u(uint32_t value, uint8_t* ret_value, size_t size);
int ks_write(ks_arch arch, int mode, const char* assembly, int syntax, unsigned char* encode);
int global_rewriter(int offset, size_t size, uint8_t* asm_code, const char* obj);
int call_rewriter(int offset, size_t size, uint8_t* asm_code, const char* obj);

#ifdef __cplusplus
}
#endif
#endif
/**********************************************************************************************************************/
/*last line intentionally left blank.*/

