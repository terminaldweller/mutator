
/***************************************************Project Mutator****************************************************/
//-*-c++-*-
/*first line intentionally left blank.*/
/*loads the objects into executable memory and registers them with lua.*/
/*Copyright (C) 2017 Farzad Sadeghi

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
#include "./bruiser.h"
#include "lua-5.3.4/src/lua.hpp"
#include <iostream>
#include <functional>
#include <tuple>
#include <vector>
#include <cstdint>
#include <cstdarg>
#include <cstring>
#include <stdarg.h>
#include <sys/mman.h>
#include <unistd.h>
#include <utility>
/**********************************************************************************************************************/
#ifndef EXECUTIONER_H
#define EXECUTIONER_H
/**********************************************************************************************************************/
namespace { // start of anonymous namespace
  constexpr int MEMORY_SIZE = 32768;
  std::vector<uint8_t> memory(MEMORY_SIZE, 0);

  void* alloc_writeable_memory(size_t _size) {
    void* ptr = mmap(0, _size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (ptr == (void*)-1) {
      perror("could not allocate virtual memory.");
      return nullptr;
    }
    return ptr;
  }

  int make_mem_executable(void* _mem, size_t _size) {
    if (mprotect(_mem, _size, PROT_READ | PROT_EXEC) == -1) {
      perror("could not make virtual memory executable.");
      return -1;
    }
    return 0;
  }
} // end of anonymous namespace

int getMemorySize(void) {return MEMORY_SIZE;}

void emitByte(uint8_t _byte, std::vector<uint8_t>& _code) {
  _code.push_back(_byte);
}

void emitBytes(std::vector<uint8_t>& _bytes, std::vector<uint8_t>& _code) {
  for (auto &iter : _bytes) {emitByte(iter, _code);}
}

std::pair<void*, size_t> loadObjsInXMem(std::vector<uint8_t>& _obj_code) {
  size_t code_size = _obj_code.size();
  void* program_memory = alloc_writeable_memory(code_size);
  if (program_memory == nullptr) {
    std::cout << "could not allocate virtual memory\n";
    return std::make_pair(nullptr, 0);
  }
  memcpy(program_memory, _obj_code.data(), code_size);
  if (make_mem_executable(program_memory, code_size) < 0) {
    std::cout << "could not make vmemory executable.\n";
    return std::make_pair(nullptr, 0);
  }
  return std::make_pair(program_memory, code_size);
}

void deallocatedXObj(std::vector<std::tuple<void*, std::string, uint32_t>> _vptrs) {
  std::cout << GREEN << "deallocating xobjs..." << NORMAL << "\n";
  for (auto& iter : _vptrs) {
    int res = munmap(std::get<0>(iter), std::get<2>(iter));
    if (res != 0) PRINT_WITH_COLOR_LB(RED, "could not unmap xobjs.");
  }
}
/**********************************************************************************************************************/
/**********************************************************************************************************************/
class XGlobals {
  public:
    XGlobals() {}

    void reserve(size_t size) {
      globals.push_back(std::make_pair(malloc(size), size));
    }

    void* getAddressByIndex(int index) {
      return nullptr;
    }

  private:
    std::list<std::pair<void*, size_t>> globals;
};
/**********************************************************************************************************************/
/**********************************************************************************************************************/
#endif
/**********************************************************************************************************************/
/*last line intentionally left blank.*/

