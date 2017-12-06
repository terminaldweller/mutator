
/***************************************************Project Mutator****************************************************/
//-*-c++-*-
/*first line intentionally left blank.*/
/*loads the objects into executable memory and registers them with lua.*/
/*Copyright (C) 2017 Farzad Sadeghi

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
#include <vector>
#include <cstdint>
#include "lua-5.3.4/src/lua.hpp"
/**********************************************************************************************************************/
namespace {
  constexpr int MEMORY_SIZE = 30000;
  std::vector<uint8_t> memory(MEMORY_SIZE, 0);
}

class Executioner {
  public:
    Executioner() {}
    ~Executioner() {}

    void getObjs(std::vector<std::vector<uint8_t>> _objs) {
      objs = _objs;
    }

    void registerWithLua(lua_State* _lua_State) {}

  private:
    std::vector<std::vector<uint8_t>> objs;
};
/**********************************************************************************************************************/
/*last line intentionally left blank.*/

