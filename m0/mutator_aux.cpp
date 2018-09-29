
/***************************************************Project Mutator****************************************************/
//-*-c++-*-
/*first line intentionally left blank.*/
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
/*********************************************************************************************************************/
/*inclusion directives*/
#include "mutator_aux.h"
#include <algorithm>
#include <string>
#include <cassert>
#include <iostream>
#include <fstream>
#include <iterator>
#include "clang/AST/AST.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "../tinyxml2/tinyxml2.h"
#include "../json/json.hpp"
/*********************************************************************************************************************/
using namespace clang;
/*********************************************************************************************************************/
namespace Devi {
/*a simple function that checks the sourcelocations for a macro expansion. returns the sourcelocation without
macro expansion address.*/
#if 1
SourceLocation SourceLocationHasMacro [[deprecated("doesnt work")]] (SourceLocation SL, Rewriter &Rewrite, std::string Kind) {
  /*does the sourcelocation include a macro expansion?*/
  if ( SL.isMacroID()) {
    /*get the expansion range which is startloc and endloc*/
#if __clang_major__ <= 6
    std::pair <SourceLocation, SourceLocation> expansionRange = Rewrite.getSourceMgr().getImmediateExpansionRange(SL);
#elif __clang_major__ == 8
    CharSourceRange expansionRange = Rewrite.getSourceMgr().getImmediateExpansionRange(SL);
#endif
    if (Kind == "start") {
#if __clang_major__ <= 6
      return expansionRange.first;
#elif __clang_major__ >= 8
      return expansionRange.getBegin();
#endif
    } else if (Kind == "end") {
#if __clang_major__ <= 6
      return expansionRange.second;
#elif __clang_major__ >= 8
      return expansionRange.getEnd();
#endif
    } else {
      std::cout << "the third argument of Devi::SourceLocationHasMacro is invalid." << std::endl;
    }
  } else {
    return (SL);
  }
  return (SL);
}
#endif

SourceLocation SourceLocationHasMacro(SourceLocation __sl, Rewriter &__rewrite) {
  if (__sl.isMacroID()) {
    return __rewrite.getSourceMgr().getSpellingLoc(__sl);
  } else {
    return __sl;
  }
}

SourceLocation getSLSpellingLoc(SourceLocation __sl, Rewriter &__rewrite) {
  if (__sl.isMacroID()) return __rewrite.getSourceMgr().getSpellingLoc(__sl);
  else return __sl;
}
/*********************************************************************************************************************/
/*********************************************************************************************************************/
/*********************************************************************************************************************/
/*the first argument is the option SysHeader from the mutator-lvl0 cl.*/
bool IsTheMatchInSysHeader(bool SysHeaderFlag, const ast_matchers::MatchFinder::MatchResult &MR, SourceLocation SL) {
  ASTContext *const ASTC = MR.Context;
  const SourceManager &SM = ASTC->getSourceManager();

  if (SM.isInSystemHeader(SL) && !SysHeaderFlag) {
    return true;
  } else {
    return false;
  }
}

bool IsTheMatchInSysHeader(bool SysHeaderFlag, const SourceManager &SM, SourceLocation SL) {
  if (SM.isInSystemHeader(SL) && !SysHeaderFlag) {
    return true;
  } else {
    return false;
  }
}

bool IsTheMatchInSysHeader(bool SysHeaderFlag, bool SysHeader, SourceLocation SL) {
  if (SysHeader && !SysHeaderFlag) {
    return true;
  } else {
    return false;
  }
}

bool IsTheMatchInSysHeader(bool SysHeaderFlag, bool SysHeader)
{
  if (SysHeader && !SysHeaderFlag) {
    return true;
  } else {
    return false;
  }
}
/*********************************************************************************************************************/
/*********************************************************************************************************************/
/*********************************************************************************************************************/
bool IsTheMatchInMainFile(bool MainFileFlag, const ast_matchers::MatchFinder::MatchResult &MR, SourceLocation SL) {
  ASTContext *const ASTC = MR.Context;
  const SourceManager &SM = ASTC->getSourceManager();
  if (SM.isInMainFile(SL) || (!SM.isInMainFile(SL) && !MainFileFlag)) {
    return true;
  } else {
    return false;
  }
}

bool IsTheMatchInMainFile(bool MainFileFlag, const SourceManager &SM, SourceLocation SL) {
  if (SM.isInMainFile(SL) || (!SM.isInMainFile(SL) && !MainFileFlag)) {
    return true;
  } else {
    return false;
  }
}

bool IsTheMatchInMainFile(bool MainFileFlag, bool MainFile, SourceLocation SL) {
  if (MainFile || (!MainFile && !MainFileFlag)) {
    return true;
  } else {
    return false;
  }
}

bool IsTheMatchInMainFile(bool MainFileFlag, bool MainFile) {
  if (MainFile || (!MainFile && !MainFileFlag)) {
    return true;
  } else {
    return false;
  }
}
/*********************************************************************************************************************/
std::vector<std::string> action_file_parser (int argc, const char** argv, std::string file_path) {
  int i = 1;
  std::vector<std::string> vs;
  std::ifstream action_file(file_path);
  for (std::string line; getline(action_file, line);) {
    size_t pos = line.find(":");
    if (line[0] == '#') {
      i++;
      continue;
    }
    if (pos == std::string::npos) {
      if (line[0] != '#') {
        if (strcmp(line.c_str(), "") != 0)std::cout << "m0 action file parser:error:malformed line at " << i << ".\n";//FIXME-RED
        //return 1;
      } else {
        i++;
        continue;
      }
    }
    std::string content = line.substr(pos + 1, std::string::npos);
    std::string option = line.substr(0, pos);
    if (option == "action_name") {}
    else if (option == "exec_opts") {vs.push_back(content);}
    else if (option == "in_files") {vs.push_back(content);}
    else if (option == "libtooling_options") {vs.push_back(content);}
    else if (option == "out_files") {vs.push_back(content);}
    else if (option == "log_files") {}
    else if (option == "print_pretty") {}
    else if (option == "end_action") {}
    else {/*std::cout << "m0 action file parser:error:bad option at line "<< i <<"\n"*/;}//FIXME-RED
    i++;
  }
  return vs;
}
/*********************************************************************************************************************/
/*End of namespace Devi*/
}
/*********************************************************************************************************************/
/*last line intentionally left blank.*/

