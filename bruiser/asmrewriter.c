

/***************************************************Project Mutator****************************************************/
/*first line intentionally left blank.*/
/*bruiser's lua asmrewriter implementation for jump tables*/
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
#include "./lua-5.3.4/src/lua.h"
#include "./lua-5.3.4/src/lauxlib.h"
#include "./lua-5.3.4/src/lualib.h"
#include "./bruisercapstone.h"
#include "./asmrewriter.h"

#include <inttypes.h>
/**********************************************************************************************************************/
static JMP_S_T* convert_jmpt(lua_State* __ls, int index) {
  JMP_S_T* dummy = (JMP_S_T*)lua_touserdata(__ls, index);
  //if (dummy == NULL) luaL_typerror(__ls, index, dummy);
  return dummy;
}

static JMP_S_T* check_jmpt(lua_State* __ls, int index) {
  JMP_S_T* dummy;
  luaL_checktype(__ls, index, LUA_TUSERDATA);
  dummy = (JMP_S_T*)luaL_checkudata(__ls, index, "jmp_s_t");
  //if (dummy == NULL) luaL_typerror(__ls, index, dummy);
  return dummy;
}

static JMP_S_T* push_jmpt(lua_State* __ls) {
  JMP_S_T* dummy = (JMP_S_T*)lua_newuserdata(__ls, sizeof(JMP_S_T));
  luaL_getmetatable(__ls, "jmp_s_t");
  lua_setmetatable(__ls, -2);
  return dummy;
}

static int new_jmpt(lua_State* __ls) {
  JMP_T jmp_t = luaL_optinteger(__ls, 1, 0);
  uint64_t location = luaL_optinteger(__ls, 2, 0);
  uint8_t size = luaL_optinteger(__ls, 3, 0);
  //
  //
  //
  uint64_t address = luaL_optinteger(__ls, 7, 0);
  uint64_t address_y = luaL_optinteger(__ls, 8, 0);
  uint64_t address_n = luaL_optinteger(__ls, 9, 0);
  unsigned char y = luaL_optinteger(__ls, 10, 0);
  unsigned char n = luaL_optinteger(__ls, 11, 0);
  unsigned char z = luaL_optinteger(__ls, 12, 0);
  JMP_S_T* dummy = push_jmpt(__ls);
  dummy->type = jmp_t;
  dummy->location = location;
  dummy->size = size;
  //dummy->next =;
  //dummy->next_y =;
  //dummy->next_n =;
  dummy->address = address;
  dummy->address_y = address_y;
  dummy->address_n = address_n;
  dummy->y = y;
  dummy->n = n;
  dummy->z = z;
  return 1;
}

static int jmpt_custom(lua_State* __ls) {
  JMP_S_T* dummy = check_jmpt(__ls, 1);
  printf("this is the jump table custom function.\n");
  lua_pushnumber(__ls, dummy->type);
  lua_pushnumber(__ls, dummy->location);
  lua_pushnumber(__ls, dummy->size);
  lua_pushlightuserdata(__ls, dummy->next);
  lua_pushlightuserdata(__ls, dummy->next_y);
  lua_pushlightuserdata(__ls, dummy->next_n);
  lua_pushnumber(__ls, dummy->address);
  lua_pushnumber(__ls, dummy->address_y);
  lua_pushnumber(__ls, dummy->address_n);
  lua_pushnumber(__ls, dummy->y);
  lua_pushnumber(__ls, dummy->n);
  lua_pushnumber(__ls, dummy->z);
  return 12;
}

#define SET_GENERATOR(X) \
  static int jmpt_set_##X(lua_State* __ls) {\
  JMP_S_T* dummy = check_jmpt(__ls,1);\
  dummy->type = luaL_checkinteger(__ls, 2);\
  lua_settop(__ls, 1);\
  return 1;\
}

#define X_LIST_GEN \
  X(type, "setter method for type")\
  X(location, "setter method for location")\
  X(size, "setter method for size")\
  X(address, "setter method for address")\
  X(address_y, "setter method for address_y")\
  X(address_n, "setter method for address_n")\
  X(y, "setter method for y")\
  X(n, "setter method for n")\
  X(z, "setter method for z")

#define X(X1,X2) SET_GENERATOR(X1)
X_LIST_GEN
#undef X
#undef X_LIST_GEN
#undef SET_GENERATOR

static int jmpt_set_next(lua_State* __ls) {}
static int jmpt_set_next_y(lua_State* __ls) {}
static int jmpt_set_next_n(lua_State* __ls) {}

static int jmpt_gc(lua_State* __ls) {}

static const luaL_Reg jmpt_methods[] = {
  {"new", new_jmpt},
  {"set_type", jmpt_set_type},
  {"set_location", jmpt_set_location},
  {"set_size", jmpt_set_size},
  {"set_address", jmpt_set_address},
  {"set_address_y", jmpt_set_address_y},
  {"set_address_n", jmpt_set_address_n},
  {"set_next", jmpt_set_next},
  {"set_next_y", jmpt_set_next_y},
  {"set_next_n", jmpt_set_next_n},
  {"set_y", jmpt_set_y},
  {"set_n", jmpt_set_n},
  {"set_z", jmpt_set_z},
  {0,0}
};

static const luaL_Reg jmpt_meta[] = {
  {"__gc", jmpt_gc},
  {0, 0}
};

int jmpt_register(lua_State* __ls) {
  luaL_newlib(__ls, jmpt_methods);
  luaL_newmetatable(__ls, "jmp_s_t");
  luaL_newlib(__ls, jmpt_meta);
  lua_pushliteral(__ls, "__index");
  lua_pushvalue(__ls, -3);
  lua_rawset(__ls, -3);
  lua_pushliteral(__ls, "__metatable");
  lua_pushvalue(__ls, -3);
  lua_rawset(__ls, -3);
  lua_pop(__ls, 1);
  return 1;
}
//@DEVI-after jmpt_register, the methods are still on the stack. remove them by lua_pop(__ls, 1)
/**********************************************************************************************************************/
//@DEVI-the main is only meant for testing
#pragma weak main
int main(int argc, char** argv) {
  return 0;
}
/**********************************************************************************************************************/
/*last line intentionally left blank.*/

