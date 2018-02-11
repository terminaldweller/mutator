
/***************************************************Project Mutator****************************************************/
/*first line intentionally left blank.*/
/*bruiser's libffi side for calling xobjects*/
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
#include <ffi.h>
#include <stdint.h>
/**********************************************************************************************************************/
#ifndef BRUISER_FFI_H
#define BRUISER_FFI_H

#ifdef __cplusplus
extern "C" {
#endif

#define REINTERPRET_GENERATOR(X) \
  X ffi_reinterpret_##X(void* result);

#define X_LIST_GEN \
  X(uint8_t, "uint8_t")\
  X(uint16_t, "uint8_t")\
  X(uint32_t, "uint8_t")\
  X(uint64_t, "uint8_t")\
  X(int8_t, "uint8_t")\
  X(int16_t, "uint8_t")\
  X(int32_t, "uint8_t")\
  X(int64_t, "uint8_t")\
  X(uintptr_t, "uint8_t")\

#define X(X1,X2) REINTERPRET_GENERATOR(X1)
X_LIST_GEN
#undef X
#undef X_LIST_GEN
#undef REINTERPRET_GENERATOR
float ffi_reinterpret_float(void* result);
double ffi_reinterpret_double(void* result);
char* ffi_reinterpret_string(void* result);

  /**
   * @brief constructs the arguments to be passed to ffi_call.
   * @param ret the void** that will be passed to ffi_call.
   * @param argc number of the arguments the function that's gonna be called has.
   * @param ... pairs of the arg types in string format plus the references to the actual args themselves.
   */
void ffi_value_ctor(void** ret, int argc, ...);

/**
 * @brief returns the ffi_type of the given arg.
 * @param arg_string the type of the arg in string format.
 * @return the address of the corresponding ffi_type.
 */
ffi_type* ffi_type_ctor(const char* arg_string);

/**
 * @brief call the xobj.
 * @param argc number of args the xobj accepts.
 * @param arg_string a char** of the arg types.
 * @param rtype ffi_type of the return value.
 * @param x_ptr the void* xobj pointer.
 * @param values the actual args to be passed to ffi_call.
 * @return return a void* to the return value of the xobj.
 */
void* ffi_callX(int argc, const char** arg_string, ffi_type rtype, void* x_ptr, void** values);
void* ffi_callX_var(int argc, const char** arg_string, ffi_type rtype, void* x_ptr, void** values);
#ifdef __cplusplus
}
#endif
#endif
/**********************************************************************************************************************/
/*last line intentionally left blank.*/

