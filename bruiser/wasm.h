
/***************************************************Project Mutator****************************************************/
/*first line intentionally left blank.*/
/*bruiser's wasm interface implementation*/
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
/**********************************************************************************************************************/
#ifndef WASM_H
#define WASM_H
#include <inttypes.h>
#include "./lua-5.3.4/src/lua.h"
#include "./lua-5.3.4/src/lauxlib.h"
#include "./lua-5.3.4/src/lualib.h"

#ifdef __cplusplus
extern "C" {
#endif

//typedef varuint1 uint8_t;
//typedef varuint7 uint8_t;
//typedef varuint32 uint32_t;
//typedef varint1 int8_t;
//typedef varint7 int8_t;
//typedef varint32 int32_t;

typedef uint8_t varint1;
typedef uint8_t varint7;
typedef uint32_t varint32;
typedef int8_t varuint1;
typedef int8_t varuint7;
typedef int32_t varuint32;

  enum value_type_t {f64_vt = -4, f32_vt, i64_vt, i32_vt};
  enum external_kind_t {Function, Table, Memory, Global};
  enum type_ctor_t {i32_ctor = -1, i64_ctor = -2, f32_ctor = -3, f64_ctor = -4, anyfunc_ctor = -16, func_ctor = -32, block_type_ctor = -64};

  typedef struct {
    varuint32 size;
    char* code;
  }init_expr_t;

  typedef struct {
    varuint1 flags;
    varuint32 initial;
    varuint32 maximum;
  } resizable_limit_t;

  typedef struct {
    enum value_type_t value_type;
    varuint1 mutability;
  }global_type_t;

  typedef struct {
    varint7 element_type;
    resizable_limit_t* resizable_limit;
  }table_type_t;

  typedef struct {
    resizable_limit_t* resizable_limit;
  }memory_type_t;

  // func_type
  typedef struct {
    varint7 form;
    varuint32 param_count;
    varint7* param_types;
    varuint1 return_count;
    varint7 * return_types;
  }W_Type_Section_Entry;

  typedef struct {
    varuint32 count;
    W_Type_Section_Entry** entries;
  }W_Type_Section;

  typedef struct {
    varuint32 module_length;
    char* module_str;
    varuint32 field_len;
    char* field_str;
    enum external_kind_t kind;
    // based on external_kind it can be 4 different types. thats why im casting to void*.
    void* type;
  }W_Import_Section_Entry;

  typedef struct {
    int count;
    W_Import_Section_Entry** entries;
  }W_Import_Section;

  typedef struct {
    varuint32 count;
    // indexes into the type section
    varuint32* types;
  }W_Function_Section;

  typedef struct W_Table_Section {
    varuint32 count;
    table_type_t** entries;
  }W_Table_Section;

  typedef struct {
    varuint32 count;
    memory_type_t** entries;
  }W_Memory_Section;

  typedef struct {
    global_type_t* type;
    init_expr_t init;
  }W_Global_Entry;

  typedef struct {
    varuint32 count;
    W_Global_Entry** globals;
  }W_Global_Section;

  typedef struct {
    varuint32 field_len;
    char* field_str;
    enum external_kind_t kind;
    int varuint32;
  }W_Export_Entry;

  typedef struct {
    int count;
    W_Export_Entry** entries;
  }W_Export_Section;

  typedef struct {
    varuint32 index;
  }W_Start_Section;

  typedef struct {
    varuint32 index;
    init_expr_t offset;
    varuint32 num_length;
    varuint32* elems;
  }W_Elem_Segment;

  typedef struct {
    varuint32 count;
    W_Elem_Segment** entries;
  }W_Element_Section;

  typedef struct {
    varuint32 count;
    enum value_type_t type;
  }W_Local_Entry;

  typedef struct W_Function_Body {
    varuint32 body_size;
    varuint32 local_count;
    W_Local_Entry** locals;
    char* code;
    //char end = 0x0b;
  }W_Function_Body;

  typedef struct {
    varuint32 count;
    W_Function_Body** bodies;
  }W_Code_Section;

  typedef struct {
    varuint32 index;
    init_expr_t offset;
    varuint32 size;
    char* data;
  }W_Data_Segment;

  typedef struct {
    varuint32 count;
    struct W_Data_Segment** entries;
  }W_Data_Section;

#if 0
  typedef struct W_Custom_Section {};
  typedef struct W_Name_Section {};
  typedef struct W_Relocation_Section {};
#endif

  typedef struct Wasm_Module {
    W_Import_Section* import_section;
    W_Function_Section* function_section;
    W_Table_Section* table_section;
    W_Memory_Section* memory_section;
    W_Global_Section* global_section;
    W_Export_Section* export_section;
    W_Start_Section* start_section;
    W_Element_Section* element_section;
    W_Code_Section* code_section;
    W_Data_Section* data_section;
    void** W_Custom_Sections;
    char* name;
  }Wasm_Module;

  // get the raw binary of the wasm module
  // char* getWRaw();
  
  // get wasm section raw binary by name
  // char* get_W_Section_Raw(const char* section_name);

#ifdef __cplusplus
}
#endif // end of extern c
#endif // end of header guard
/**********************************************************************************************************************/
/*last line intentionally left blank.*/

