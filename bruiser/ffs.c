
/***************************************************Project Mutator****************************************************/
/*first line intentionally left blank.*/
/*when c++ is too annoying to deal with...*/
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
#include "ffs.h"
// @DEVI-the acceptable indexes right now are 0 and 1 since we are only reserving 2 void* slots in luaconf.h.
void* lua_getextraspace_wrapper(lua_State* __ls, int index) {
  return lua_getextraspace(__ls) + sizeof(void*)*index;
}
/**********************************************************************************************************************/
/*last line intentionally left blank.*/

