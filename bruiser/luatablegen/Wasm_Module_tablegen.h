
// automatically generated by luatablegen
#include "../lua-5.3.4/src/lua.h"
#include "../lua-5.3.4/src/lauxlib.h"
#include "../lua-5.3.4/src/lualib.h"
#include <inttypes.h>
#include <stdbool.h>

#ifndef _Wasm_Module_H
#define _Wasm_Module_H
#ifdef __cplusplus
extern "C" {
#endif

#include "../wasm.h"

static Wasm_Module* convert_Wasm_Module (lua_State* __ls, int index);
static Wasm_Module* check_Wasm_Module(lua_State* __ls, int index);
Wasm_Module* push_Wasm_Module(lua_State* __ls);
int Wasm_Module_push_args(lua_State* __ls, Wasm_Module* _st);
int new_Wasm_Module(lua_State* __ls);
static int getter_Wasm_Module_type_section(lua_State* __ls);
static int getter_Wasm_Module_import_section(lua_State* __ls);
static int getter_Wasm_Module_function_section(lua_State* __ls);
static int getter_Wasm_Module_table_section(lua_State* __ls);
static int getter_Wasm_Module_memory_section(lua_State* __ls);
static int getter_Wasm_Module_global_section(lua_State* __ls);
static int getter_Wasm_Module_export_section(lua_State* __ls);
static int getter_Wasm_Module_start_section(lua_State* __ls);
static int getter_Wasm_Module_element_section(lua_State* __ls);
static int getter_Wasm_Module_code_section(lua_State* __ls);
static int getter_Wasm_Module_data_section(lua_State* __ls);
static int getter_Wasm_Module_W_Custom_Sections(lua_State* __ls);
static int getter_Wasm_Module_name(lua_State* __ls);
static int setter_Wasm_Module_type_section(lua_State* __ls);
static int setter_Wasm_Module_import_section(lua_State* __ls);
static int setter_Wasm_Module_function_section(lua_State* __ls);
static int setter_Wasm_Module_table_section(lua_State* __ls);
static int setter_Wasm_Module_memory_section(lua_State* __ls);
static int setter_Wasm_Module_global_section(lua_State* __ls);
static int setter_Wasm_Module_export_section(lua_State* __ls);
static int setter_Wasm_Module_start_section(lua_State* __ls);
static int setter_Wasm_Module_element_section(lua_State* __ls);
static int setter_Wasm_Module_code_section(lua_State* __ls);
static int setter_Wasm_Module_data_section(lua_State* __ls);
static int setter_Wasm_Module_W_Custom_Sections(lua_State* __ls);
static int setter_Wasm_Module_name(lua_State* __ls);
int Wasm_Module_register(lua_State* __ls);

#ifdef __cplusplus
}
#endif //end of extern c
#endif //end of inclusion guard


