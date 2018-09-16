
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/resource.h>

#include "./structs.h"
#include "./read.h"
#include "./aggregate.h"


#pragma weak main
int main (int argc, char** argv) {
  const rlim_t kStackSize = 160 * 1024 * 1024;   // min stack size = 16 MB
  struct rlimit rl;
  int result;

  result = getrlimit(RLIMIT_STACK, &rl);
  if (result == 0)
  {
      if (rl.rlim_cur < kStackSize)
      {
          rl.rlim_cur = kStackSize;
          result = setrlimit(RLIMIT_STACK, &rl);
          if (result != 0)
          {
              fprintf(stderr, "setrlimit returned result = %d\n", result);
          }
      }
  }
  int wasm = open("./test.wasm", O_RDONLY);
  wasm_lib_ret_t* lib_ret = read_aggr_wasm(wasm);
  printf("finished reading\n");

#if 0
  printf("magic_number:%x\n", lib_ret->obj->magic_number_container->magic_number);
  printf("version:%x\n", lib_ret->obj->version_container->version);

  printf("type section id:%d\n", lib_ret->obj->W_Type_Section_container->id);
  printf("type section payloadlength:%d\n", lib_ret->obj->W_Type_Section_container->payloadlength);
  printf("type_section entry count:%d\n", lib_ret->obj->W_Type_Section_container->count);
  for (int i=0; i < lib_ret->obj->W_Type_Section_container->count; ++i) {
    printf("param_count:%d\n",lib_ret->obj->W_Type_Section_container->entries[i]->param_count);
    for (int j = 0; j < lib_ret->obj->W_Type_Section_container->entries[i]->param_count; ++j)
      printf("param_types:%d\n",lib_ret->obj->W_Type_Section_container->entries[i]->param_types[j]);
    printf("return_count:%d\n", lib_ret->obj->W_Type_Section_container->entries[i]->return_count);
    for (int j = 0; j < lib_ret->obj->W_Type_Section_container->entries[i]->return_count; ++j)
      printf("param_types:%d\n",lib_ret->obj->W_Type_Section_container->entries[i]->return_types[j]);
  }
  printf("import_section_id:%d\n", lib_ret->obj->W_Import_Section_container->id);
  printf("import_section_payloadlength:%d\n", lib_ret->obj->W_Import_Section_container->payloadlength);
  printf("import_section_count:%d\n", lib_ret->obj->W_Import_Section_container->count);
  for(int i = 0; i < lib_ret->obj->W_Import_Section_container->count; ++i) {
    printf("module_length:%d\n", lib_ret->obj->W_Import_Section_container->entries[i]->module_length);
    printf("module_str:%s\n", lib_ret->obj->W_Import_Section_container->entries[i]->module_str);
    printf("field_length:%d\n", lib_ret->obj->W_Import_Section_container->entries[i]->field_len);
    printf("field_str:%s\n", lib_ret->obj->W_Import_Section_container->entries[i]->field_str);
    printf("kind:%d\n", lib_ret->obj->W_Import_Section_container->entries[i]->kind);
    if (lib_ret->obj->W_Import_Section_container->entries[i]->kind == 0)
      printf("type:%d\n", lib_ret->obj->W_Import_Section_container->entries[i]->kind);
    printf("\n");
  }
  printf("function_section_id:%d\n", lib_ret->obj->W_Function_Section_container->id);
  printf("function_section_payloadlength:%d\n", lib_ret->obj->W_Function_Section_container->payloadlength);
  printf("function_section_count:%d\n", lib_ret->obj->W_Function_Section_container->count);
  for (int i = 0; i < lib_ret->obj->W_Function_Section_container->count; ++i)
    printf("type:%d\n", lib_ret->obj->W_Function_Section_container->types[i]);

  printf("table_section_id:%d\n", lib_ret->obj->W_Table_Section_container->id);
  printf("table_section_payloadlength:%d\n", lib_ret->obj->W_Table_Section_container->payloadlength);
  printf("table_section_count:%d\n", lib_ret->obj->W_Table_Section_container->count);
  for (int i = 0; i < lib_ret->obj->W_Table_Section_container->count; ++i) {
    printf("element_type:%d\n", lib_ret->obj->W_Table_Section_container->entries[i]->element_type);
    printf("rl_flags:%d\n", lib_ret->obj->W_Table_Section_container->entries[i]->resizable_limit->flags);
    printf("rl_initial:%d\n", lib_ret->obj->W_Table_Section_container->entries[i]->resizable_limit->initial);
    printf("rl_maximum:%d\n", lib_ret->obj->W_Table_Section_container->entries[i]->resizable_limit->maximum);
  }

  printf("memory_section_id:%d\n", lib_ret->obj->W_Memory_Section_container->id);
  printf("memory_section_payload_length:%d\n", lib_ret->obj->W_Memory_Section_container->payloadlength);
  printf("rl_flags:%d\n", lib_ret->obj->W_Memory_Section_container->entries->resizable_limit->flags);
  printf("rl_initial:%d\n", lib_ret->obj->W_Memory_Section_container->entries->resizable_limit->initial);
  printf("rl_maximum:%d\n", lib_ret->obj->W_Memory_Section_container->entries->resizable_limit->maximum);

  if (lib_ret->obj->W_Global_Section_container == NULL) printf("global section doesnt exist.\n");

  printf("export_section_id:%d\n", lib_ret->obj->W_Export_Section_container->id);
  printf("export_section_payloadlength:%d\n", lib_ret->obj->W_Export_Section_container->payloadlength);
  printf("entry count:%d\n", lib_ret->obj->W_Export_Section_container->count);

  for (int i = 0; i < lib_ret->obj->W_Export_Section_container->count; ++i) {
    printf("field_len:%d\n", lib_ret->obj->W_Export_Section_container->entries[i]->field_len);
    printf("field_str:%s\n", lib_ret->obj->W_Export_Section_container->entries[i]->field_str);
    printf("kind:%d\n", lib_ret->obj->W_Export_Section_container->entries[i]->kind);
    printf("index:%d\n", lib_ret->obj->W_Export_Section_container->entries[i]->index);
  }

  if (lib_ret->obj->W_Start_Section_container == NULL) printf("start section doesnt exist.\n");

  printf("element_seciton_id:%d\n", lib_ret->obj->W_Element_Section_container->id);
  printf("element_section_payloadlength:%d\n", lib_ret->obj->W_Element_Section_container->payloadlength);
  printf("entry count:%d\n", lib_ret->obj->W_Element_Section_container->count);

  for (int i = 0; i < lib_ret->obj->W_Element_Section_container->count; ++i) {
    printf("index:%d\n", lib_ret->obj->W_Element_Section_container->entries[i]->index);
    for (int j = 0; j < 3; ++j) {
      printf("code:%d\n", lib_ret->obj->W_Element_Section_container->entries[i]->init->code[j]);
    }
    printf("num_length:%d\n", lib_ret->obj->W_Element_Section_container->entries[i]->num_length);
    for (int j = 0; j < lib_ret->obj->W_Element_Section_container->entries[i]->num_length; ++j) {
      printf("elems:%d\n", lib_ret->obj->W_Element_Section_container->entries[i]->elems[j]);
    }
  }

  printf("code_section_id:%d\n", lib_ret->obj->W_Code_Section_container->id);
  printf("code_section_payloadlength:%d\n", lib_ret->obj->W_Code_Section_container->payloadlength);
  printf("count:%d\n", lib_ret->obj->W_Code_Section_container->count);

  for (int i = 0; i < lib_ret->obj->W_Code_Section_container->count; ++i) {
    printf("body_size:%d\n", lib_ret->obj->W_Code_Section_container->bodies[i]->body_size);
    printf("local_count:%d\n", lib_ret->obj->W_Code_Section_container->bodies[i]->local_count);
    if (lib_ret->obj->W_Code_Section_container->bodies[i]->local_count > 0) {
      for (int j =0; j < lib_ret->obj->W_Code_Section_container->bodies[i]->local_count; ++j) {
        for (int k = 0; k < lib_ret->obj->W_Code_Section_container->bodies[i]->locals[j]->count; ++k) {
        }
      }
    }
    printf("code:\n");
    for (int j = 0; j < lib_ret->obj->W_Code_Section_container->bodies[i]->body_size; ++j) {
      printf("%02x ", lib_ret->obj->W_Code_Section_container->bodies[i]->code[j]);
    }
    printf("\n");
  }

  printf("data_section_id:%d\n", lib_ret->obj->W_Data_Section_container->id);
  printf("data_section_payloadlength:%d\n", lib_ret->obj->W_Data_Section_container->payloadlength);
  printf("data seg count:%d\n", lib_ret->obj->W_Data_Section_container->count);

  for (int i = 0; i < lib_ret->obj->W_Data_Section_container->count; ++i) {
    printf("index:%d\n", lib_ret->obj->W_Data_Section_container->entries[i]->index);
    printf("size:%d\n", lib_ret->obj->W_Data_Section_container->entries[i]->size);
    printf("code:\n");
    for (int j = 0; j < lib_ret->obj->W_Data_Section_container->entries[i]->size; ++j) {
      printf("%c ", lib_ret->obj->W_Data_Section_container->entries[i]->data[j]);
    }
    printf("\n");
    int j = 0;
    printf("offset:\n");
    while(1) {
      printf("%02x ", lib_ret->obj->W_Data_Section_container->entries[i]->offset->code[j]);
      if (lib_ret->obj->W_Data_Section_container->entries[i]->offset->code[j] == 11) {
        break;
      }
      j++;
    }
    printf("\n");
  }
#endif

  printf("sizeof magic:%d\n", sizeof(magic_number));
  printf("sizeof version:%d\n", sizeof(version));
  printf("current void count:%d\n", lib_ret->current_void_count);
  printf("void_train first:0x%x\n", lib_ret->void_train[0]);
  printf("void_train first:0x%x\n", lib_ret->void_train[1]);
  printf("void_train self address:0x%x\n", lib_ret->void_train);
  //free(lib_ret->void_train[0]);
  //release_all(lib_ret->void_train, lib_ret->current_void_count);
  //free(lib_ret->void_train[2]);
  //free(lib_ret->void_train[1]);
  //free(lib_ret->void_train[0]);
  for (int i = lib_ret->current_void_count - 1; i >= 0; --i) {
    printf("%d:0x%x ", i, lib_ret->void_train[i]);
    //if (i == 1) continue;
    free(lib_ret->void_train[i]);
  }
  free(lib_ret->void_train);
  free(lib_ret->obj);
  free(lib_ret);
  return 0;
}
