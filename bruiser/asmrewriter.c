
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
  if (dummy == NULL) printf("bad user data type.\n");
  return dummy;
}

static JMP_S_T* check_jmpt(lua_State* __ls, int index) {
  JMP_S_T* dummy;
  luaL_checktype(__ls, index, LUA_TUSERDATA);
  dummy = (JMP_S_T*)luaL_checkudata(__ls, index, "jmp_s_t");
  if (dummy == NULL) printf("bad user data type.\n");
  return dummy;
}

JMP_S_T* push_jmpt(lua_State* __ls) {
  lua_checkstack(__ls, 1);
  JMP_S_T* dummy = lua_newuserdata(__ls, sizeof(JMP_S_T));
  luaL_getmetatable(__ls, "jmp_s_t");
  lua_setmetatable(__ls, -2);
  return dummy;
}

int jmpt_push_args(lua_State* __ls, JMP_S_T* jmpt) {
  lua_checkstack(__ls, 12);
  lua_pushinteger(__ls, jmpt->type);
  lua_pushinteger(__ls, jmpt->location);
  lua_pushinteger(__ls, jmpt->size);
  lua_pushlightuserdata(__ls, jmpt->next);
  lua_pushlightuserdata(__ls, jmpt->next_y);
  lua_pushlightuserdata(__ls, jmpt->next_n);
  lua_pushinteger(__ls, jmpt->address);
  lua_pushinteger(__ls, jmpt->address_y);
  lua_pushinteger(__ls, jmpt->address_n);
  lua_pushinteger(__ls, jmpt->y);
  lua_pushinteger(__ls, jmpt->n);
  lua_pushinteger(__ls, jmpt->z);
}

int new_jmpt_2(lua_State* __ls) {
  lua_checkstack(__ls, 12);
  JMP_T jmp_t = luaL_optinteger(__ls, -12, 0);
  uint64_t location = luaL_optinteger(__ls, -11, 0);
  uint8_t size = luaL_optinteger(__ls, -10, 0);
  JMP_S_T* next = lua_touserdata(__ls, -9);
  JMP_S_T* next_y = lua_touserdata(__ls, -8);
  JMP_S_T* next_n = lua_touserdata(__ls, -7);
  uint64_t address = luaL_optinteger(__ls, -6, 0);
  uint64_t address_y = luaL_optinteger(__ls, -5, 0);
  uint64_t address_n = luaL_optinteger(__ls, -4, 0);
  unsigned char y = luaL_optinteger(__ls, -3, 0);
  unsigned char n = luaL_optinteger(__ls, -2, 0);
  unsigned char z = luaL_optinteger(__ls, -1, 0);
  JMP_S_T* dummy = push_jmpt(__ls);
  dummy->type = jmp_t;
  dummy->location = location;
  dummy->size = size;
  dummy->next = next;
  dummy->next_y = next_y;
  dummy->next_n = next_n;
  dummy->address = address;
  dummy->address_y = address_y;
  dummy->address_n = address_n;
  dummy->y = y;
  dummy->n = n;
  dummy->z = z;
  return 1;
}

int new_jmpt(lua_State* __ls) {
  lua_checkstack(__ls, 12);
  JMP_T jmp_t = luaL_optinteger(__ls, 1, 0);
  uint64_t location = luaL_optinteger(__ls, 2, 0);
  uint8_t size = luaL_optinteger(__ls, 3, 0);
  JMP_S_T* next = lua_touserdata(__ls, 4);
  JMP_S_T* next_y = lua_touserdata(__ls, 5);
  JMP_S_T* next_n = lua_touserdata(__ls, 6);
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
  dummy->next = next;
  dummy->next_y = next_y;
  dummy->next_n = next_n;
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
  //printf("this is the jump table custom function.\n");
  //lua_checkstack(__ls, 12);
  lua_pushinteger(__ls, dummy->type);
  lua_pushinteger(__ls, dummy->location);
  lua_pushinteger(__ls, dummy->size);
  lua_pushlightuserdata(__ls, dummy->next);
  lua_pushlightuserdata(__ls, dummy->next_y);
  lua_pushlightuserdata(__ls, dummy->next_n);
  lua_pushinteger(__ls, dummy->address);
  lua_pushinteger(__ls, dummy->address_y);
  lua_pushinteger(__ls, dummy->address_n);
  lua_pushinteger(__ls, dummy->y);
  lua_pushinteger(__ls, dummy->n);
  lua_pushinteger(__ls, dummy->z);
  return 12;
}

#define GET_GENERATOR(X) \
static int X(lua_State* __ls) { \
  JMP_S_T* dummy = check_jmpt(__ls, 1);\
  lua_pop(__ls, -1);\
  lua_pushinteger(__ls, dummy->X);\
  return 1;\
}

#define X_LIST_GEN \
  X(type, "getter method for type")\
  X(location, "getter method for location")\
  X(size, "getter method for size")\
  X(address, "getter method for address")\
  X(address_y, "getter method for address_y")\
  X(address_n, "getter method for address_n")\
  X(y, "getter method for y")\
  X(n, "getter method for n")\
  X(z, "getter method for z")

#define X(X1,X2) GET_GENERATOR(X1)
X_LIST_GEN
#undef X
#undef X_LIST_GEN
#undef SET_GENERATOR

static int next(lua_State* __ls) {
  JMP_S_T* dummy = check_jmpt(__ls, 1);
  lua_pop(__ls, -1);\
  lua_pushlightuserdata(__ls, dummy->next);
  return 1;
}

static int next_y(lua_State* __ls) {
  JMP_S_T* dummy = check_jmpt(__ls, 1);
  lua_pop(__ls, -1);\
  lua_pushlightuserdata(__ls, dummy->next_y);
  return 1;
}

static int next_n(lua_State* __ls) {
  JMP_S_T* dummy = check_jmpt(__ls, 1);
  lua_pop(__ls, -1);\
  lua_pushlightuserdata(__ls, dummy->next_n);
  return 1;
}

static int inext(lua_State* __ls) {
  JMP_S_T* dummy = check_jmpt(__ls, 1);
  if (dummy->next != NULL) {
    jmpt_push_args(__ls, dummy->next);
    new_jmpt_2(__ls);
  } else {
    lua_pushnil(__ls);
  }
  return 1;
}

#define SET_GENERATOR(X) \
  static int jmpt_set_##X(lua_State* __ls) {\
  JMP_S_T* dummy = check_jmpt(__ls,1);\
  dummy->X = luaL_checkinteger(__ls, 2);\
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

int jmpt_set_next(lua_State* __ls) {
  JMP_S_T* dummy = check_jmpt(__ls,1);
  dummy->next = luaL_checkudata(__ls, 2, "jmp_s_t");
  lua_settop(__ls, 1);
  return 1;
}

int jmpt_set_next_y(lua_State* __ls) {
  JMP_S_T* dummy = check_jmpt(__ls,1);
  dummy->next_y = luaL_checkudata(__ls, 2, "jmp_s_t");
  lua_settop(__ls, 1);
  return 1;
}

int jmpt_set_next_n(lua_State* __ls) {
  JMP_S_T* dummy = check_jmpt(__ls,1);
  dummy->next_n = luaL_checkudata(__ls, 2, "jmp_s_t");
  lua_settop(__ls, 1);
  return 1;
}

static int jmpt_gc(lua_State* __ls) {
  JMP_S_T* dummy = check_jmpt(__ls,1);
  //freejmptable(dummy);
}

static int jmpt_tostring(lua_State* __ls) {
  char buff[32];
  sprintf(buff, "%p", convert_jmpt(__ls , 1));
  lua_pushfstring(__ls, "jmp_s_t (%s)", buff);
  return 1;
}

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
  {"custom", jmpt_custom},
  {"type", type},
  {"location", location},
  {"size", size},
  {"next", next},
  {"next_y", next_y},
  {"next_n", next_n},
  {"address", address},
  {"address_y", address_y},
  {"address_n", address_n},
  {"y", y},
  {"n", n},
  {"z", z},
  {"inext", inext},
  {0,0}
};

static const luaL_Reg jmpt_meta[] = {
  {"__gc", jmpt_gc},
  {"__tostring", jmpt_tostring},
  {0, 0}
};

int jmpt_register(lua_State* __ls) {
  luaL_openlib(__ls, "jmp_s_t", jmpt_methods, 0);
  luaL_newmetatable(__ls, "jmp_s_t");
  luaL_openlib(__ls, 0, jmpt_meta, 0);
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
  lua_State* L = luaL_newstate();
  luaL_openlibs(L);

  jmpt_register(L);
  lua_pop(L, 1);

  if (argc > 1) luaL_dofile(L, argv[1]);
  lua_close(L);

  return 0;
}
/**********************************************************************************************************************/
/*last line intentionally left blank.*/

