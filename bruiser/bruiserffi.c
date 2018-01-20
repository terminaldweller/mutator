
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
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "bruiserffi.h"
/**********************************************************************************************************************/
ffi_type* ffi_type_ctor(const char* arg_string) {
  if (strcmp(arg_string, "void") == 0) {return &ffi_type_void;}
  else if (strcmp(arg_string, "uint8") == 0) {return &ffi_type_uint8;}
  else if (strcmp(arg_string, "sint8") == 0) {return &ffi_type_sint8;}
  else if (strcmp(arg_string, "uint16") == 0) {return &ffi_type_uint16;}
  else if (strcmp(arg_string, "sint16") == 0) {return &ffi_type_sint16;}
  else if (strcmp(arg_string, "uint32") == 0) {return &ffi_type_uint32;}
  else if (strcmp(arg_string, "sint32") == 0) {return &ffi_type_sint32;}
  else if (strcmp(arg_string, "uint64") == 0) {return &ffi_type_uint64;}
  else if (strcmp(arg_string, "sint64") == 0) {return &ffi_type_sint64;}
  else if (strcmp(arg_string, "float") == 0) {return &ffi_type_float;}
  else if (strcmp(arg_string, "double") == 0) {return &ffi_type_double;}
  else if (strcmp(arg_string, "pointer") == 0) {return &ffi_type_pointer;}
  // @DEVI-FIXME: currently we are not handling structs at all
  else if (strcmp(arg_string, "struct") == 0) {return &ffi_type_pointer;}
  else {
    fprintf(stderr, "garbage arg type was passed.\n");
    return NULL;
  }
}

void* ffi_callX(int argc, const char** arg_string, ffi_type rtype, void* x_ptr, const char* ret_type) {
  ffi_status status;
  ffi_cif cif;
  ffi_type* args_types[argc];
  for (int i = 0; i < argc; ++i) {
    if (ffi_type_ctor(arg_string[i])) args_types[i] = ffi_type_ctor(arg_string[i]);
  }

  //status = ffi_prep_cif(&cif, FFI_DEFAULT_ABI, argc, &rtype, args);
  status = ffi_prep_cif(&cif, FFI_DEFAULT_ABI, argc, &ffi_type_uint32, args_types);
  if (status == FFI_BAD_TYPEDEF) {
    fprintf(stderr, "ffi_prep_cif returned FFI_BAD_TYPEDEF: %d\n", status);
    return NULL;
  } else if (status == FFI_BAD_ABI) {
    fprintf(stderr, "ffi_prep_cif returned FFI_BAD_ABI: %d\n", status);
    return NULL;
  } else if (status == FFI_OK) {
    fprintf(stderr, "ffi_prep_cif returned FFI_OK: %d\n", status);
  } else {
    fprintf(stderr, "ffi_prep_cif returned an error: %d\n", status);
    return NULL;
  }

  uint32_t a = 30;
  uint32_t b = 20;
  void* ret;
  //void* values[argc]; //FIXME the actual arguments
  void* values[2] = {&a, &b};
  ffi_call(&cif, FFI_FN(x_ptr), &ret, values);
  return ret;
}

void* ffi_callX_var(int argc, const char** arg_string, ffi_type rtype, void* x_ptr, const char* ret_type) {}
/**********************************************************************************************************************/
// @DEVI-the following lines are only meant for testing.
uint32_t add2(uint32_t a, uint32_t b) {return a+b;}
uint32_t sub2(uint32_t a, uint32_t b) {return a-b;}
int main(int argc, char** argv) {
  void* padd = &add2;
  void* psub = &sub2;
  int argcount = 2;
  ffi_type ret_type = ffi_type_uint32;
  const char* args[] = {"uint32", "uint32"};
  const char* ret_string = "uint32";

  void* result = ffi_callX(argcount, args, ret_type, psub, ret_string);
  fprintf(stdout, "first result %d\n", (uint32_t)result);
  result = ffi_callX(argcount, args, ret_type, padd, ret_string);
  fprintf(stdout, "first result %d\n", (uint32_t)result);
  return 0;
}
/**********************************************************************************************************************/
/*last line intentionally left blank.*/

