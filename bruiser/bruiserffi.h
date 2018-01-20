
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
#include <ffi.h>
/**********************************************************************************************************************/
#ifndef BRUISER_FFI_H
#define BRUISER_FFI_H

#ifdef __cplusplus
extern "C" {
#endif
ffi_type* ffi_type_ctor(const char* arg_string);
void* ffi_callX(int argc, const char** arg_string, ffi_type rtype, void* x_ptr, const char* ret_type);
void* ffi_callX_var(int argc, const char** arg_string, ffi_type rtype, void* x_ptr, const char* ret_type);
#ifdef __cplusplus
}
#endif
#endif
/**********************************************************************************************************************/
/*last line intentionally left blank.*/

