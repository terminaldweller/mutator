
/***************************************************Project Mutator****************************************************/
//-*-c++-*-
/*first line intentionally left blank.*/
/*the main for bruiser.*/
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
/*included modules*/
/*project headers*/
#include "bruiser.h"
#include "CompletionHints.h"
#include "../mutator_aux.h"
#include "mutagen.h"
#include "ORCmutation.h"
#include "executioner.h"
#include "bruiserffi.h"
#include "bruisercapstone.h"
#include "asmrewriter.h"
/*standard headers*/
#include <exception>
#include <fstream>
#include <string>
#include <cassert>
#include <iostream>
#include <regex>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <thread>
/*LLVM headers*/
#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Lex/Lexer.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Function.h"
/*other*/
#include "linenoise/linenoise.h"
#include "lua-5.3.4/src/lua.hpp"
#include <Python.h>

#include "luadummy.h"
/**********************************************************************************************************************/
/*used namespaces*/
using namespace llvm;
using namespace clang;
using namespace clang::ast_matchers;
using namespace clang::driver;
using namespace clang::tooling;
/**********************************************************************************************************************/
#define __DBG_1
#if 1
#undef __DBG_1
#endif
/**********************************************************************************************************************/
/*global vars*/
namespace { // start of anonymous namespace
  static llvm::cl::OptionCategory BruiserCategory("Empty");
  std::vector<std::string> PushToLua;

  bruiser::M0_ERR m0_err [[maybe_unused]];
  bruiser::BruiserReport BruiseRep;

  struct ShellGlobal {
    ShellGlobal() = default;

    std::vector<std::string> PATH;
    std::vector<std::string> SOURCE_FILES;
    std::string MAKEPATH;
    std::string BINPATH;
    unsigned int HISTORY_SIZE = SHELL_HISTORY_SIZE;
    bool droptocli;
  };

  struct ShellCache {
    std::string LastFileUsed;
    std::string LastFileUsedShort;
  };

  ShellGlobal ShellGlobalInstance;
  ShellCache ShellCacheInstance;
} // end of anonymous naemspace
/**********************************************************************************************************************/
cl::opt<bool> Intrusive("intrusive", cl::desc("If set true. bruiser will mutate the source."), cl::init(true), cl::cat(BruiserCategory), cl::ZeroOrMore);
cl::opt<bool> CheckSystemHeader("SysHeader", cl::desc("bruiser will run through System Headers"), cl::init(false), cl::cat(BruiserCategory), cl::ZeroOrMore);
cl::opt<bool> MainFileOnly("MainOnly", cl::desc("bruiser will only report the results that reside in the main file"), cl::init(false), cl::cat(BruiserCategory), cl::ZeroOrMore);
cl::opt<std::string> M0XMLPath("xmlpath", cl::desc("tells bruiser where to find the XML file containing the Mutator-LVL0 report."), cl::init(bruiser::M0REP), cl::cat(BruiserCategory), cl::ZeroOrMore);
cl::opt<bool> LuaJIT("jit", cl::desc("should bruiser use luajit or not."), cl::init(true), cl::cat(BruiserCategory), cl::ZeroOrMore);
cl::opt<bool> Verbose("verbose", cl::desc("verbosity"), cl::init(false), cl::cat(BruiserCategory), cl::ZeroOrMore);
cl::opt<bool> Nosrc("No Source file needed", cl::desc("verbosity"), cl::init(true), cl::cat(BruiserCategory), cl::ZeroOrMore);
// @DEVI-FIXME-we need something like python's code module. lua's -i is not it.
cl::opt<bool> LuaInteractive("interactive", cl::desc("run in interactive mode"), cl::init(false), cl::cat(BruiserCategory), cl::ZeroOrMore);
cl::opt<std::string> NonCLILuaScript("lua", cl::desc("specifies a lua script for bruiser to run in non-interactive mode"), cl::init(""), cl::cat(BruiserCategory), cl::Optional);
cl::opt<std::string> LuaDefault("luadefault", cl::desc("the path to the luadefault file. the default option is where the bruiser executable is."), cl::init("./defaults.lua"), cl::cat(BruiserCategory), cl::ZeroOrMore);
/**********************************************************************************************************************/
template <typename T>
int pushLuaTableInt(lua_State* __ls, std::vector<T> vec) {
  int tableindex = 1;
  lua_newtable(__ls);
  if (!lua_checkstack(__ls, vec.size())) {
    PRINT_WITH_COLOR_LB(RED, "cant grow lua stack. current size is too small.");
    return -1;
  }
  for (auto& iter : vec) {
    lua_pushinteger(__ls, tableindex);
    tableindex++;
    lua_pushinteger(__ls, iter);
    lua_settable(__ls, -3);
  }
  return 0;
}

int pushLuaTableString(lua_State* __ls, std::vector<std::string> vec) {
  int tableindex = 1;
  lua_newtable(__ls);
  if (!lua_checkstack(__ls, vec.size())) {
    PRINT_WITH_COLOR_LB(RED, "cant grow lua stack. current size is too small.");
    return -1;
  }
  for (auto& iter : vec) {
    lua_pushinteger(__ls, tableindex);
    tableindex++;
    lua_pushstring(__ls, iter.c_str());
    lua_settable(__ls, -3);
  }
  return 0;
}

template <typename T>
int pushLuaTableNumber(lua_State* __ls, std::vector<T> vec) {
  int tableindex = 1;
  lua_newtable(__ls);
  if (!lua_checkstack(__ls, vec.size())) {
    PRINT_WITH_COLOR_LB(RED, "cant grow lua stack. current size is too small.");
    return -1;
  }
  for (auto& iter : vec) {
    lua_pushinteger(__ls, tableindex);
    tableindex++;
    lua_pushnumber(__ls, iter);
    lua_settable(__ls, -3);
  }
  return 0;
}

template <typename T>
std::vector<T> getLuaTableInt(lua_State* __ls, int numargs, int argnum) {
  std::vector<T> ret;
  int table_length = lua_rawlen(__ls, argnum);
  if (!lua_checkstack(__ls, table_length)) {
    std::cout << RED << "need to grow lua stack by " << table_length << ":";
    PRINT_WITH_COLOR_LB(RED, "cant grow lua stack by that much.");
  }
  for (int i = 1; i <= table_length; ++i) {
    lua_rawgeti(__ls, argnum, i);
    ret.push_back(lua_tointeger(__ls, 1 + numargs));
    lua_pop(__ls, 1);
  }
  return ret;
}

std::vector<std::string> getLuaTableString(lua_State* __ls, int numargs, int argnum) {
  std::vector<std::string> ret;
  int table_length = lua_rawlen(__ls, argnum);
  if (!lua_checkstack(__ls, table_length)) {
    std::cout << RED << "need to grow lua stack by " << table_length << ":";
    PRINT_WITH_COLOR_LB(RED, "cant grow lua stack by that much.");
  }
  for (int i = 1; i <= table_length; ++i) {
    lua_rawgeti(__ls, argnum, i);
    ret.push_back(lua_tostring(__ls, 1 + numargs));
    lua_pop(__ls, 1);
  }
  return ret;
}

template <typename T>
std::vector<T> getLuaTableNumber(lua_State* __ls, int numargs, int argnum) {
  std::vector<T> ret;
  int table_length = lua_rawlen(__ls, argnum);
  if (!lua_checkstack(__ls, table_length)) {
    std::cout << RED << "need to grow lua stack by " << table_length << ":";
    PRINT_WITH_COLOR_LB(RED, "cant grow lua stack by that much.");
  }
  for (int i = 1; i <= table_length; ++i) {
    lua_rawgeti(__ls, argnum, i);
    ret.push_back(lua_tonumber(__ls, 1 + numargs));
    lua_pop(__ls, 1);
  }
  return ret;
}
/**********************************************************************************************************************/
class LuaEngine
{
  public:
    LuaEngine() {
      LS = luaL_newstate();
    }

    /*@DEVI-this will just create member functions that open a single lua libarary.
     * For example to load the string library just call LuaEngine::LoadstringLib().*/
#define OPEN_LUA_LIBS(__x1) \
      void Load##__x1##Lib(void){\
      luaL_requiref(LS, #__x1, luaopen_##__x1, 1);}

    OPEN_LUA_LIBS(base)
    OPEN_LUA_LIBS(table)
    OPEN_LUA_LIBS(io)
    OPEN_LUA_LIBS(string)
    OPEN_LUA_LIBS(math)
    OPEN_LUA_LIBS(os)

#undef OPEN_LUA_LIBS

    void LoadAuxLibs(void) {
      luaL_requiref(LS, "table", luaopen_table, 1);
      luaL_requiref(LS, "io", luaopen_io, 1);
      luaL_requiref(LS, "string", luaopen_string, 1);
    }

    void LoadEverylib(void) {
      luaL_openlibs(LS);
    }

    void registerJMPTable(void) {
      jmpt_register(LS);
      lua_pop(LS, 1);
    }

    void RunLuaDefaults(void) {
      luaL_dofile(LS, LuaDefault.c_str());
    }

    void RunString(char* __lua_string) {}

    void RunChunk(char* __lua_chunk) {
      dostring(LS, __lua_chunk, "test");
    }

    int RunScript(char* __lua_script) {
      return luaL_dofile(LS, __lua_script);
    }

    void Test(void) {
      luaL_dofile(LS, "./lua-scripts/test.lua");
      luaL_dofile(LS, "./lua-scripts/test1.lua");
      luaL_dofile(LS, "./lua-scripts/test2.lua");
    }

    void Test2(void) {
      luaL_dofile(LS, "./lua-scripts/test1.lua");
    }

    void Test3(void) {
      luaL_dofile(LS, "./lua-scripts/test2.lua");
    }

    void Test4(void) {
      luaL_dofile(LS, "./lua-scripts/test3.lua");
    }

    lua_State* GetLuaState(void) {
      return this->LS;
    }

    void Cleanup(void) {
      lua_close(LS);
    }

    void HandleLuaErrorGeneric(lua_State* LS, const char* fmt, ...) {
      va_list argp;
      va_start(argp, fmt);
      std::vfprintf(stderr, fmt, argp);
      va_end(argp);
      lua_close(LS);
      exit(EXIT_FAILURE);
    }

  private:
    lua_State* LS;
};
/**********************************************************************************************************************/
class PyExec {
  public:
    PyExec(std::string __py_script_name, std::string __py_func_name, std::string __obj_path ) : 
      py_script_name(__py_script_name), py_func_name(__py_func_name), obj_path(__obj_path) {}

    int run(void) {
      //std::wstring py_sys_path = L"../bfd";
      Py_Initialize();
      int argc = 2;
      //std::wstring argv[2];
      wchar_t* argv[2];
      argv[0] = Py_DecodeLocale((char*)py_script_name.c_str(), 0);
      argv[1] = Py_DecodeLocale((char*)obj_path.c_str(), 0);
      PySys_SetArgv(argc, argv);
      pName = PyUnicode_DecodeFSDefault(py_script_name.c_str());
      PyRun_SimpleString("import sys\nsys.path.append(\"../bfd\")\n");
      pModule = PyImport_Import(pName);
      Py_DECREF(pName);

      if (pModule != nullptr) {
        pFunc = PyObject_GetAttrString(pModule, py_func_name.c_str());
        if (pFunc && PyCallable_Check(pFunc)) {
          if (Verbose) std::cout << GREEN << "function is callable." << NORMAL << "\n";
          pArgs = PyTuple_New(1);
          pValue = PyUnicode_FromString(obj_path.c_str());
          PyTuple_SetItem(pArgs, 0, pValue);
          pArgs = nullptr;
          if (Verbose) std::cout << BLUE << "calling python function..." << NORMAL << "\n";
          pValue = PyObject_CallObject(pFunc, pArgs);
          //Py_DECREF(pArgs);
          if (pValue != nullptr) {
            if (Verbose) std::cout << GREEN << "call finished successfully." << NORMAL << "\n";
            //printf("Result of call: %ld\n", PyLong_AsLong(pValue));
            //Py_DECREF(pValue);
          } else {
            Py_DECREF(pFunc);
            Py_DECREF(pModule);
            PyErr_Print();
            std::cout << RED << "Call failed." << NORMAL << "\n";
            return EXIT_FAILURE;
          }
        }
      else {
        if (PyErr_Occurred()) PyErr_Print();
        fprintf(stderr, "Cannot find function\"%s\"\n", py_func_name.c_str());
      }
      Py_XDECREF(pFunc);
      Py_DECREF(pModule);
    }
    else {
      PyErr_Print();
      fprintf(stderr, "Failed to load \"%ls\"\n", argv[0]);
      return 1;
    }
    Py_Finalize();
    return 0;
    }

    std::vector<std::string> actionParser(std::string action) {}
    void convertNPush(PyObject* pyobject) {}

    int64_t pyInt(PyObject* po) {return PyLong_AsLong(po);}

    double pyFloat(PyObject* po) {return PyFloat_AsDouble(po);}

    std::vector<PyObject*> pyList_unpack(PyObject* po) {
      std::vector<PyObject*> dummy;
      if (PyList_Check(po)) {
        int size = PyList_Size(po);
        for (int i = 0; i < size; ++i) {
          dummy.push_back(PyList_GetItem(po, i));
        }
      } else {
        PRINT_WITH_COLOR_LB(RED, "Not a PyList object.");
      }
      return dummy;
    }

    std::string pyString(PyObject* po) {
      return PyBytes_AsString(PyUnicode_AsEncodedString(PyObject_Repr(po), "utf-8", "surrogateescape"));
    }

    std::pair<std::vector<PyObject*>, std::vector<PyObject*>> pyDict_unpack(PyObject* po) {
      std::vector<PyObject*> Keys, Values;
      if (PyDict_Check(po)) {
        Keys = pyList_unpack(PyDict_Keys(po));
        Values = pyList_unpack(PyDict_Values(po));
      } else {
        PRINT_WITH_COLOR_LB(RED, "Not a PyDict object.");
      }
      return std::make_pair(Keys, Values);
    }

    char* pyBytes(PyObject* po) {
      char* dummy;
      if (PyBytes_Check(po)) {
        dummy = PyBytes_AsString(po);
      } else {
        PRINT_WITH_COLOR_LB(RED, "Not a PyBytes object.");
      }
      return dummy;
    }

    char* pyByteArray(PyObject* po) {
      char* dummy;
      if (PyByteArray_Check(po)) {
        dummy = PyByteArray_AsString(po);
      } else {
        PRINT_WITH_COLOR_LB(RED, "Not a PyByteArray object.");
      }
    }

    int getAsCppStringVec(void) {
      if (Verbose) PRINT_WITH_COLOR_LB(BLUE, "processing return result...");
      if (PyList_Check(pValue)) {
        if (Verbose) std::cout << GREEN << "got a python list\n" << NORMAL;
        int list_length = PyList_Size(pValue);
        if (Verbose) std::cout << BLUE << "length of list: " << list_length << NORMAL  <<"\n";
        for (int i = 0; i < list_length; ++i) {
          PyObject* pybytes = PyList_GetItem(pValue, i);
          //@DEVI-FIXIT-the weird '' we get for strings is because of repr().
          PyObject* pyrepr = PyObject_Repr(pybytes);
          PyObject* pyunicode = PyUnicode_AsEncodedString(pyrepr, "utf-8", "surrogateescape");
          const char* dummy = PyBytes_AsString(pyunicode);
          //std::cout << RED << dummy << "\n" << NORMAL;
          hexobj_str.push_back(std::string(dummy));
        }
      }
      return 0;
    }

    int getAsCppByte_PyIntList(void) {
      if(PyList_Check(pValue)) {
        int list_length = PyList_Size(pValue);
        for(int i = 0; i < list_length; ++i) {
          PyObject* pybytes = PyList_GetItem(pValue, i);
          if (PyLong_Check(pybytes)) {
            text_section.push_back(PyLong_AsLong(pybytes));
          }
        }
      }
      return 0;
    }

    int getAsCppByte(void) {
      if (Verbose) PRINT_WITH_COLOR_LB(BLUE, "processing return result...");
      std::vector<uint8_t> tempvec;
      if(PyList_Check(pValue)) {
        int list_length = PyList_Size(pValue);
        if (Verbose) std::cout << BLUE << "length of list: " << list_length << NORMAL << "\n";
        for(int i = 0; i < list_length; ++i) {
          PyObject* pybytes = PyList_GetItem(pValue, i);
          if(PyList_Check(pybytes)) {
            int list_length_2 = PyList_Size(pybytes);
            for(int j = 0; j < list_length_2; ++j) {
              PyObject* dummy_int = PyList_GetItem(pybytes, j);
              if (PyLong_Check(dummy_int)) {
                unsigned char byte = PyLong_AsLong(dummy_int);
                tempvec.push_back(int(byte));
              }
            }
            //if (!tempvec.empty()) {hexobj.push_back(tempvec);}
            hexobj.push_back(tempvec);
            tempvec.clear();
          }
        }
      }
      return 0;
    }

    void killPyObj(void) {
      Py_DECREF(pValue);
    }

    void printHexObjs(void) {
        PRINT_WITH_COLOR_LB(YELLOW, "functions with a zero size will not be printed:");
        for (auto &iter : hexobj) {
          for (auto &iterer : iter) {
            std::cout << RED << std::hex << int(iterer) << " ";
          }
          std::cout << "\n" << NORMAL;
        }
    }

    std::vector<std::vector<uint8_t>> exportObjs(void) {return hexobj;}
    std::vector<std::string> exportStrings(void) {return hexobj_str;}
    std::vector<std::uint8_t> exportTextSection(void) {return text_section;}

    void getVarargs(std::vector<void*> _varargs) {varargs = _varargs;}

  private:
    std::string py_script_name;
    std::string py_func_name;
    std::string obj_path;
    PyObject* pName;
    PyObject* pModule;
    PyObject* pDict;
    PyObject* pFunc;
    PyObject* pArgs;
    PyObject* pValue;
    int argc;
    char** argv;
    std::vector<std::string> hexobj_str;
    std::vector<std::vector<uint8_t>> hexobj;
    std::vector<uint8_t> text_section;
    std::vector<void*> varargs;
};
/**********************************************************************************************************************/
class XObjReliquary {};
/**********************************************************************************************************************/
class CompilationDatabaseProcessor {
  public:
  CompilationDatabaseProcessor(CompilationDatabase &__cdb) : CDB(__cdb) {}

  void CalcMakePath(void) {
    std::vector<std::string> Paths;
    std::vector<CompileCommand> CCV = CDB.getAllCompileCommands();

    for(auto &iter : CCV) {
      SourceFiles.push_back(iter.Filename);
      //PRINT_WITH_COLOR_LB(RED, SourceFiles.back().c_str());
    }

    MakePath = CCV[0].Directory;
    //PRINT_WITH_COLOR_LB(RED, MakePath.c_str());
  }

  bool CompilationDatabseIsEmpty(void) {
    std::vector<CompileCommand> CCV = CDB.getAllCompileCommands();
    if(CCV.empty()) {return true;}
    return false;
  }

  std::string GetMakePath(void) {return this->MakePath;}

  std::vector<std::string> GetSourceFiles(void) {return this->SourceFiles;}

  void PopulateGPATH(void) {
    ShellGlobalInstance.PATH.push_back(MakePath);
  }

  void PopulateGSOURCEFILES(void) {
    for (auto &iter : SourceFiles) {
      ShellGlobalInstance.SOURCE_FILES.push_back(iter);
    }
  }

  private:
  CompilationDatabase &CDB;
  std::string MakePath;
  std::vector<std::string> SourceFiles;
};
/**********************************************************************************************************************/
/*the implementation of the bruiser logger.*/
bruiser::BruiserReport::BruiserReport ()
{
  BruiserLog.open("bruiser.log");
}

bruiser::BruiserReport::~BruiserReport()
{
  BruiserLog.close();
}

/**
 * @brief Will print the argument in the log file. Expects to receive valid types usable for a stream.
 *
 * @param __arg
 *
 * @return Returns true if the write was successful, false otherwise.
 */
template <typename T>
bool bruiser::BruiserReport::PrintToLog(T __arg)
{
  BruiserLog << __arg << "\n";
  return !BruiserLog.bad();
}
/**********************************************************************************************************************/
bruiser::TypeInfo::TypeInfo(const clang::ast_type_traits::DynTypedNode* __dtn) : DTN(__dtn) {};

bruiser::TypeInfo::~TypeInfo() {};

const clang::Type* bruiser::TypeInfo::getTypeInfo(clang::ASTContext* __astc)
{
  const clang::Expr* EXP = DTN->get<clang::Expr>();

  const clang::Type* TP = EXP->getType().getTypePtr();

  return __astc->getCanonicalType(TP);
}
/**********************************************************************************************************************/
namespace bruiser
{
  void BruiserLinenoiseCompletionCallback(const char* __buf, linenoiseCompletions* __lc)
  {
    if (__buf[0] == 'h')
    {
      linenoiseAddCompletion(__lc, "help");
    }
  }

  char* BruiserLinenoiseHintsCallback(const char* __buf, int* __color, int* __bold)
  {
    if (!strcasecmp(__buf, "h"))
    {
      *__color = LN_MAGENTA;
      *__bold = NO_BOLD;
      return (char *)"elp";
    }

    return NULL;
  }
}
/**********************************************************************************************************************/
class AbstractMatcherHandler : public virtual MatchFinder::MatchCallback
{
  public:
    AbstractMatcherHandler (Rewriter &Rewrite) : R(Rewrite) {}

  public:
    virtual void run(const MatchFinder::MatchResult &MR)
    {

    }

  private:
    Rewriter &R [[maybe_unused]];
};
/**********************************************************************************************************************/
class MatcherHandlerLVL0 : public AbstractMatcherHandler
{
  public:
    explicit MatcherHandlerLVL0 (Rewriter &Rewrite) : AbstractMatcherHandler(Rewrite) {}

    virtual ~MatcherHandlerLVL0() {}

    virtual void run(const MatchFinder::MatchResult &MR) override
    {

    }

  private:
};
/**********************************************************************************************************************/
class NameFinder
{
  public:
    NameFinder () {}

    class runDeclRefExprMatcher
    {
      public:
        runDeclRefExprMatcher (Rewriter &__rwrt) : LVL0Proto(__rwrt), __rwrt(__rwrt) {}

        virtual void runMatcher(const StringRef __sr, ASTContext &__ctx)
        {
          Matcher.addMatcher(declRefExpr(to(namedDecl(hasName(__sr.str())))).bind("declrefexpbyname"), &LVL0Proto);
          Matcher.matchAST(__ctx);
        }

      private:
        MatchFinder Matcher;
        MatcherHandlerLVL0 LVL0Proto;
        Rewriter __rwrt;
        StringRef __sr;
    };

    class runNamedDeclMatcher
    {
      public:
        runNamedDeclMatcher (Rewriter &__rwrt) : LVL0Proto(__rwrt), __rwrt(__rwrt) {}

        virtual void runMatcher(const StringRef __sr, ASTContext &__ctx)
        {
          Matcher.addMatcher(declRefExpr(to(namedDecl(hasName(__sr.str())))).bind("nameddeclbyname"), &LVL0Proto);
          Matcher.matchAST(__ctx);
        }

      private:
        MatchFinder Matcher;
        MatcherHandlerLVL0 LVL0Proto;
        Rewriter __rwrt;
        StringRef __sr;
    };

  private:
};
/**********************************************************************************************************************/
/**********************************************************************************************************************/
/**********************************************************************************************************************/
class IfBreaker : public MatchFinder::MatchCallback
{
  public:
    IfBreaker (Rewriter &Rewrite) : Rewrite(Rewrite) {}

    virtual void run(const MatchFinder::MatchResult &MR)
    {
      if (MR.Nodes.getNodeAs<clang::UnaryOperator>("uno") != nullptr)
      {
        const clang::UnaryOperator* UO = MR.Nodes.getNodeAs<clang::UnaryOperator>("uno");

        SourceLocation SL = UO->getLocStart();
        CheckSLValidity(SL);
        SL = Devi::SourceLocationHasMacro(SL, Rewrite);

        const Expr* EXP = UO->getSubExpr();
        const ast_type_traits::DynTypedNode DynNode = ast_type_traits::DynTypedNode::create<clang::Expr>(*EXP);
        bruiser::TypeInfo TIProto(&DynNode);

        const clang::Type* CTP [[maybe_unused]] = TIProto.getTypeInfo(MR.Context);

        NameFinder::runDeclRefExprMatcher DRENameMatcher(Rewrite);

        DRENameMatcher.runMatcher(StringRef(), *MR.Context);

      }

      if (MR.Nodes.getNodeAs<clang::BinaryOperator>("dous") != nullptr)
      {
        const clang::BinaryOperator* BO = MR.Nodes.getNodeAs<clang::BinaryOperator>("dous");

        SourceLocation SL = BO->getLocStart();
        CheckSLValidity(SL);
        SL = Devi::SourceLocationHasMacro(SL, Rewrite);

        const Expr* LHS = BO->getLHS();
        const Expr* RHS = BO->getRHS();

        QualType LQT = LHS->getType();
        QualType RQT = RHS->getType();

        const clang::Type* LTP = LQT.getTypePtr();
        const clang::Type* RTP = RQT.getTypePtr();

        const clang::Type* CLTP [[maybe_unused]] = MR.Context->getCanonicalType(LTP);
        const clang::Type* CRTP [[maybe_unused]] = MR.Context->getCanonicalType(RTP);
      }
    }

  private:
    Rewriter &Rewrite;
    MatchFinder Matcher;
};
/**********************************************************************************************************************/
/**
 * @brief Hijacks the main main and replaces it with bruiser's main.
 */
class MainWrapper : public MatchFinder::MatchCallback
{
public:
  MainWrapper (Rewriter &Rewrite) : Rewrite(Rewrite) {}

  virtual void run(const MatchFinder::MatchResult &MR)
  {
    if (MR.Nodes.getNodeAs<clang::FunctionDecl>("mainwrapper") != nullptr)
    {
      const FunctionDecl* FD = MR.Nodes.getNodeAs<clang::FunctionDecl>("mainwrapper");

      SourceLocation SL = FD->getLocStart();
      CheckSLValidity(SL);
      SL = Devi::SourceLocationHasMacro(SL, Rewrite);

      SourceLocation SLE = FD->getLocEnd();
      CheckSLValidity(SLE);
      SLE = Devi::SourceLocationHasMacro(SLE, Rewrite);

      if (!Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL) || Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
      {
        return void();
      }

      SourceRange SR(SL, SLE);

      std::string MainSig = Rewrite.getRewrittenText(SR);

      size_t mainbegin = MainSig.find("main");

      StringRef __sr("sub_main");

      Rewrite.ReplaceText(SL.getLocWithOffset(mainbegin), 4U, __sr);

      /*@DEVI-obviously the best way to do this is to use the main signature already used, instead of going with a general predefined one. the current form is a temp.*/
      Rewrite.InsertTextAfter(SLE.getLocWithOffset(1U), StringRef("\n\nint main(int argc, const char **argv)\n{\n\treturn sub_main(argc, argv);\n}\n"));

      BruiseRep.PrintToLog("hijacked main main.");
    }
  }

  private:
  Rewriter &Rewrite;
};
/**********************************************************************************************************************/
class LiveListFuncs : public MatchFinder::MatchCallback
{
  public:
    LiveListFuncs (Rewriter &R) : R(R) {}

    virtual void run(const MatchFinder::MatchResult &MR)
    {
      if (MR.Nodes.getNodeAs<clang::FunctionDecl>("livelistfuncs") != nullptr)
      {
        const clang::FunctionDecl* FD = MR.Nodes.getNodeAs<clang::FunctionDecl>("livelistfuncs");

        SourceLocation SL = FD->getLocStart();
        CheckSLValidity(SL);
        SL = Devi::SourceLocationHasMacro(SL, R);

        if (!Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL) || Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
        {
          return void();
        }

        if (FD->hasBody())
        {
          Stmt* Body = FD->getBody();
          SourceLocation SLBody = Body->getLocStart();
          SourceLocation SLShebang = FD->getLocStart();
          PRINT_WITH_COLOR_LB(CYAN, R.getRewrittenText(clang::SourceRange(SLShebang, SLBody.getLocWithOffset(-1))).c_str());
          PushToLua.push_back(R.getRewrittenText(clang::SourceRange(SLShebang, SLBody.getLocWithOffset(-1))));
        }
        else
        {
          SourceLocation SL = FD->getLocStart();
          SourceLocation SLE = FD->getLocEnd();
          PRINT_WITH_COLOR_LB(CYAN, R.getRewrittenText(clang::SourceRange(SL, SLE)).c_str());
          PushToLua.push_back(R.getRewrittenText(clang::SourceRange(SL, SLE)));
        }
      }
    }

  private:
    Rewriter R;
};
/**********************************************************************************************************************/
class LiveListVars : public MatchFinder::MatchCallback
{
  public:
    LiveListVars (Rewriter &R) : R(R) {}

    virtual void run(const MatchFinder::MatchResult &MR)
    {
      if (MR.Nodes.getNodeAs<clang::VarDecl>("livelistvars") != nullptr)
      {
        const clang::VarDecl* VD = MR.Nodes.getNodeAs<clang::VarDecl>("livelistvars");

        SourceLocation SL = VD->getLocStart();
        CheckSLValidity(SL);
        SL = Devi::SourceLocationHasMacro(SL, R);

        if (!Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL) || Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
        {
          return void();
        }

        PRINT_WITH_COLOR_LB(CYAN, R.getRewrittenText(SourceRange(VD->getLocStart(), VD->getLocEnd())).c_str());
        PushToLua.push_back(R.getRewrittenText(SourceRange(VD->getLocStart(), VD->getLocEnd())));
      }
    }

  private:
    Rewriter R;
};
/**********************************************************************************************************************/
class LiveListRecords : public MatchFinder::MatchCallback
{
  public:
    LiveListRecords (Rewriter &R) : R(R) {}

    virtual void run(const MatchFinder::MatchResult &MR)
    {
      if (MR.Nodes.getNodeAs<clang::RecordDecl>("livelistvars") != nullptr)
      {
        const clang::RecordDecl* RD = MR.Nodes.getNodeAs<clang::RecordDecl>("livelistvars");

        PRINT_WITH_COLOR_LB(CYAN, R.getRewrittenText(SourceRange(RD->getLocStart(), RD->getLocEnd())).c_str());
        PushToLua.push_back(R.getRewrittenText(SourceRange(RD->getLocStart(), RD->getLocEnd())));
      }
    }

  private:
    Rewriter R;
};
/**********************************************************************************************************************/
/**********************************************************************************************************************/
/**********************************************************************************************************************/
/**********************************************************************************************************************/
/**********************************************************************************************************************/
/**********************************************************************************************************************/
/**********************************************************************************************************************/
class BruiserASTConsumer : public ASTConsumer {

public:
  BruiserASTConsumer(Rewriter &R) : HIfBreaker(R), HMainWrapper(R)
  {}

  void HandleTranslationUnit(ASTContext &Context) override
  {
    Matcher.addMatcher(ifStmt(hasDescendant(expr(anyOf(unaryOperator().bind("uno"), binaryOperator().bind("dous"))))), &HIfBreaker);

    Matcher.addMatcher(functionDecl(hasName("main")).bind("mainwrapper"), &HMainWrapper);


    Matcher.matchAST(Context);
  }

private:
  IfBreaker HIfBreaker;
  MainWrapper HMainWrapper;
  MatchFinder Matcher;
  Rewriter R;
};
/**********************************************************************************************************************/
class LiveConsumerFactory
{
  public:
    LiveConsumerFactory() {}

    template<typename T>
      void operator()(T __consumer)
      {

      }

};
/**********************************************************************************************************************/
class LiveConsumer : public ASTConsumer
{
  public:
    LiveConsumer(Rewriter &R) : HLLVars(R), HLLFuncs(R), HLLRecords(R)
    {}

    void HandleTranslationUnit(ASTContext &ctx) override
    {
      Matcher.matchAST(ctx);
    }

  private:
    MatchFinder Matcher;
    LiveListVars HLLVars;
    LiveListFuncs HLLFuncs;
    LiveListRecords HLLRecords;
    Rewriter R;
};
/**********************************************************************************************************************/
class LiveListVarsConsumer : public ASTConsumer
{
  public:
    LiveListVarsConsumer(Rewriter &R) : HLLVars(R)
    {}

    void HandleTranslationUnit(ASTContext &ctx) override
    {
      Matcher.addMatcher(varDecl().bind("livelistvars"), &HLLVars);

      Matcher.matchAST(ctx);
    }

  private:
    LiveListVars HLLVars;
    Rewriter R;
    MatchFinder Matcher;
};
/**********************************************************************************************************************/
class LiveListFuncsConsumer : public ASTConsumer
{
  public:
    LiveListFuncsConsumer(Rewriter &R) : HLLFuncs(R)
    {}

    void HandleTranslationUnit(ASTContext &ctx) override
    {
      Matcher.addMatcher(functionDecl().bind("livelistfuncs"), &HLLFuncs);

      Matcher.matchAST(ctx);
    }

  private:
    MatchFinder Matcher;
    LiveListFuncs HLLFuncs;
    Rewriter R;
};
/**********************************************************************************************************************/
class LiveListClassConsumer : public ASTConsumer
{
  public:
    LiveListClassConsumer(Rewriter &R) : HLLRecords(R)
    {}

    void HandleTranslationUnit(ASTContext &ctx) override
    {
      Matcher.addMatcher(recordDecl(isClass()).bind("livelistclass"), &HLLRecords);

      Matcher.matchAST(ctx);
    }

  private:
    MatchFinder Matcher;
    LiveListRecords HLLRecords;
    Rewriter R;
};
/**********************************************************************************************************************/
class LiveListStructConsumer : public ASTConsumer
{
  public:
    LiveListStructConsumer(Rewriter &R) : HLLRecords(R)
    {}

    void HandleTranslationUnit(ASTContext &ctx) override
    {
      Matcher.addMatcher(recordDecl(isStruct()).bind("liveliststruct"), &HLLRecords);

      Matcher.matchAST(ctx);
    }

  private:
    MatchFinder Matcher;
    LiveListRecords HLLRecords;
    Rewriter R;
};
/**********************************************************************************************************************/
class LiveListUnionConsumer : public ASTConsumer
{
  public:
    LiveListUnionConsumer(Rewriter &R) : HLLRecords(R)
    {}

    void HandleTranslationUnit(ASTContext &ctx) override
    {
      Matcher.addMatcher(recordDecl(isUnion()).bind("livelistclass"), &HLLRecords);

      Matcher.matchAST(ctx);
    }

  private:
    MatchFinder Matcher;
    LiveListRecords HLLRecords;
    Rewriter R;
};
/**********************************************************************************************************************/
class LiveListArrayConsumer : public ASTConsumer
{
  public:
    LiveListArrayConsumer(Rewriter &R) : HLLVars(R)
    {}

    void HandleTranslationUnit(ASTContext &ctx) override
    {
      Matcher.addMatcher(varDecl(hasType(arrayType())).bind("livelistvars"), &HLLVars);

      Matcher.matchAST(ctx);
    }

  private:
    MatchFinder Matcher;
    LiveListVars HLLVars;
    Rewriter R;
};
/**********************************************************************************************************************/
/**********************************************************************************************************************/
/**********************************************************************************************************************/
/**********************************************************************************************************************/
class BlankDiagConsumer : public clang::DiagnosticConsumer
{
  public:
    BlankDiagConsumer() = default;
    virtual ~BlankDiagConsumer() {}
    virtual void HandleDiagnostic(DiagnosticsEngine::Level DiagLevel, const Diagnostic &Info) override {}
};
/**********************************************************************************************************************/
class BruiserFrontendAction : public ASTFrontendAction
{
public:
  BruiserFrontendAction() {}
  virtual ~BruiserFrontendAction()
  {
    delete BDCProto;
    delete tee;
  }

  void EndSourceFileAction() override
  {
    std::error_code EC;
    std::string OutputFilename = "./libtooling-tee";
    TheRewriter.getEditBuffer(TheRewriter.getSourceMgr().getMainFileID()).write(llvm::outs());
    tee = new raw_fd_ostream(StringRef(OutputFilename), EC, sys::fs::F_None);
    TheRewriter.getEditBuffer(TheRewriter.getSourceMgr().getMainFileID()).write(*tee);
  }

  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, StringRef file) override
  {
    DiagnosticsEngine &DE = CI.getPreprocessor().getDiagnostics();
    DE.setClient(BDCProto, false);
    TheRewriter.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
    return llvm::make_unique<BruiserASTConsumer>(TheRewriter);
  }

private:
  Rewriter TheRewriter;
  BlankDiagConsumer* BDCProto = new BlankDiagConsumer;
  raw_ostream *tee = &llvm::outs();
};
/**********************************************************************************************************************/
class LiveActionListVars : public ASTFrontendAction
{
  public:
    LiveActionListVars() = default;
    virtual ~LiveActionListVars()
    {
      delete BDCProto;
    }

    void EndSourceFileAction() override {}

    std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, StringRef file) override
    {
      DiagnosticsEngine &DE = CI.getPreprocessor().getDiagnostics();
      DE.setClient(BDCProto, false);
      TheRewriter.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
      return llvm::make_unique<LiveListVarsConsumer>(TheRewriter);
    }

  private:
    Rewriter TheRewriter;
    BlankDiagConsumer* BDCProto = new BlankDiagConsumer();
};
/**********************************************************************************************************************/
class LiveActionListFuncs : public ASTFrontendAction
{
  public:
    LiveActionListFuncs() {}
    ~LiveActionListFuncs()
    {
      delete BDCProto;
    }

    void EndSourceFileAction() override {}

    std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, StringRef file) override
    {
      DiagnosticsEngine &DE = CI.getPreprocessor().getDiagnostics();
      DE.setClient(BDCProto, false);
      TheRewriter.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
      return llvm::make_unique<LiveListFuncsConsumer>(TheRewriter);
    }

  private:
    Rewriter TheRewriter;
    BlankDiagConsumer* BDCProto = new BlankDiagConsumer;
};
/**********************************************************************************************************************/
class LiveActionListStructs : public ASTFrontendAction
{
  public:
    LiveActionListStructs() {}
    ~LiveActionListStructs()
    {
      delete BDCProto;
    }

    void EndSourceFileAction() override {}

    std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, StringRef file) override
    {
      DiagnosticsEngine &DE = CI.getPreprocessor().getDiagnostics();
      DE.setClient(BDCProto, false);
      TheRewriter.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
      return llvm::make_unique<LiveListStructConsumer>(TheRewriter);
    }

  private:
    Rewriter TheRewriter;
    BlankDiagConsumer* BDCProto = new BlankDiagConsumer;
};
/**********************************************************************************************************************/
class LiveActionListClasses : public ASTFrontendAction
{
  public:
    LiveActionListClasses() {}
    ~LiveActionListClasses()
    {
      delete BDCProto;
    }

    void EndSourceFileAction() override {}

    std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, StringRef file) override
    {
      DiagnosticsEngine &DE = CI.getPreprocessor().getDiagnostics();
      DE.setClient(BDCProto, false);
      TheRewriter.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
      return llvm::make_unique<LiveListClassConsumer>(TheRewriter);
    }

  private:
    Rewriter TheRewriter;
    BlankDiagConsumer* BDCProto = new BlankDiagConsumer;
};
/**********************************************************************************************************************/
class LiveActionListUnions : public ASTFrontendAction
{
  public:
    LiveActionListUnions() {}
    ~LiveActionListUnions()
    {
      delete BDCProto;
    }

    void EndSourceFileAction() override {}

    std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, StringRef file) override
    {
      DiagnosticsEngine &DE = CI.getPreprocessor().getDiagnostics();
      DE.setClient(BDCProto, false);
      TheRewriter.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
      return llvm::make_unique<LiveListUnionConsumer>(TheRewriter);
    }

  private:
    Rewriter TheRewriter;
    BlankDiagConsumer* BDCProto = new BlankDiagConsumer;
};
/**********************************************************************************************************************/
class LiveActionListArrays : public ASTFrontendAction
{
  public:
    LiveActionListArrays() {}
    ~LiveActionListArrays()
    {
      delete BDCProto;
    }

    void EndSourceFileAction() override {}

    std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, StringRef file) override
    {
      DiagnosticsEngine &DE = CI.getPreprocessor().getDiagnostics();
      DE.setClient(BDCProto, false);
      TheRewriter.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
      return llvm::make_unique<LiveListArrayConsumer>(TheRewriter);
    }

  private:
    Rewriter TheRewriter;
    BlankDiagConsumer* BDCProto = new BlankDiagConsumer;
};
/**********************************************************************************************************************/
/**********************************************************************************************************************/
/*lua wrappers*/
class LuaWrapper
{
  public:
    LuaWrapper(ClangTool &__CT, Executioner& __EX, XGlobals __XG) : CT(__CT), executioner(__EX), xglobals(__XG) {}

    /*print out the history*/
    int BruiserLuaHistory(lua_State* __ls)
    {
      std::ifstream historyfile;
      historyfile.open(SHELL_HISTORY_FILE);

      std::string tempstring;
      unsigned int tempint = 0;
      while(std::getline(historyfile, tempstring))
      {
        printf(BLUE"%d ", tempint);
        printf(CYAN"%s", tempstring.c_str());
        printf(NORMAL"\n");

        tempint++;
      }

      return tempint;
    }

    /*print the help menu*/
    int BruiserLuaHelp(lua_State* __ls)
    {
      unsigned int argcount = 0U;

      for (auto &iter : bruiser::CMDHelp)
      {
        printf(GREEN"name: ");
        printf(CYAN"%s ", iter.name.c_str());
        printf(GREEN"proto: ");
        printf(CYAN"%s ", iter.proto.c_str());
        printf(GREEN"description: ");
        printf(CYAN"%s ", iter.descr.c_str());
        printf(GREEN"prototype: ");
        printf(CYAN"%s ", iter.protoprefix.c_str());
        printf(GREEN"return value: ");
        printf(CYAN"%s ", iter.retval.c_str());
        printf(NORMAL"\n");
        argcount++;
      }

      std::cout << NORMAL;
      return argcount;
    }

    /*hijakcs the main main*/
    int BruiserLuaHijackMain(lua_State* __ls)
    {
        int RunResult = this->GetClangTool().run(newFrontendActionFactory<BruiserFrontendAction>().get());
        printf(CYAN"hijacking main returned %d", RunResult);
        printf(NORMAL"\n");

        std::ifstream libtooling_tee("../test/bruisertest/libtooling-tee");
        std::string luaoutstr;
        std::string dummy;

        while(std::getline(libtooling_tee, dummy))
        {
          luaoutstr = luaoutstr + dummy + "\n";
        }

        lua_pushstring(__ls, luaoutstr.c_str());

        return 1;
    }

    /*print out bruiser version*/
    int BruiserLuaVersion(lua_State* __ls)
    {
        PRINT_WITH_COLOR_LB(GREEN, "bruiser experimental version something.");
        PRINT_WITH_COLOR_LB(GREEN, "project mutator");
        PRINT_WITH_COLOR_LB(GREEN, "GPL v2.0");
        PRINT_WITH_COLOR_LB(GREEN, "bloodstalker 2017");

        return 1;
    }

    /*clear the screen*/
    int BruiserLuaClear(lua_State* __ls)
    {
      linenoiseClearScreen();
      return 0;
    }

    int BruiserPyLoader(lua_State* __ls ) {
      int numargs = lua_gettop(__ls);
      //std::string filename = "../bfd/load.py";
      std::string filename = "load";
      std::string funcname;
      std::string objjpath;
      std::string action;

      if (numargs == 3) {
        if (Verbose) std::cout << CYAN << "got args." << NORMAL << "\n";
        funcname = lua_tostring(__ls, 1);
        if (funcname == "") PRINT_WITH_COLOR_LB(RED, "first argument is nil");
        objjpath = lua_tostring(__ls, 2);
        if (objjpath == "") PRINT_WITH_COLOR_LB(RED, "second argument is nil");
        action = lua_tostring(__ls, 3);
        if (action == "") PRINT_WITH_COLOR_LB(RED, "third argument is nil");
        lua_pop(__ls, 3);
        std::cout << NORMAL;
      }
      else {
        std::cout << RED << "wrong number of arguments provided. should give the python script name, python func name and its args.\n" << NORMAL;
        return EXIT_FAILURE;
      }

      if (Verbose) std::cout << CYAN << "initing the py embed class...\n" << NORMAL;
      PyExec py(filename.c_str(), funcname.c_str(), objjpath.c_str());

      if (Verbose) std::cout << BLUE << "running load.py: " << NORMAL << "\n";
      if (py.run() == EXIT_FAILURE) return 0;
      if (action == "code_list") {
        py.getAsCppByte();
        //py.printHexObjs();

        int tableindex1 = 1;
        int tableindex2 = 1;
        // the return type to lua is a table of tables
        lua_newtable(__ls);
        if (!lua_checkstack(__ls, py.exportObjs().size())) {
          PRINT_WITH_COLOR_LB(RED, "cant grow lua stack. current size is too small.");
        }
        for (auto& iter : py.exportObjs()) {
          lua_pushnumber(__ls, tableindex1);
          lua_newtable(__ls);
          for (auto& iterer : iter) {
            lua_pushnumber(__ls, tableindex2);
            tableindex2++;
            lua_pushnumber(__ls, iterer);
            lua_settable(__ls, -3);
          }
          tableindex2 = 1;
          tableindex1++;
          lua_settable(__ls, -3);
        }
      } else if (action == "symbol_list") {
        py.getAsCppStringVec();
        int tableindex = 1 ;
        // the return type to lua is a table
        lua_newtable(__ls);
        if (!lua_checkstack(__ls, py.exportStrings().size())) {
          PRINT_WITH_COLOR_LB(RED, "cant grow lua stack. current size is too small.");
        }
        for (auto& iter : py.exportStrings()) {
          lua_pushnumber(__ls, tableindex);
          tableindex++;
          lua_pushstring(__ls, iter.c_str());
          lua_settable(__ls, -3);
        }
      } else if (action == "bytes") {
        py.getAsCppByte_PyIntList();
        int tableindex = 1 ;
        // the return type to lua is a table
        lua_newtable(__ls);
        if (!lua_checkstack(__ls, py.exportStrings().size())) {
          PRINT_WITH_COLOR_LB(RED, "cant grow lua stack. current size is too small.");
        }
        for (auto& iter : py.exportTextSection()) {
          lua_pushnumber(__ls, tableindex);
          tableindex++;
          lua_pushinteger(__ls, iter);
          lua_settable(__ls, -3);
        }
      }

      if (Verbose) PRINT_WITH_COLOR_LB(GREEN, "done.");
      return 1;
    }

    int BruiserLuaxobjRegister(lua_State* __ls) {
      int numargs = lua_gettop(__ls);
      if (numargs != 2) {
        PRINT_WITH_COLOR_LB(RED, "arg number should be 2.");
      }
      std::vector<uint8_t> xobj_code_;
      std::string xobj_name;
      int table_length = lua_rawlen(__ls, 1);
      if (lua_type(__ls, 1) != LUA_TTABLE) {
        PRINT_WITH_COLOR_LB(RED, "the stack value is not a table but is being accessed as such.");
      } else {
        if (Verbose) PRINT_WITH_COLOR_LB(GREEN, "stack index 1 is a table.");
      }
      if (Verbose) std::cout << CYAN << "table_length: " << table_length << NORMAL << "\n";
      if (!lua_checkstack(__ls, table_length)) {
        PRINT_WITH_COLOR_LB(RED, "cant grow lua stack. current size is too small.");
      }
      for (int i = 1; i <= table_length; ++i) {
        lua_rawgeti(__ls, 1, i);
        xobj_code_.push_back(int(lua_tonumber(__ls, i + 2)));
      }
      if (Verbose) {
        std::cout << BLUE << "function code: ";
        for (auto& iter : xobj_code_) {std::cout << NORMAL << int(iter) << " ";}
        std::cout << NORMAL  <<"\n";
      }
      xobj_name = lua_tostring(__ls, 2);
      std::pair<void*, size_t> xobj = executioner.loadObjsInXMem(xobj_code_);
      if (Verbose) {
        std::cout << "xobj will be registered as " << YELLOW << xobj_name << NORMAL << ". " << "it is recommended to use a post- or pre-fix for the xobj names to avoid namespace pollution." "\n";
        std::cout << GREEN << "pointer: " << BLUE << xobj.first << " " << GREEN << "size: " << BLUE << xobj.second << NORMAL << "\n";
      }
      executioner.pushvptr(xobj.first, xobj_name);
      return 0;
    }

    int BruiserLuaCallX(lua_State* __ls) {
      int numargs = lua_gettop(__ls);
      if (numargs != 5) {PRINT_WITH_COLOR_LB(RED, "xcall: bad number of args. expected exactly five.");}
      int argc = lua_tointeger(__ls, 1);
      // 2-table of strings
      std::string ffi_ret_type_string = lua_tostring(__ls, 3);

      void* x_ptr; 
      if (lua_type(__ls, 4) == LUA_TNUMBER) {
        x_ptr = executioner.getvptrbyindex(lua_tointeger(__ls, 4)).first;
      } else if (lua_type(__ls, 4) == LUA_TSTRING) {
        x_ptr = executioner.getvptrbyname(lua_tostring(__ls, 4)).first;
      } else {
        PRINT_WITH_COLOR_LB(RED, "argument 4 is neihter an index nor a string.");
        lua_pushnil(__ls);
        return 1;
      }
      // 5-the actual args-table of values

      // @DEVI-FIXME: currently we are not handling structs at all
      ffi_type ret_type;
      if (std::strcmp(ffi_ret_type_string.c_str(), "void") == 0) {ret_type = ffi_type_void;}
      else if (std::strcmp(ffi_ret_type_string.c_str(), "uint8") == 0) {ret_type = ffi_type_uint8;}
      else if (std::strcmp(ffi_ret_type_string.c_str(), "sint8") == 0) {ret_type = ffi_type_sint8;}
      else if (std::strcmp(ffi_ret_type_string.c_str(), "uint16") == 0) {ret_type = ffi_type_uint16;}
      else if (std::strcmp(ffi_ret_type_string.c_str(), "sint16") == 0) {ret_type = ffi_type_sint16;}
      else if (std::strcmp(ffi_ret_type_string.c_str(), "uint32") == 0) {ret_type = ffi_type_uint32;}
      else if (std::strcmp(ffi_ret_type_string.c_str(), "sint32") == 0) {ret_type = ffi_type_sint32;}
      else if (std::strcmp(ffi_ret_type_string.c_str(), "uint64") == 0) {ret_type = ffi_type_uint64;}
      else if (std::strcmp(ffi_ret_type_string.c_str(), "sint64") == 0) {ret_type = ffi_type_sint64;}
      else if (std::strcmp(ffi_ret_type_string.c_str(), "float") == 0) {ret_type = ffi_type_float;}
      else if (std::strcmp(ffi_ret_type_string.c_str(), "double") == 0) {ret_type = ffi_type_double;}
      else if (std::strcmp(ffi_ret_type_string.c_str(), "string") == 0) {ret_type = ffi_type_pointer;}
      else if (std::strcmp(ffi_ret_type_string.c_str(), "pointer") == 0) {ret_type = ffi_type_pointer;}
      else if (std::strcmp(ffi_ret_type_string.c_str(), "struct") == 0) {ret_type = ffi_type_pointer;}
      else {PRINT_WITH_COLOR_LB(RED, "unknown return type string.");return 0;
      }

      const char* args[argc];
      int table_length_2 = lua_rawlen(__ls, 2);
      if (lua_type(__ls, 2) != LUA_TTABLE) {
        PRINT_WITH_COLOR_LB(RED, "xcall: the stack value is not a table but is being accessed as such.");
        return 0;
      } else {
        if (Verbose) PRINT_WITH_COLOR_LB(GREEN, "xcall: stack index 2 is a table.");
      }
      if (Verbose) std::cout << CYAN << "table_length: " << table_length_2 << NORMAL << "\n";
      for (int i = 1; i <= table_length_2; ++i) {
        lua_rawgeti(__ls, 2, i);
        args[i-1] = lua_tostring(__ls, i + numargs);
      }

      std::list<uint64_t> uints;
      std::list<int64_t> ints;
      std::list<std::string> strings;
      std::list<bool> bools;
      std::list<uintptr_t> ptrs;
      std::list<float> floats;
      std::list<double> doubles;
      void* values[argc];
      int table_length_5 = lua_rawlen(__ls, 5);

      if (!lua_checkstack(__ls, 10)) {
        PRINT_WITH_COLOR_LB(RED, "cant grow lua stack. current size is too small.");
        return 0;
      }
      if (lua_type(__ls, 5) != LUA_TTABLE) {
        PRINT_WITH_COLOR_LB(RED, "xcall: the stack value is not a table but is being accessed as such.");
        return 0;
      } else {
        if (Verbose) PRINT_WITH_COLOR_LB(GREEN, "xcall: stack index 5 is a table.");
      }

      if (Verbose) std::cout << CYAN << "table_length: " << table_length_5 << NORMAL << "\n";
      for (int i = 1; i <= table_length_5; ++i) {
        lua_rawgeti(__ls, 5, i);
        if (lua_type(__ls, i+numargs+argc) == LUA_TBOOLEAN) {}
        else if (lua_type(__ls, i+numargs+argc) == LUA_TLIGHTUSERDATA) {}
        else if (lua_type(__ls, i+numargs+argc) == LUA_TNUMBER) {
          double dummy = lua_tonumber(__ls, i + numargs + argc);
          if (dummy == (long long int)dummy) { // FIXME
            uints.push_back(lua_tointeger(__ls, i + numargs + argc));
            values[i-1]=&uints.back();
          } // int
          else {
            doubles.push_back(lua_tonumber(__ls, i + numargs + argc));
            values[i-1]=&doubles.back();
          } // float
        }
        else if (lua_type(__ls, i+numargs+argc) == LUA_TSTRING) {
          strings.push_back(lua_tostring(__ls, i + numargs + argc));
          values[i-1]=&strings.back();
        }
        else if (lua_type(__ls, i+numargs+argc) == LUA_TTABLE) {}
        else if (lua_type(__ls, i+numargs+argc) == LUA_TFUNCTION) {}
        else if (lua_type(__ls, i+numargs+argc) == LUA_TUSERDATA) {}
        else if (lua_type(__ls, i+numargs+argc) == LUA_TTHREAD) {}
      }

      void* result;
      if (x_ptr != nullptr) {
        result = ffi_callX(argc, args, ret_type, x_ptr, values);
        if (result == nullptr) {PRINT_WITH_COLOR_LB(RED, "ffi_callX returned null.");return 0;}
        
        if (std::strcmp(ffi_ret_type_string.c_str(), "void") == 0) {lua_pushnil(__ls);}
        else if (std::strcmp(ffi_ret_type_string.c_str(), "uint8") == 0) {lua_pushinteger(__ls, ffi_reinterpret_uint8_t(result));}
        else if (std::strcmp(ffi_ret_type_string.c_str(), "sint8") == 0) {lua_pushinteger(__ls, ffi_reinterpret_int8_t(result));}
        else if (std::strcmp(ffi_ret_type_string.c_str(), "uint16") == 0) {lua_pushinteger(__ls, ffi_reinterpret_uint16_t(result));}
        else if (std::strcmp(ffi_ret_type_string.c_str(), "sint16") == 0) {lua_pushinteger(__ls, ffi_reinterpret_int16_t(result));}
        else if (std::strcmp(ffi_ret_type_string.c_str(), "uint32") == 0) {lua_pushinteger(__ls, ffi_reinterpret_uint32_t(result));}
        else if (std::strcmp(ffi_ret_type_string.c_str(), "sint32") == 0) {lua_pushinteger(__ls, ffi_reinterpret_int32_t(result));}
        else if (std::strcmp(ffi_ret_type_string.c_str(), "uint64") == 0) {lua_pushinteger(__ls, ffi_reinterpret_uint64_t(result));}
        else if (std::strcmp(ffi_ret_type_string.c_str(), "sint64") == 0) {lua_pushinteger(__ls, ffi_reinterpret_int64_t(result));}
        else if (std::strcmp(ffi_ret_type_string.c_str(), "float") == 0) {lua_pushnumber(__ls, ffi_reinterpret_float(result));}
        else if (std::strcmp(ffi_ret_type_string.c_str(), "double") == 0) {lua_pushnumber(__ls, ffi_reinterpret_double(result));}
        else if (std::strcmp(ffi_ret_type_string.c_str(), "string") == 0) {lua_pushstring(__ls, ffi_reinterpret_string(result));}
        else if (std::strcmp(ffi_ret_type_string.c_str(), "pointer") == 0) {lua_pushinteger(__ls, ffi_reinterpret_uintptr_t(result));}
        else if (std::strcmp(ffi_ret_type_string.c_str(), "struct") == 0) {}
        else {PRINT_WITH_COLOR_LB(RED, "unknown return type string.");return 0;}
        return 1;
      } else {
        PRINT_WITH_COLOR_LB(RED, "the index is too high into the xobj vector.");
        return 0;
      }
    }

    int BruiserLuaXObjGetList(lua_State* __ls) {
      auto xlist = executioner.getvptrs();
        lua_newtable(__ls);
        if (!lua_checkstack(__ls, xlist.size())) {
          PRINT_WITH_COLOR_LB(RED, "cant grow lua stack. current size is too small.");
        }
        for (auto& iter : xlist) {
          if (Verbose) std::cout << CYAN << iter.second << NORMAL;
          lua_pushstring(__ls, iter.second.c_str());
          if (Verbose) std::cout << " " << MAGENTA << (long int)iter.first << NORMAL <<"\n";
          lua_pushinteger(__ls, (long int)iter.first);
          lua_settable(__ls, -3);
        }
      return 1;
    }

    int BruiserLuaXObjAllocGlobal(lua_State* __ls) {
      int numargs = lua_gettop(__ls);
      if (numargs != 2) {PRINT_WITH_COLOR_LB(RED, "expected exactly two args. did not get that.");return 0;}
      std::string glob_name = lua_tostring(__ls , 1);
      size_t size = lua_tointeger(__ls, 2);
      xglobals.reserve(size);
      return 0;
    }

    int BruiserLuaXObjAllocAllGlobals(lua_State* __ls) {}

    int BruiserGetJumpTable(lua_State* __ls) {
      int numargs = lua_gettop(__ls);
      if (numargs != 2) {PRINT_WITH_COLOR_LB(RED, "expected exactly two args. did not get that.");return 0;}
      uint64_t size = lua_tointeger(__ls, 1);
      PRINT_WITH_COLOR_LB(CYAN, "cpp:calling getluatableint...");
      std::vector<uint8_t> code_v = getLuaTableInt<uint8_t>(__ls, 2, 2);
      PRINT_WITH_COLOR_LB(GREEN, "cpp:called getluatableint...");
      if (Verbose) PRINT_WITH_COLOR_LB(BLUE, "making jump table...");
      auto head = makejmptable(size, code_v.data(), Verbose, __ls);
      if (Verbose) PRINT_WITH_COLOR_LB(GREEN, "finished makejmptable call.");
      jmpt_push_args(__ls, head);
      new_jmpt_2(__ls);
      return 1;
    }

    int BruiserFreeJumpTable(lua_State* __ls) {
      int numargs = lua_gettop(__ls);
      if (numargs != 1) {PRINT_WITH_COLOR_LB(RED, "expected exactly one argument.");}
      uint64_t head = lua_tointeger(__ls, 1);
      freejmptable((JMP_S_T*)head);
      return 0;
    }

    int BruiserDumpJumpTable(lua_State* __ls) {
      int numargs = lua_gettop(__ls);
      if (numargs != 1) {PRINT_WITH_COLOR_LB(RED, "expected exactly one argument of type lightuserdata.");}
      uint64_t ptr = lua_tointeger(__ls, 1);
      dumpjmptable((JMP_S_T*)ptr);
      return 0;
    }

    /*read the m0 report*/
    int BruiserLuaM0(lua_State* __ls)
    {
        bruiser::ReadM0 M0Rep;
        tinyxml2::XMLError XMLErr;

        XMLErr = M0Rep.LoadXMLDoc();
        if (XMLErr != XML_SUCCESS)
        {
          std::cout << RED << "could not load m0 xml report.\n" << NORMAL;
          std::cout << RED << "tinyxml2 returned " << XMLErr << NORMAL;
          return XMLErr;
        }

        XMLErr = M0Rep.ReadFirstElement();
        if (XMLErr != XML_SUCCESS)
        {
          std::cerr << RED << "could not read first element of m0 xml report.\n" << NORMAL;
          return XMLErr;
        }

        bruiser::SearchM0(M0Rep.getRootPointer());

        return 1;
    }

    int BruiserLuaReadXMLReport(lua_State* __ls)
    {
      int numargs = lua_gettop(__ls);
      std::string xml_address;

      if (numargs == 1)
      {
        xml_address = lua_tostring(__ls, 1);
      }
      else
      {
        xml_address = bruiser::M0REP;
      }

      bruiser::ReadM0 MutagenRep(xml_address.c_str());
      tinyxml2::XMLError error;

      error = MutagenRep.LoadXMLDoc();
      if (error != XML_SUCCESS)
      {
        PRINT_WITH_COLOR_LB(RED, "could not load report.");
        PRINT_WITH_COLOR(RED, "tinyxml2 returned ");
        std::cout << RED << error << NORMAL;
        lua_pushnumber(__ls, (double)error);
        return 1;
      }

      error = MutagenRep.ReadFirstElement();
      if (error != XML_SUCCESS)
      {
        PRINT_WITH_COLOR_LB(RED, "could not read first element of xml report.");
        lua_pushnumber(__ls, (double)error);
        return 1;
      }

      bruiser::SearchM0(MutagenRep.getRootPointer());

      return 1;
    }

    /*quit*/
    int BruiserLuaQuit(lua_State* __ls)
    {
      dostring(__ls, "os.exit()", "test");
      return 0;
    }

    /*quit*/
    int BruiserLuaExit(lua_State* __ls)
    {
      dostring(__ls, "os.exit()", "test");
      return 0;
    }

    int BruiserLuaRunMake(lua_State* __ls)
    {
      unsigned int args = 0U;

      if ((args = lua_gettop(__ls)) != 1U)
      {
        PRINT_WITH_COLOR_LB(RED, "function was not called by one argument. Run help().");
        lua_pushnumber(__ls, 1);
        return 1;
      }

      std::string makearg = lua_tostring(__ls , 1);

      if (ShellGlobalInstance.MAKEPATH == "")
      {
        PRINT_WITH_COLOR_LB(RED, "MAKEPATH is not set. set it using setmakepath or type help.");
        lua_pushnumber(__ls, 1);
        return 1;
      }

      pid_t pid = fork();

      if (pid < 0)
      {
        PRINT_WITH_COLOR_LB(RED, "could not fork...");
        lua_pushnumber(__ls, EXIT_FAILURE);
      }

      if (pid == 0)
      {
        std::cout << BLUE << "MAKEPATH: " << ShellGlobalInstance.MAKEPATH << NORMAL << "\n";
        std::cout << BLUE << "Running: " << "make -C " << ShellGlobalInstance.MAKEPATH << " " << makearg << NORMAL << "\n";
        int retval = execl("/usr/bin/make", "make", "-C", ShellGlobalInstance.MAKEPATH.c_str(), makearg.c_str(), NULL);
        lua_pushnumber(__ls, retval);
        exit(EXIT_SUCCESS);
      }

      if (pid > 0)
      {
        int status;
        pid_t returned;
        returned = waitpid(pid, &status, 0);
        lua_pushnumber(__ls, returned);
      }

      return 1;
    }

    int BruiserLuaChangeHistorySize(lua_State* __ls)
    {
      unsigned int args = 0U;

      if ((args = lua_gettop(__ls)) != 1U)
      {
        PRINT_WITH_COLOR_LB(RED, "function was not called by one argument. Run help().");
        return 0;
      }

      unsigned int historysize = lua_tonumber(__ls, 1);

      linenoiseHistorySetMaxLen(historysize);
      ShellGlobalInstance.HISTORY_SIZE = historysize;

      return 0;
    }

    //@DEVI-FIXME-broken
    int BruiserLuaShowSourcecode(lua_State* __ls)
    {
      unsigned int args = 0U;

      if ((args = lua_gettop(__ls)) != 3U)
      {
        PRINT_WITH_COLOR_LB(RED, "function called with the wrong number of arguments. Run help().");
        return 0;
      }

      unsigned int linebegin = lua_tonumber(__ls, 1);
      unsigned int lineend = lua_tonumber(__ls, 2);
      std::string filename = lua_tostring(__ls, 3);
      std::fstream targetfile;

      for(auto &iter : ShellGlobalInstance.SOURCE_FILES)
      {
        if (iter.rfind(filename) == iter.size() - filename.size())
        {
          ShellCacheInstance.LastFileUsed = iter;
          ShellCacheInstance.LastFileUsedShort = filename;

          targetfile.open(iter);

          if(targetfile.rdstate() != std::ios_base::goodbit)
          {
            PRINT_WITH_COLOR_LB(RED, "could not open the file.");
          }
        }
      }

      std::string line;
      std::string dummy;
      while(getline(targetfile, line))
      {
        dummy += line;
        //lua_pushstring(__ls, line.c_str());
      }

      lua_pushstring(__ls , dummy.c_str());
      targetfile.close();
      //return lineend - linebegin + 1U;
      return 1;
    }

    int BruiserLuaMutagenExtraction(lua_State* __ls)
    {
      int numargs = lua_gettop(__ls);
      std::string extractiontarget;

      if (numargs == 1)
      {
        extractiontarget = lua_tostring(__ls, 1);
      }

      pid_t pid = fork();

      if (pid < 0)
      {
        /*bruiser could not spawn a child*/
        PRINT_WITH_COLOR_LB(RED, "could not fork a child process(m0).");
        lua_pushnumber(__ls, EXIT_FAILURE);
      }

      /*only the child process runs this*/
      if (pid == 0)
      {
        for(auto &iter : ShellGlobalInstance.SOURCE_FILES)
        {
          if (iter.rfind(extractiontarget) == iter.size() - extractiontarget.size())
          {
            ShellCacheInstance.LastFileUsedShort = extractiontarget;
            ShellCacheInstance.LastFileUsed = iter;
            std::cout << BLUE << "running: " << CYAN << "../mutator-lvl0 " << iter.c_str() << NORMAL << "\n";
            //int retval = execl("../", "mutator-lvl0", iter.c_str(), NULL);
            int retval = execl("../mutator-lvl0", "mutator-lvl0", iter.c_str(), NULL);
            std::cout << BLUE << "child process retuned " << retval << NORMAL << "\n";
            lua_pushnumber(__ls, retval);
            exit(EXIT_SUCCESS);
          }
        }
      }

      /*only the parent process runs this*/
      if (pid > 0)
      {
        /*the parent-bruiser- will need to wait on the child. the child will run m0.*/
        int status;
        pid_t returned;
        returned =  waitpid(pid, &status, 0);
        lua_pushnumber(__ls, returned);
      }

      return 1;
    }

    int BruiserLuaDropToCLI(lua_State* __ls) {
      int numargs = lua_gettop(__ls);
      if (numargs != 0) {
        PRINT_WITH_COLOR_LB(RED, "wrong number of args. should be called with no arguments.");
        return 0;
      }
      ShellGlobalInstance.droptocli = true;
    }

    int BruiserLuaStrainRecognition(lua_State* __ls)
    {
      unsigned int numthreads = std::thread::hardware_concurrency();
      lua_pushnumber(__ls, numthreads);
      return 1;
    }

    int BruiserLuaSetMakePath(lua_State* __ls)
    {
      int numargs = lua_gettop(__ls);

      if (numargs != 1)
      {
        PRINT_WITH_COLOR_LB(RED, "wrong number of args. run help.");
        return 0;
      }

      ShellGlobalInstance.MAKEPATH = lua_tostring(__ls, 1);

      return 0;
    }

    int BruiserLuaRun(lua_State* __ls)
    {
      int numargs = lua_gettop(__ls);

      if (numargs != 1)
      {
        PRINT_WITH_COLOR_LB(RED, "wrong number of args. run help().");
        lua_pushnumber(__ls, 1);
        return 1;
      }

      if (ShellGlobalInstance.BINPATH == "")
      {
        PRINT_WITH_COLOR_LB(RED, "BINPATH is not set. use setbinpath() to set it.");
        lua_pushnumber(__ls, 1);
        return 1;
      }

      std::string binname = lua_tostring(__ls, 1);

      pid_t pid = fork();

      if (pid < 0)
      {
        PRINT_WITH_COLOR_LB(RED, "could not fork...");
        lua_pushnumber(__ls, 1);
        return 1;
      }

      if (pid == 0)
      {
        int retval = execl((ShellGlobalInstance.BINPATH + "/" + binname).c_str(), binname.c_str(), NULL);
        std::cout << BLUE << "child returned " << retval << NORMAL << "\n";
        lua_pushnumber(__ls, retval);
        exit(EXIT_SUCCESS);
      }

      if (pid > 0)
      {
        unsigned int current_time [[maybe_unused]] = 0U;

        while (current_time < GLOBAL_TIME_OUT)
        {
          int status;
          int returned;
          returned = waitpid(-1, &status, WNOHANG);

          if (returned < 0)
          {
            lua_pushnumber(__ls, 1);
            break;
          }

          if (WIFEXITED(status) || WIFSIGNALED(status))
          {
            lua_pushnumber(__ls, 0);
            break;
          }

          current_time++;
        }
      }

      return 1;
    }

    int BruiserLuaSetBinPath(lua_State* __ls)
    {
      int numargs = lua_gettop(__ls);

      if (numargs != 1)
      {
        PRINT_WITH_COLOR_LB(RED, "wrong number of args. should be one string. see help().");
      }

      ShellGlobalInstance.BINPATH = lua_tostring(__ls, 1);

      return 0;
    }

    int BruiserLuaGetBinPath(lua_State* __ls)
    {
      lua_pushstring(__ls, ShellGlobalInstance.BINPATH.c_str());
      std::cout << BLUE << ShellGlobalInstance.BINPATH << NORMAL << "\n";
      return 1;
    }

    int BruiserLuaGetMakePath(lua_State* __ls)
    {
      lua_pushstring(__ls, ShellGlobalInstance.MAKEPATH.c_str());
      std::cout << BLUE << ShellGlobalInstance.MAKEPATH << NORMAL << "\n";
      return 1;
    }

    int BruiserLuaGetPath(lua_State* __ls)
    {
      unsigned int returncount = 0;

      for (auto &iter : ShellGlobalInstance.PATH)
      { lua_pushstring(__ls, iter.c_str());
        std::cout << BLUE << iter.c_str() << NORMAL << "\n";
        returncount++;
      }

      return returncount;
    }

    int BruiserLuaGetSourceFiles(lua_State* __ls)
    {
      unsigned int returncount = 0;

      for (auto &iter : ShellGlobalInstance.SOURCE_FILES)
      {
        lua_pushstring(__ls, iter.c_str());
        std::cout << BLUE << iter.c_str() << NORMAL << "\n";
        returncount++;
      }

      return returncount;
    }

    int BruiserLuaCallXFunc(lua_State* __ls) {
      int numargs = lua_gettop(__ls);
      std::string argtype;

      if (numargs % 2 != 0) {
        PRINT_WITH_COLOR_LB(RED, "Each arg should be accompanied with its type.");
      }

      for (int i = 0; i < numargs; i = i + 2) {
        argtype = lua_tostring(__ls, i);
      }

      return 0;
    }

    int BruiserLuaChangeDirectory(lua_State* __ls)
    {
      int numargs = lua_gettop(__ls);

      if (numargs != 1)
      {
        PRINT_WITH_COLOR_LB(RED, "wrond number of arguments. needed one. see help().");
        lua_pushnumber(__ls, 1U);
        return 1;
      }

      std::string destinationpath = lua_tostring(__ls, 1);

      pid_t pid = fork();

      if (pid < 0)
      {
        PRINT_WITH_COLOR_LB(RED, "could not fork...");
        lua_pushnumber(__ls, EXIT_FAILURE);
        return 1;
      }

      if (pid == 0)
      {
        int retval = execl("/usr/bin/cd", "cd", destinationpath.c_str(), NULL);
        std::cout << BLUE << "child returned " << retval << NORMAL << "\n";
      }

      if (pid > 0)
      {
        int status;
        pid_t returned;
        returned =  waitpid(pid, &status, 0);
        lua_pushnumber(__ls, returned);
      }

      return 1;
    }

    int BruiserLuaYolo(lua_State* __ls)
    {
      return 0;
    }

    int BruiserLuaListObjects(lua_State* __ls) {
      // @DEVI-has one string object to signify what kind of object to list
      return 0;
    }

    int BruiserLuaPWD(lua_State* __ls)
    {
      pid_t pid = fork();

      if (pid < 0)
      {
        PRINT_WITH_COLOR_LB(RED, "could not fork...");
        lua_pushnumber(__ls, EXIT_FAILURE);
        return 1;
      }

      if (pid == 0)
      {
        int retval = execl("/usr/bin/pwd", "pwd", NULL);
        std::cout << BLUE << "child returned " << retval << NORMAL << "\n";
      }

      if (pid > 0)
      {
        int status;
        pid_t returned;
        returned =  waitpid(pid, &status, 0);
        lua_pushnumber(__ls, returned);
      }

      return 1;
    }

#define LIST_GENERATOR(__x1) \
    int List##__x1(lua_State* __ls)\
    {\
      assert(PushToLua.size() == 0);\
      unsigned int InArgCnt = 0U;\
      InArgCnt = lua_gettop(__ls);\
      unsigned int returncount=0U;\
      this->GetClangTool().run(newFrontendActionFactory<LiveActionList##__x1>().get());\
      for(auto &iter : PushToLua)\
      {lua_pushstring(__ls, iter.c_str());returncount++;}\
      PushToLua.clear();\
      return returncount;\
    }

#define LIST_LIST_GENERATORS \
    X(Funcs, "lists all functions") \
    X(Vars, "lists all variables") \
    X(Arrays, "lists all arrays") \
    X(Classes, "lists all classes") \
    X(Structs, "lists all structs") \
    X(Unions, "lists all unions") \

#define X(__x1, __x2) LIST_GENERATOR(__x1)

    LIST_LIST_GENERATORS

#undef X
#undef LIST_GENERATOR

    ClangTool GetClangTool(void)
    {
      return this->CT;
    }

  private:
    ClangTool CT;
    Executioner executioner;
    XGlobals xglobals;
};
/**********************************************************************************************************************/
/**********************************************************************************************************************/
typedef int (LuaWrapper::*mem_func)(lua_State* L);
template<mem_func func>
int LuaDispatch(lua_State* __ls)
{
  LuaWrapper* LWPtr = *static_cast<LuaWrapper**>(lua_getextraspace(__ls));
  return ((*LWPtr).*func)(__ls);
}
/**********************************************************************************************************************/
/**********************************************************************************************************************/
/*Main*/
int main(int argc, const char **argv) {
  /*initializing the log*/
  bruiser::BruiserReport BruiserLog;

  /*initing xobj stuff*/
  Executioner executioner;
  Arguary arguary;
  XGlobals xglobals;

  /*gets the compilation database and options for the clang instances that we would later run*/
  CommonOptionsParser op(argc, argv, BruiserCategory);
  CompilationDatabase &CDB = op.getCompilations();
  std::vector<CompileCommand> CCV = CDB.getAllCompileCommands();
  /*populating the shellglobalinstance*/
  CompilationDatabaseProcessor CDBP(CDB);
  ClangTool Tool(op.getCompilations(), op.getSourcePathList());

  /*checking whether the compilation database is found and not empty if Nosrc is set*/
  if (CDBP.CompilationDatabseIsEmpty()) {
    PRINT_WITH_COLOR_LB(RED, "Nosrc is set and bruiser can't find the compilation database. quitting...");
    return 1;
  } else {
    CDBP.CalcMakePath();
    CDBP.PopulateGPATH();
    CDBP.PopulateGSOURCEFILES();
  }

  /*initialize the LuaWrapper class so we can register and run them from lua.*/
  LuaWrapper LW(Tool, executioner, xglobals);

  /*linenoise init*/
  linenoiseSetCompletionCallback(bruiser::ShellCompletion);
  linenoiseSetHintsCallback(bruiser::ShellHints);
  /*setting up the initial history size to SHELL_HISTORY_SIZE*/
  linenoiseHistorySetMaxLen(SHELL_HISTORY_SIZE);
  linenoiseHistoryLoad(SHELL_HISTORY_FILE);
  linenoiseSetMultiLine(1);

  /*start running the cli*/
  {
    char* command;

    LuaEngine LE;
    LE.LoadEverylib();
    LE.RunLuaDefaults();
    LE.registerJMPTable();
    *static_cast<LuaWrapper**>(lua_getextraspace(LE.GetLuaState())) = &LW;

    /*@DEVI-this part is just registering our LuaWrapper member functions with lua so we can call them from lua.*/
    lua_register(LE.GetLuaState(), "history", &LuaDispatch<&LuaWrapper::BruiserLuaHistory>);
    lua_register(LE.GetLuaState(), "help", &LuaDispatch<&LuaWrapper::BruiserLuaHelp>);
    lua_register(LE.GetLuaState(), "hijackmain", &LuaDispatch<&LuaWrapper::BruiserLuaHijackMain>);
    lua_register(LE.GetLuaState(), "version", &LuaDispatch<&LuaWrapper::BruiserLuaVersion>);
    lua_register(LE.GetLuaState(), "clear", &LuaDispatch<&LuaWrapper::BruiserLuaClear>);
    lua_register(LE.GetLuaState(), "m0", &LuaDispatch<&LuaWrapper::BruiserLuaM0>);
    lua_register(LE.GetLuaState(), "readxmlfile", &LuaDispatch<&LuaWrapper::BruiserLuaReadXMLReport>);
    lua_register(LE.GetLuaState(), "quit", &LuaDispatch<&LuaWrapper::BruiserLuaQuit>);
    lua_register(LE.GetLuaState(), "exit", &LuaDispatch<&LuaWrapper::BruiserLuaExit>);
    lua_register(LE.GetLuaState(), "make", &LuaDispatch<&LuaWrapper::BruiserLuaRunMake>);
    lua_register(LE.GetLuaState(), "historysize", &LuaDispatch<&LuaWrapper::BruiserLuaChangeHistorySize>);
    lua_register(LE.GetLuaState(), "showsource", &LuaDispatch<&LuaWrapper::BruiserLuaShowSourcecode>);
    lua_register(LE.GetLuaState(), "extractmutagen", &LuaDispatch<&LuaWrapper::BruiserLuaMutagenExtraction>);
    lua_register(LE.GetLuaState(), "strainrecognition", &LuaDispatch<&LuaWrapper::BruiserLuaStrainRecognition>);
    lua_register(LE.GetLuaState(), "setmakepath", &LuaDispatch<&LuaWrapper::BruiserLuaSetMakePath>);
    lua_register(LE.GetLuaState(), "run", &LuaDispatch<&LuaWrapper::BruiserLuaRun>);
    lua_register(LE.GetLuaState(), "setbinpath", &LuaDispatch<&LuaWrapper::BruiserLuaSetBinPath>);
    lua_register(LE.GetLuaState(), "getbinpath", &LuaDispatch<&LuaWrapper::BruiserLuaGetBinPath>);
    lua_register(LE.GetLuaState(), "getmakepath", &LuaDispatch<&LuaWrapper::BruiserLuaGetMakePath>);
    lua_register(LE.GetLuaState(), "getpaths", &LuaDispatch<&LuaWrapper::BruiserLuaGetPath>);
    lua_register(LE.GetLuaState(), "getsourcefiles", &LuaDispatch<&LuaWrapper::BruiserLuaGetSourceFiles>);
    lua_register(LE.GetLuaState(), "changedirectory", &LuaDispatch<&LuaWrapper::BruiserLuaChangeDirectory>);
    lua_register(LE.GetLuaState(), "yolo", &LuaDispatch<&LuaWrapper::BruiserLuaYolo>);
    lua_register(LE.GetLuaState(), "pwd", &LuaDispatch<&LuaWrapper::BruiserLuaPWD>);
    lua_register(LE.GetLuaState(), "objload", &LuaDispatch<&LuaWrapper::BruiserPyLoader>);
    lua_register(LE.GetLuaState(), "listObjects", &LuaDispatch<&LuaWrapper::BruiserLuaListObjects>);
    lua_register(LE.GetLuaState(), "xobjregister", &LuaDispatch<&LuaWrapper::BruiserLuaxobjRegister>);
    lua_register(LE.GetLuaState(), "xcall", &LuaDispatch<&LuaWrapper::BruiserLuaCallX>);
    lua_register(LE.GetLuaState(), "xobjlist", &LuaDispatch<&LuaWrapper::BruiserLuaXObjGetList>);
    lua_register(LE.GetLuaState(), "xallocglobal", &LuaDispatch<&LuaWrapper::BruiserLuaXObjAllocGlobal>);
    lua_register(LE.GetLuaState(), "xallocallglobals", &LuaDispatch<&LuaWrapper::BruiserLuaXObjAllocAllGlobals>);
    lua_register(LE.GetLuaState(), "getjmptable", &LuaDispatch<&LuaWrapper::BruiserGetJumpTable>);
    lua_register(LE.GetLuaState(), "freejmptable", &LuaDispatch<&LuaWrapper::BruiserFreeJumpTable>);
    lua_register(LE.GetLuaState(), "dumpjmptable", &LuaDispatch<&LuaWrapper::BruiserDumpJumpTable>);
    /*its just regisering the List function from LuaWrapper with X-macros.*/
#define X(__x1, __x2) lua_register(LE.GetLuaState(), #__x1, &LuaDispatch<&LuaWrapper::List##__x1>);

    LIST_LIST_GENERATORS

#undef X
#undef LIST_LIST_GENERATORS

    /*The non-cli execution loop*/
    if (NonCLILuaScript != "") {
      luaL_dofile(LE.GetLuaState(), NonCLILuaScript.c_str());
      dostring(LE.GetLuaState(), "os.exit()", "test");
      return 0;
    }

    /*cli execution loop*/
    while((command = linenoise(">>>")) != NULL) {
      linenoiseHistoryAdd(command);
      linenoiseHistorySave(SHELL_HISTORY_FILE);
      LE.RunChunk(command);
      linenoiseFree(command);
    }

    LE.Cleanup();

    /*end of bruiser main*/
    return 0;
  } //end of cli block

} //end of main
/**********************************************************************************************************************/
/*last line intentionally left blank.*/

