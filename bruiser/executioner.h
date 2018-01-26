
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
/**********************************************************************************************************************/
#ifndef EXECUTIONER_H
#define EXECUTIONER_H
/**********************************************************************************************************************/
namespace { // start of anonymous namespace
  using XObject = void(*)(void);
  using xobj_2int = int(*)(int, int);
  using xobj_int = int(*)(int, ...);
  using xobj_float = float(*)(float, ...);
  using xobj_double = double(*)(double, ...);
  using LuaRegFunc = int(*)(lua_State*);

  template<typename T>
  T xobjcaster(void* ptr, T v) {return v;}
  template<typename T, typename... Args>
  T xobjcaster(void* ptr, T first, Args... args) {/*return (first(*)(args...))xobjcaster(ptr);*/}

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

  inline std::vector<uint8_t> codegen(lua_State* __ls) {
    int numargs = lua_gettop(__ls);
    for (int i = 2; i <= numargs; ++i) {
      if (lua_type(__ls, i) == LUA_TBOOLEAN) {
      }
      else if (lua_type(__ls, i) == LUA_TLIGHTUSERDATA) {
      }
      else if (lua_type(__ls, i) == LUA_TNUMBER) {
      }
      else if (lua_type(__ls, i) == LUA_TSTRING) {
#ifdef __x86_64__
        asm volatile("movl %%eax, %0");
#endif
      }
      else if (lua_type(__ls, i) == LUA_TTABLE) {
      }
      else if (lua_type(__ls, i) == LUA_TFUNCTION) {
      }
      else if (lua_type(__ls, i) == LUA_TUSERDATA) {
      }
      else if (lua_type(__ls, i) == LUA_TTHREAD) {
      }
      else { // type is nil
        PRINT_WITH_COLOR_LB(RED, "you passed a Nil argument...");
      }
    }
  }

  std::vector<uint8_t> arg_emitter(std::vector<uint8_t> _args) {
    std::vector<uint8_t> ret;
    return ret;
  }

  int LuaXobjWrapper(lua_State* __ls) {
    int numargs = lua_gettop(__ls);
    std::vector<uint8_t> arg_vec;
    std::string xfuncname;
    std::vector<std::pair<intptr_t, int>> arg_ptr;
    std::vector<std::pair<std::string, int>> arg_str;
    std::vector<std::pair<double, int>> arg_double;
    std::vector<std::pair<bool, int>> arg_bool;

    if (lua_type(__ls, 1) == LUA_TSTRING) {
      xfuncname = lua_tostring(__ls, 1);
    } else {
      PRINT_WITH_COLOR_LB(RED, "the first argument should be a string that is the name of the xfunc to be called.");
    }
    
    // detecting arg types
    for (int i = 2; i <= numargs; ++i) {
      if (lua_type(__ls, i) == LUA_TBOOLEAN) {
        arg_bool.push_back(std::make_pair(!!lua_tonumber(__ls, i), i));
      }
      else if (lua_type(__ls, i) == LUA_TLIGHTUSERDATA) {
      }
      else if (lua_type(__ls, i) == LUA_TNUMBER) {
        arg_double.push_back(std::make_pair(lua_tonumber(__ls, i), i));
      }
      else if (lua_type(__ls, i) == LUA_TSTRING) {
        arg_str.push_back(std::make_pair(lua_tostring(__ls, i), i));
      }
      else if (lua_type(__ls, i) == LUA_TTABLE) {
      }
      else if (lua_type(__ls, i) == LUA_TFUNCTION) {
      }
      else if (lua_type(__ls, i) == LUA_TUSERDATA) {
      }
      else if (lua_type(__ls, i) == LUA_TTHREAD) {
      }
      // type is Nil
      else {
        PRINT_WITH_COLOR_LB(RED, "you passed a Nil argument...");
      }
    }

    pid_t pid = fork();
    if (pid < 0) {
      PRINT_WITH_COLOR_LB(RED, "could not fork...");
      lua_pushnumber(__ls, EXIT_FAILURE);
    }
    if (pid == 0) {}
    if (pid > 0) {
    }

    return 0;
  }
} // end of anonymous namespace

int getMemorySize(void) {return MEMORY_SIZE;}

class Executioner {
  public:
    Executioner() {}
    ~Executioner() {
      for (auto &iter : obj_mem_ptrs) {
        if (iter.first != nullptr) {
          if (munmap(iter.first, iter.second) < 0) {
            perror("could not unmap vmemory.");
          }
        }
      }
    }

    void getObjs(std::vector<std::vector<uint8_t>>& _objs) {objs = _objs;}

    void getNames(std::vector<std::string>& _names) {names = _names;}

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
      xvoidptrs.push_back(program_memory);
      return std::make_pair(program_memory, code_size);
    }

    std::vector<uint64_t> getAllArgs(lua_State* __ls) {
      int numargs = lua_gettop(__ls);
      std::vector<uint64_t> args;
      for (int i = 0; i < numargs; ++i) {}
      return args;
    }

    void loadAll(void) {
      for (auto &iter : objs) {
        this->loadObjsInXMem(iter);
      }
    }

    void emitByte(uint8_t _byte, std::vector<uint8_t>& _code) {
      _code.push_back(_byte);
    }

    void emitBytes(std::vector<uint8_t>& _bytes, std::vector<uint8_t>& _code) {
      for (auto &iter : _bytes) {this->emitByte(iter, _code);}
    }

    void registerWithLua(lua_State* _lua_State) {
      lua_register(_lua_State, "xobjwrapper", LuaXobjWrapper);
    }

    XObject getXobject(void* _ptr) {return (XObject)_ptr;}

    void xobjsGetPtrs(void) {
      for (auto& iter : obj_mem_ptrs) {
        XObject dummy = (XObject)iter.first;
        xobjs.push_back(dummy);
      }
    }

    void pusheph(std::function<int(lua_State*)> __eph) {ephs.push_back(__eph);}
    void pushvptr(void* _vptr, std::string _name) {vptrs.push_back(std::make_pair(_vptr, _name));}
    std::vector<std::pair<void*, std::string>> getvptrs(void) {return vptrs;}
    std::pair<void*, std::string> getvptrbyindex(unsigned int _index) {
      if (vptrs.size() - 1 >= _index) {
        return vptrs[_index];
      }
      return std::make_pair(nullptr, "");
    }
    std::pair<void*, std::string> getvptrbyname(const char* name) {
      for (auto &iter : vptrs) {
        if (std::strcmp(name, iter.second.c_str()) == 0) return iter;
      }
      return std::make_pair(nullptr, "");
    }

  private:
    std::vector<std::pair<void*, size_t>> obj_mem_ptrs;
    std::vector<std::vector<uint8_t>> objs;
    std::vector<std::string> names;
    std::vector<XObject> xobjs;
    std::vector<void*> xvoidptrs;
    std::vector<std::function<int(lua_State*)>> ephs;
    std::vector<std::pair<void*, std::string>> vptrs;
};
/**********************************************************************************************************************/
/**********************************************************************************************************************/
#if 1
class EphemeralFunc {
  public:
    EphemeralFunc(xobj_2int _ptr, std::string _name) : ptr(_ptr), name(_name) {}
    virtual ~EphemeralFunc() {}

    int lua_func(lua_State* __ls) {
      int numargs = lua_gettop(__ls);
      if (numargs != 2) {
        PRINT_WITH_COLOR(RED, "expected 2 arguments...");
        lua_tonumber(__ls, 0);
        return 1;
      }
      int arg1 = lua_tonumber(__ls, 1);
      int arg2 = lua_tonumber(__ls, 1);
      std::cout << RED << "right before execution..." << NORMAL << "\n";
      int result = ptr(arg1, arg2);
      lua_pushnumber(__ls, result);
      return 1;
    }

  private:
    xobj_2int ptr;
    std::string name;
};

typedef int (EphemeralFunc::*m_func)(lua_State* L);
template<m_func func>
int LuaDispatch2(lua_State* __ls)
{
  EphemeralFunc* LWPtr = *static_cast<EphemeralFunc**>(lua_getextraspace(__ls));
  return ((*LWPtr).*func)(__ls);
}

int devi_luareg(lua_State* __ls, xobj_2int __xobj, std::string __name, Executioner& __EX) {
  EphemeralFunc eph(__xobj, __name);
  //__EX.pusheph(eph.lua_func);
  lua_register(__ls, __name.c_str(), &LuaDispatch2<&EphemeralFunc::lua_func>);
  return 0;
}
#endif
/**********************************************************************************************************************/
/**********************************************************************************************************************/
class Arguary {
  public:
    Arguary() = default;
    ~Arguary() {}

    void pass_ptr(void* _arg) {ptr_stack.push_back(_arg);}
    void pass_int(int _arg) {int_stack.push_back(_arg);}
    void pass_uint64(uint64_t _arg) {uint64_stack.push_back(_arg);}
    void pass_string(char* _arg) {string_stack.push_back(_arg);}
    void pass_float(float _arg) {float_stack.push_back(_arg);}
    void pass_double(double _arg) {double_stack.push_back(_arg);}
    void pass_llint(long long int _arg) {llint_stack.push_back(_arg);}
    void clear_arg_stacks(void) {
      ptr_stack.clear();
      int_stack.clear();
      uint64_stack.clear();
      string_stack.clear();
      float_stack.clear();
      double_stack.clear();
      llint_stack.clear();
    }

  private:
    std::vector<void*> ptr_stack;
    std::vector<int> int_stack;
    std::vector<uint64_t> uint64_stack;
    std::vector<char*> string_stack;
    std::vector<float> float_stack;
    std::vector<double> double_stack;
    std::vector<long long int> llint_stack;
};
/**********************************************************************************************************************/
/**********************************************************************************************************************/
class XGlobals {
  public:
    XGlobals() {}
    ~XGlobals() {}
  private:
    std::list<std::pair<void*, size_t>> globals;
};
/**********************************************************************************************************************/
/**********************************************************************************************************************/
#endif
/**********************************************************************************************************************/
/*last line intentionally left blank.*/

