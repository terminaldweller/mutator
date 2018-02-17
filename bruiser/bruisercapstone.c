
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
/**********************************************************************************************************************/
#include "./bruisercapstone.h"
#include "./devi_extra.h"
#include <capstone/capstone.h>
#include <errno.h>
#include <inttypes.h>
#include <keystone/keystone.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
/**********************************************************************************************************************/
/**********************************************************************************************************************/
extern char etext, edata, end;
// quad
#define CODE_1 "\x55\x48\x89\xe5\x48\x83\xec\x20\x89\x7d\xfc\x89\x75\xf8\x89\x55\xf4\x89\x4d\xf0\x8b\x7d\xfc\x8b\x75\xf8\xe8\xd1\xfd\xff\xff\x8b\x7d\xf4\x8b\x75\xf0\x89\x45\xec\xe8\xc3\xfd\xff\xff\x8b\x4d\xec\x1\xc1\x89\xc8\x48\x83\xc4\x20\x5d\xc3"
// glob
#define CODE_2 "\x55\x48\x89\xe5\x48\x8b\x05\x0d\x15\x20\x00\x48\x8b\x0d\xee\x14\x20\x00\x48\x8b\x15\xf7\x14\x20\x00\x48\x8b\x35\xd8\x14\x20\x00\x8b\x3e\x03\x3a\x03\x39\x03\x38\x89\xf8\x5d\xc3"
/**********************************************************************************************************************/
/**********************************************************************************************************************/
uint32_t get_textsection_length(void) {return &edata-&etext;}
/**********************************************************************************************************************/
/**********************************************************************************************************************/
uintptr_t get_symbol_rt_address(const char* symbol_name) {}
/**********************************************************************************************************************/
/**********************************************************************************************************************/
void int2byte(int value, uint8_t* ret_value, size_t size) {
  for (int i = 0; i < size; ++i) {
    ret_value[i] = (value & (0xff << (8*i))) >> (8*i);
  }
}
/**********************************************************************************************************************/
/**********************************************************************************************************************/
void leb128_encode_s(int32_t value, uint8_t* ret_value, size_t size) {
  uint8_t dummy;
  if (value == 0) {for (int i = 0; i < size; ++i) ret_value[i] = 0;}
  for (int i = 0; i < size; ++i) {
    dummy = value & 0x7f;
    ret_value[i] = dummy | 0x80;
    value >>=7;
    if (((value == 0) && ((dummy & 0x40) == 0)) || ((value == -1) && (dummy & 0x40))) {
      ret_value[size - 1] ^= 0x80;
      break;
    }
  }
}
/**********************************************************************************************************************/
/**********************************************************************************************************************/
void leb128_encode_u(uint32_t value, uint8_t* ret_value, size_t size) {
  uint8_t dummy;
  if (value == 0) {for (int i = 0; i < size; ++i) ret_value[i] = 0;}
  for (int i = 0; i < size; ++i) {
    dummy = value & 0x7f;
    ret_value[i] = dummy | 0x80;
    value >>=7;
  }
  ret_value[size - 1] ^= 0x80;
}
/**********************************************************************************************************************/
/**********************************************************************************************************************/
void leb128_decode_s(int32_t value, uint8_t* ret_value, size_t size) {}
/**********************************************************************************************************************/
/**********************************************************************************************************************/
void leb128_decode_u(uint32_t value, uint8_t* ret_value, size_t size) {}
/**********************************************************************************************************************/
/**********************************************************************************************************************/
int ks_write(ks_arch arch, int mode, const char* assembly, int syntax, unsigned char* encode) {
  ks_engine* ks;
  ks_err err = KS_ERR_ARCH;
  size_t count;
  size_t size;

  err = ks_open(arch, mode, &ks);
  if (err != KS_ERR_OK) {printf("failed on ks_open().\n"); return -1;}
  if (syntax) ks_option(ks, KS_OPT_SYNTAX, syntax);

  if (ks_asm(ks, assembly, 0, &encode, &size, &count)) {printf("errored out\n"); return -1;}
#if 0
  else {
    printf("%s =", assembly);
    for (size_t i = 0; i < size; ++i) {
      printf("%02x ", encode[i]);
    }
    printf("\n");
  }
#endif

  ks_close(ks);
  return size;
}
/**********************************************************************************************************************/
/**********************************************************************************************************************/
int global_rewriter(int offset, size_t size, uint8_t* asm_code, const char* obj) {
  csh handle;
  cs_insn* insn;
  size_t count;
  uint8_t code[16];
  size_t size_counter = 0;
  unsigned char *encode;

  if (cs_open(CS_ARCH_X86, CS_MODE_64, &handle) != CS_ERR_OK) return -1;
  count = cs_disasm(handle, obj, size, 0x0, 0, &insn);
  printf("number of instructions: %d.\n\n", count);
  cs_option(handle, CS_OPT_DETAIL, CS_OPT_ON);

  if (count > 0) {
    size_t j;
    for (j = 0; j < count; ++j) {
      printf(CYAN"%d.\t"NORMAL, j);
      printf(GREEN"0x%"PRIx64":\t%s\t\t%s\t"NORMAL, insn[j].address, insn[j].mnemonic, insn[j].op_str);
      printf(BLUE"insn size: %d\n"NORMAL, insn[j].size);
      //for (int i = 0; i < 16; ++i) {code[i] = insn[j].bytes[i]; printf("%02x ", code[i]);}
      //printf("\n");

      if (strcmp(insn[j].mnemonic, "mov") == 0) {
      }

      for (int i = 0; i < insn[j].size; ++i) {
          asm_code[size_counter] = insn[j].bytes[i];
          size_counter++;
      }
    }

    cs_free(insn, count);
  } else {
    printf("ERROR!!!\n");
  }
  cs_close(&handle);
  return size_counter;
}
/**********************************************************************************************************************/
/**********************************************************************************************************************/
int call_rewriter(int offset, size_t size, uint8_t* asm_code, const char* obj) {
  csh handle;
  cs_insn* insn;
  size_t count;
  uint8_t rewritten[16];
  uint8_t code[16];
  size_t size_counter = 0;

  if (cs_open(CS_ARCH_X86, CS_MODE_64, &handle) != CS_ERR_OK) return -1;
  count = cs_disasm(handle, obj, size, 0x0, 0, &insn);
  printf("number of instructions: %d.\n\n", count);
  cs_option(handle, CS_OPT_DETAIL, CS_OPT_ON);

  if (count > 0) {
    size_t j;
    for (j = 0; j < count; ++j) {
      printf(CYAN"%d.\t"NORMAL, j);
      printf(GREEN"0x%"PRIx64":\t%s""\t\t%s\t"NORMAL, insn[j].address, insn[j].mnemonic, insn[j].op_str);
      for (int i = 0; i < 16; ++i) {code[i] = insn[j].bytes[i]; printf(BLUE"%02x "NORMAL, code[i]);}
      printf("\n");

      if (strcmp(insn[j].mnemonic, "call") == 0) {
        char* endptr;
        intmax_t address = strtoimax(insn[j].op_str, &endptr, 0);
        uintmax_t uaddress = strtoumax(insn[j].op_str, &endptr, 0);
        // rewriting
        asm_code[size_counter] = 0xe8, size_counter++;
        uint8_t temp[4];
        //@DEVI-call cant be the last instructino in a function
        int2byte(offset + insn[j].address, temp, 4);
        for (int i = 0; i < 4; ++i) {asm_code[size_counter] = temp[i]; size_counter++;}
        continue;
      }
      for (int i = 0; i < insn[j].size; ++i) {
        asm_code[size_counter] = insn[j].bytes[i];
        size_counter++;
      }
    }

    cs_free(insn, count);
  } else {
    printf("ERROR!!!\n");
  }
  cs_close(&handle);
  return size_counter;
}
/**********************************************************************************************************************/
/**********************************************************************************************************************/
// @DEVI-the following lines are only meant for testing.
#pragma weak main
int main(int argc, char** argv) {
  uint8_t asm_code[58];
  uint8_t asm_code2[44];
  printf("----------------------------------------------------------\n");
  printf("call_rewriter:\n");
  int new_size = call_rewriter(-528, 58, asm_code, CODE_1);
  printf("new size is %d.\n", new_size);
  for (int i = new_size; i < 58; ++i) {asm_code[i] = 0;}
  for (int i = 0; i < 58; ++i) {printf("%02x ", asm_code[i]);}

  printf("\n----------------------------------------------------------\n");
  printf("global_rewriter:\n");
  new_size = global_rewriter(-528, 44, asm_code2, CODE_2);
  printf("new size is %d.\n", new_size);
  for (int i = new_size; i < 44; ++i) {asm_code2[i] = 0;}
  for (int i = 0; i < 44; ++i) {printf("%02x ", asm_code2[i]);}
  
  printf("\n----------------------------------------------------------\n");
  printf("etext: %10p\n", &etext);
  printf("edata: %10p\n", &edata);
  printf("end: %10p\n", &end);
  printf("text section length: %d\n", get_textsection_length());

  printf("----------------------------------------------------------\n");
  uint8_t value[4];
  int2byte(-528, value, 4);
  for (int i = 0; i < 4; ++i) {printf("%02x ", value[i]);}
  printf("\n");
  leb128_encode_u(528, value, 4);
  for (int i = 0; i < 4; ++i) {printf("%02x ", value[i]);}
  printf("\n");
  leb128_encode_s(-528, value, 4);
  for (int i = 0; i < 4; ++i) {printf("%02x ", value[i]);}
  printf("\n");
  printf("----------------------------------------------------------\n");

  unsigned char* encode;
  ks_write(KS_ARCH_X86, KS_MODE_64, "add rax, rcx", 0, encode);
  ks_free(encode);

  return 0;
}
/**********************************************************************************************************************/
/*last line intentionally left blank.*/

