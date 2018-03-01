

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
#include "./lua-5.3.4/src/lua.hpp"
#include "./bruisercapstone.h"

#include <inttypes.h>
/**********************************************************************************************************************/
#ifndef ASM_REWRITER_H
#define ASM_REWRITER_H

static JMP_S_T* convert_jmpt(lua_State* __ls, int index);
static JMP_S_T* check_jmpt(lua_State* __ls, int index);
static JMP_S_T* push_jmpt(lua_State* __ls);
static int new_jmpt(lua_State* __ls);
static int jmpt_custom(lua_State* __ls);

#define SET_GENERATOR(X) \
  static int jmpt_set_##X(lua_State* __ls);

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

static const luaL_reg jmpt_methods[] = {
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

static int jmpt_gc(lua_State* __ls) {}

static const luaL_reg jmpt_meta[] = {
  {"__gc", jmpt_gc},
  {0, 0}
}

int jmpt_register(lua_State* __ls) {

#endif
/**********************************************************************************************************************/
/*last line intentionally left blank.*/

