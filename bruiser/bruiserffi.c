
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
// @TODO-structs and unions not supported
// @TODO-vararg xobjs are not supported
/**********************************************************************************************************************/
#include <ffi.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <capstone/capstone.h>
#include "bruiserffi.h"
/**********************************************************************************************************************/
#define VOIDIFY(X) (void*)X
/**********************************************************************************************************************/
#pragma GCC diagnostic ignored "-Wpointer-to-int-cast"
#pragma GCC diagnostic push
#define REINTERPRET_GENERATOR(X) \
  X ffi_reinterpret_##X(void* result) {return (X)result;}

#define X_LIST_GEN \
  X(uint8_t, "for uint8_t")\
  X(uint16_t, "for uint16_t")\
  X(uint32_t, "for uint32_t")\
  X(uint64_t, "for uint64_t")\
  X(int8_t, "for int8_t")\
  X(int16_t, "for int16_t")\
  X(int32_t, "for int32_t")\
  X(int64_t, "for int64_t")\
  X(uintptr_t, "for pointers")\

#define X(X1,X2) REINTERPRET_GENERATOR(X1)
X_LIST_GEN
#undef X
#undef X_LIST_GEN
#undef REINTERPRET_GENERATOR
float ffi_reinterpret_float(void* result) {return *(float*)&result;}
double ffi_reinterpret_double(void* result) {return *(double*)&result;}
char* ffi_reinterpret_string(void* result) {return (char*)result;}

void ffi_value_ctor(void** ret, int argc, ...) {
  va_list value_list;
  char* arg_string;
  uint16_t counter = 0U;

  va_start(value_list, argc);
  for (int i = 0; i < argc; ++i) {
    arg_string = va_arg(value_list, char*);
    if (strcmp(arg_string, "uint8") == 0) {
      uint8_t* dummy = va_arg(value_list, uint8_t*);
      ret[counter] = VOIDIFY(dummy);
    }
    else if (strcmp(arg_string, "sint8") == 0) {
      int8_t* dummy = va_arg(value_list, int8_t*);
      ret[counter] = VOIDIFY(dummy);
    }
    else if (strcmp(arg_string, "uint16") == 0) {
      uint16_t* dummy = va_arg(value_list, uint16_t*);
      ret[counter] = VOIDIFY(dummy);
    }
    else if (strcmp(arg_string, "sint16") == 0) {
      int16_t* dummy = va_arg(value_list, int16_t*);
      ret[counter] = VOIDIFY(dummy);
    }
    else if (strcmp(arg_string, "uint32") == 0) {
      uint32_t* dummy = va_arg(value_list, uint32_t*);
      ret[counter] = VOIDIFY(dummy);
    }
    else if (strcmp(arg_string, "sint32") == 0) {
      int32_t* dummy = va_arg(value_list, int32_t*);
      ret[counter] = VOIDIFY(dummy);
    }
    else if (strcmp(arg_string, "uint64") == 0) {
      uint64_t* dummy = va_arg(value_list, uint64_t*);
      ret[counter] = VOIDIFY(dummy);
    }
    else if (strcmp(arg_string, "sint64") == 0) {
      int64_t* dummy = va_arg(value_list, int64_t*);
      ret[counter] = VOIDIFY(dummy);
    }
    else if (strcmp(arg_string, "float") == 0) {
      float* dummy = va_arg(value_list, float*);
      ret[counter] = dummy;
    }
    else if (strcmp(arg_string, "double") == 0) {
      double* dummy = va_arg(value_list, double*);
      ret[counter] = dummy;
    }
    else if (strcmp(arg_string, "pointer") == 0) {
      ret[counter] = va_arg(value_list, void*);
    }
    else {
      ret[counter] = NULL;
      fprintf(stderr, "got garbage arg value string...\n");
    }
    counter++;
  }
}

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
  else if (strcmp(arg_string, "string") == 0) {return &ffi_type_pointer;}
  // @DEVI-FIXME: currently we are not handling structs at all
  else if (strcmp(arg_string, "struct") == 0) {return &ffi_type_pointer;}
  else {
    fprintf(stderr, "garbage arg type was passed.\n");
    return NULL;
  }
}

void* ffi_callX(int argc, const char** arg_string, ffi_type rtype, void* x_ptr, void** values) {
  ffi_status status;
  ffi_cif cif;
  ffi_type* args_types[argc];
  void* ret;

  for (int i = 0; i < argc; ++i) {
    if (ffi_type_ctor(arg_string[i])) args_types[i] = ffi_type_ctor(arg_string[i]);
  }

  status = ffi_prep_cif(&cif, FFI_DEFAULT_ABI, argc, &rtype, args_types);
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

  ffi_call(&cif, FFI_FN(x_ptr), &ret, values);
  return ret;
}

void* ffi_callX_var(int argc, const char** arg_string, ffi_type rtype, void* x_ptr, void** values) {return NULL;}
/**********************************************************************************************************************/
/**********************************************************************************************************************/
// @DEVI-the following lines are only meant for testing.
uint32_t add2(uint32_t a, uint32_t b) {return a+b;}
uint32_t sub2(uint32_t a, uint32_t b) {return a-b;}
double addd(double a, double b) {return a+b;}
char* passthrough(char* a) {return a;}
#pragma weak main
int main(int argc, char** argv) {
  void* padd = &add2;
  void* psub = &sub2;
  void* padd2 = &addd;
  void* pstring = &passthrough;
  int argcount = 2;
  ffi_type ret_type = ffi_type_uint32;
  const char* args[] = {"uint32", "uint32"};
  const char* ret_string = "uint32";
  uint32_t a = 30;
  uint32_t b = 20;
  void* values[2];
  ffi_value_ctor(values, 2, "uint32", &a, "uint32", &b);

  void* result = ffi_callX(argcount, args, ret_type, padd, values);
  fprintf(stdout, "result of callling add is %d\n", (uint32_t)result);
  result = ffi_callX(argcount, args, ret_type, psub, values);
  fprintf(stdout, "result of calling sub is %d\n", (uint32_t)result);

  ret_type = ffi_type_double;
  double c = 111.111;
  double d = 111.111;
  const char* args2[] = {"double", "double"};
  void* values2[] = {&c, &d};
  result = ffi_callX(argcount, args2, ret_type, padd2, values2);
  fprintf(stdout, "result of calling addd is %f\n", ffi_reinterpret_double(result));
  const char* args3[] = {"string"};
  char* dummy = "i live!";
  void* values3[] = {&dummy};
  result = ffi_callX(1, args3, ffi_type_pointer, pstring, values3);
  fprintf(stdout, "result of calling passthrough is %s\n", ffi_reinterpret_string(result));

  return 0;
}
/**********************************************************************************************************************/
/*last line intentionally left blank.*/

