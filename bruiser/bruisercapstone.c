
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
// main
# define CODE_3 "\x31\xed\x49\x89\xd1\x5e\x48\x89\xe2\x48\x83\xe4\xf0\x50\x54\x49\xc7\xc0\x60\x07\x40\x00\x48\xc7\xc1\xf0\x06\x40\x00\x48\xc7\xc7\x90\x06\x40\x00\xff\x15\xa6\x0b\x20\x00\xf4\x0f\x1f\x44\x00\x00\x55\xb8\x38\x10\x60\x00\x48\x3d\x38\x10\x60\x00\x48\x89\xe5\x74\x17\xb8\x00\x00\x00\x00\x48\x85\xc0\x74\x0d\x5d\xbf\x38\x10\x60\x00\xff\xe0\x0f\x1f\x44\x00\x00\x5d\xc3\x66\x0f\x1f\x44\x00\x00\xbe\x38\x10\x60\x00\x55\x48\x81\xee\x38\x10\x60\x00\x48\x89\xe5\x48\xc1\xfe\x03\x48\x89\xf0\x48\xc1\xe8\x3f\x48\x01\xc6\x48\xd1\xfe\x74\x15\xb8\x00\x00\x00\x00\x48\x85\xc0\x74\x0b\x5d\xbf\x38\x10\x60\x00\xff\xe0\x0f\x1f\x00\x5d\xc3\x66\x0f\x1f\x44\x00\x00\x80\x3d\x6d\x0b\x20\x00\x00\x75\x17\x55\x48\x89\xe5\xe8\x7e\xff\xff\xff\xc6\x05\x5b\x0b\x20\x00\x01\x5d\xc3\x0f\x1f\x44\x00\x00\xf3\xc3\x0f\x1f\x40\x00\x66\x2e\x0f\x1f\x84\x00\x00\x00\x00\x00\x55\x48\x89\xe5\x5d\xeb\x89\x66\x0f\x1f\x84\x00\x00\x00\x00\x00\x55\x48\x89\xe5\xb8\x01\x00\x00\x00\x5d\xc3\x0f\x1f\x44\x00\x00\x55\x48\x89\xe5\xb8\x02\x00\x00\x00\x5d\xc3\x0f\x1f\x44\x00\x00\x55\x48\x89\xe5\xb8\x03\x00\x00\x00\x5d\xc3\x0f\x1f\x44\x00\x00\x55\x48\x89\xe5\xb8\x04\x00\x00\x00\x5d\xc3\x0f\x1f\x44\x00\x00\x55\x48\x89\xe5\xb8\x05\x00\x00\x00\x5d\xc3\x0f\x1f\x44\x00\x00\x55\x48\x89\xe5\xb8\x06\x00\x00\x00\x5d\xc3\x0f\x1f\x44\x00\x00\x55\x48\x89\xe5\x89\x7d\xfc\x89\x75\xf8\x8b\x75\xfc\x03\x75\xf8\x89\xf0\x5d\xc3\x66\x66\x66\x2e\x0f\x1f\x84\x00\x00\x00\x00\x00\x55\x48\x89\xe5\x89\x7d\xfc\x89\x75\xf8\x8b\x75\xfc\x2b\x75\xf8\x89\xf0\x5d\xc3\x66\x66\x66\x2e\x0f\x1f\x84\x00\x00\x00\x00\x00\x55\x48\x89\xe5\xf2\x0f\x11\x45\xf8\xf2\x0f\x11\x4d\xf0\xf2\x0f\x10\x45\xf8\xf2\x0f\x58\x45\xf0\x5d\xc3\x66\x0f\x1f\x44\x00\x00\x55\x48\x89\xe5\xf2\x0f\x11\x45\xf8\xf2\x0f\x11\x4d\xf0\xf2\x0f\x10\x45\xf8\xf2\x0f\x5c\x45\xf0\x5d\xc3\x66\x0f\x1f\x44\x00\x00\x55\x48\x89\xe5\xf2\x0f\x11\x45\xf8\xf2\x0f\x11\x4d\xf0\xf2\x0f\x11\x55\xe8\xf2\x0f\x10\x45\xf8\xf2\x0f\x58\x45\xf0\xf2\x0f\x58\x45\xe8\x5d\xc3\x66\x66\x66\x2e\x0f\x1f\x84\x00\x00\x00\x00\x00\x55\x48\x89\xe5\x48\x83\xec\x20\x89\x7d\xfc\x89\x75\xf8\x89\x55\xf4\x89\x4d\xf0\x8b\x7d\xfc\x8b\x75\xf8\xe8\x31\xff\xff\xff\x8b\x7d\xf4\x8b\x75\xf0\x89\x45\xec\xe8\x23\xff\xff\xff\x8b\x4d\xec\x01\xc1\x89\xc8\x48\x83\xc4\x20\x5d\xc3\x66\x0f\x1f\x44\x00\x00\x55\x48\x89\xe5\x48\x89\x7d\xf8\x48\x8b\x45\xf8\x5d\xc3\x66\x90\x55\x48\x89\xe5\x48\x8d\x05\xc5\x09\x20\x00\x48\x8d\x0d\xba\x09\x20\x00\x48\x8d\x15\xaf\x09\x20\x00\x48\x8d\x35\xa4\x09\x20\x00\x8b\x3e\x03\x3a\x03\x39\x03\x38\x89\xf8\x5d\xc3\x0f\x1f\x40\x00\x55\x48\x89\xe5\x48\x83\xec\x20\xb8\x0a\x00\x00\x00\xb9\x14\x00\x00\x00\xc7\x45\xfc\x00\x00\x00\x00\x89\x7d\xf8\x48\x89\x75\xf0\x89\xc7\x89\xce\xe8\xa7\xfe\xff\xff\x48\x8d\x3d\xc0\x00\x00\x00\x89\x45\xec\xb0\x00\xe8\x46\xfd\xff\xff\xbf\x14\x00\x00\x00\xbe\x0a\x00\x00\x00\x89\x45\xe8\xe8\xa4\xfe\xff\xff\x48\x83\xc4\x20\x5d\xc3\x66\x2e\x0f\x1f\x84\x00\x00\x00\x00\x00\x0f\x1f\x40\x00\x41\x57\x41\x56\x49\x89\xd7\x41\x55\x41\x54\x4c\x8d\x25\xee\x06\x20\x00\x55\x48\x8d\x2d\xee\x06\x20\x00\x53\x41\x89\xfd\x49\x89\xf6\x4c\x29\xe5\x48\x83\xec\x08\x48\xc1\xfd\x03\xe8\xc7\xfc\xff\xff\x48\x85\xed\x74\x20\x31\xdb\x0f\x1f\x84\x00\x00\x00\x00\x00\x4c\x89\xfa\x4c\x89\xf6\x44\x89\xef\x41\xff\x14\xdc\x48\x83\xc3\x01\x48\x39\xdd\x75\xea\x48\x83\xc4\x08\x5b\x5d\x41\x5c\x41\x5d\x41\x5e\x41\x5f\xc3\x90\x66\x2e\x0f\x1f\x84\x00\x00\x00\x00\x00\xf3\xc3"
/**********************************************************************************************************************/
/**********************************************************************************************************************/
JMP_S_T* iter_next(JMP_S_T* arg) {return arg->next;}
JMP_S_T* iter_next_y(JMP_S_T* arg) {return arg->next_y;}
JMP_S_T* iter_next_n(JMP_S_T* arg) {return arg->next_n;}
/**********************************************************************************************************************/
/**********************************************************************************************************************/
/**********************************************************************************************************************/
/**********************************************************************************************************************/
uint32_t get_textsection_length(void) {return &edata-&etext;}
/**********************************************************************************************************************/
/**********************************************************************************************************************/
uintptr_t get_symbol_rt_address(const char* symbol_name) {return NULL;}
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
#if 1
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
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpointer-sign"
  count = cs_disasm(handle, obj, size, 0x0, 0, &insn);
#pragma GCC diagnostic pop
  printf("number of instructions: %zu.\n\n", count);
  cs_option(handle, CS_OPT_DETAIL, CS_OPT_ON);

  if (count > 0) {
    size_t j;
    for (j = 0; j < count; ++j) {
      printf(CYAN"%zu.\t"NORMAL, j);
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
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpointer-sign"
  count = cs_disasm(handle, obj, size, 0x0, 0, &insn);
#pragma GCC diagnostic pop
  printf("number of instructions: %zu.\n\n", count);
  cs_option(handle, CS_OPT_DETAIL, CS_OPT_ON);

  if (count > 0) {
    size_t j;
    for (j = 0; j < count; ++j) {
      printf(CYAN"%zu.\t"NORMAL, j);
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
        //@DEVI-call cant be the last instructino in a function assuming its well-formed
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
JMP_S_T* makejmptable(size_t size, uint8_t* obj, bool Verbose) {
  csh handle;
  cs_insn* insn;
  size_t count;
  uint8_t rewritten[16];
  uint8_t code[16];
  size_t size_counter = 0;

  JMP_S_T* head = malloc(sizeof(JMP_S_T));
  JMP_S_T* tail = malloc(sizeof(JMP_S_T));
  head->type = NONE;
  head->next = NULL;
  tail = head;

  if (cs_open(CS_ARCH_X86, CS_MODE_64, &handle) != CS_ERR_OK) return NULL;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpointer-sign"
  count = cs_disasm(handle, obj, size, 0x0, 0, &insn);
#pragma GCC diagnostic pop
  if (Verbose) printf("number of instructions: %zu.\n\n", count);
  cs_option(handle, CS_OPT_DETAIL, CS_OPT_ON);

  intmax_t address;
  if (count > 0) {
    size_t j;
    for (j = 0; j < count; ++j) {
      if (Verbose) printf(CYAN"%zu.\t"NORMAL, j);
      if (Verbose) printf(GREEN"0x%"PRIx64":\t%s""\t\t%s\t"NORMAL, insn[j].address, insn[j].mnemonic, insn[j].op_str);
      if (Verbose) for (int i = 0; i < 16; ++i) {code[i] = insn[j].bytes[i]; printf(BLUE"%02x "NORMAL, code[i]);}
      if (Verbose) printf("\n");

      if (strcmp(insn[j].mnemonic, "jmp") == 0) {
        char* endptr;
        address = strtoumax(insn[j].op_str, &endptr, 0);
#if 1
        if (Verbose) printf(RED"found a jmp\n");
        if (Verbose) for (int i = 0; i < 16; ++i) {code[i] = insn[j].bytes[i]; printf(RED"%02x "NORMAL, code[i]);}
        if (Verbose) printf("\n");
        if (Verbose) printf(RED"%jx\n", address);
        if (Verbose) printf(RED"%d\n", insn[j].size);
#endif
        JMP_S_T* dummy = malloc(sizeof(JMP_S_T));
        dummy->location = insn[j].address;
        dummy->type = JMP;
        dummy->address = address;
        dummy->size = insn[j].size;
        dummy->next = NULL;
        tail->next = dummy;
        tail = dummy;
      }

      if (strcmp(insn[j].mnemonic, "je") == 0) {
        char* endptr;
        address = strtoimax(insn[j].op_str, &endptr, 0);
#if 1
        if (Verbose) printf(RED"found a je\n");
        if (Verbose) for (int i = 0; i < 16; ++i) {code[i] = insn[j].bytes[i]; printf(RED"%02x "NORMAL, code[i]);}
        if (Verbose) printf("\n");
        if (Verbose) printf(RED"%jx\n", address);
        if (Verbose) printf(RED"%d\n", insn[j].size);
#endif
        JMP_S_T* dummy = malloc(sizeof(JMP_S_T));
        dummy->location = insn[j].address;
        dummy->type = JE;
        dummy->address_y = address;
        dummy->size = insn[j].size;
        dummy->next = NULL;
        tail->next = dummy;
        tail = dummy;
      }

      if (strcmp(insn[j].mnemonic, "jne") == 0) {
        char* endptr;
        address = strtoimax(insn[j].op_str, &endptr, 0);
#if 1
        if (Verbose) printf(RED"found a jne\n");
        if (Verbose) for (int i = 0; i < 16; ++i) {code[i] = insn[j].bytes[i]; printf(RED"%02x "NORMAL, code[i]);}
        if (Verbose) printf("\n");
        if (Verbose) printf(RED"%lx\n", address);
        if (Verbose) printf(RED"%d\n", insn[j].size);
#endif
        JMP_S_T* dummy = malloc(sizeof(JMP_S_T));
        dummy->location = insn[j].address;
        dummy->type = JNE;
        dummy->address_y = address;
        dummy->size = insn[j].size;
        dummy->next = NULL;
        tail->next = dummy;
        tail = dummy;
      }

#if 0
      for (int i = 0; i < insn[j].size; ++i) {
        asm_code[size_counter] = insn[j].bytes[i];
        size_counter++;
      }
#endif
    }

    cs_free(insn, count);
  } else {
    printf("ERROR!!!\n");
  }
  cs_close(&handle);
  return head;
}
/**********************************************************************************************************************/
int freejmptable(JMP_S_T* _head) {
  JMP_S_T* previous = _head;
  JMP_S_T* current = _head;
  while (current != NULL) {
    previous = current;
    current = current->next;
    free(previous);
  }
  return 0;
}
/**********************************************************************************************************************/
int dumpjmptable(JMP_S_T* current) {
  while (current != NULL) {
    printf("jump location: %ld", current->location);
    printf("\tjump address: %lu", current->address);
    printf("\tjump type: %d", current->type);
    printf("\tjump next: %x", &current->next);
    printf("\tinstruction size: %d\n", current->size);
    current = current->next;
  }
}
/**********************************************************************************************************************/
void jmprewriter_j(JMP_S_T* jmp, uint8_t* code, JMP_T type, uint8_t* rewritten) {
  
}
void jmprewriter_jne(JMP_S_T* jmp, uint8_t* code, JMP_T type, uint8_t* rewritten) {};
void jmprewriter_je(JMP_S_T* jmp, uint8_t* code, JMP_T type, uint8_t* rewritten) {}
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

#if 1
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
#endif

  unsigned char* encode;
  ks_write(KS_ARCH_X86, KS_MODE_64, "add rax, rcx", 0, encode);
  ks_free(encode);

#if 0
  head = malloc(sizeof(JMP_S_T));
  tail = malloc(sizeof(JMP_S_T));
  head->type = NONE;
  head->next = NULL;
  tail = head;
#endif
  uint8_t asm_code3[834];
  JMP_S_T* current = makejmptable(834, CODE_3, true);

#if 0
  while (current != NULL) {
    printf("jump location: %lx", current->location);
    printf("\tjump address: %lu", current->address);
    printf("\tjump type: %d", current->type);
    printf("\tinstruction size: %d\n", current->size);
    current = current->next;
  }
#endif
  dumpjmptable(current);
  freejmptable(current);

  return 0;
}
/**********************************************************************************************************************/
/*last line intentionally left blank.*/

