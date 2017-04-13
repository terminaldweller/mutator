
/***************************************************Project Mutator****************************************************/
//-*-c++-*-
/*first line intentionally left blank.*/
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
/*********************************************************************************************************************/
/*inclusion directives*/
#include "mutator_aux.h"
#include <string>
#include <cassert>
#include <iostream>
#include <fstream>
#include "clang/AST/AST.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "tinyxml2/tinyxml2.h"
#include "json/json.hpp"
/*********************************************************************************************************************/
using namespace clang;
/*********************************************************************************************************************/
namespace Devi {
/*a simple function that checks the sourcelocations for a macro expansion. returns the sourcelocation without
macro expansion address.*/
SourceLocation SourceLocationHasMacro [[deprecated("doesnt work")]] (SourceLocation SL, Rewriter &Rewrite, std::string Kind)
{
  /*does the sourcelocation include a macro expansion?*/
  if ( SL.isMacroID() )
  {
    /*get the expansion range which is startloc and endloc*/
    std::pair <SourceLocation, SourceLocation> expansionRange = Rewrite.getSourceMgr().getImmediateExpansionRange(SL);

    if (Kind == "start")
    {
      return (expansionRange.first);
    }
    else if (Kind == "end")
    {
      return (expansionRange.second);
    }
    else
    {
      std::cout << "the third argument of Devi::SourceLocationHasMacro is invalid." << std::endl;
    }

  }
  else
  {
    return (SL);
  }

  return (SL);
}

SourceLocation SourceLocationHasMacro(SourceLocation __sl, Rewriter &__rewrite)
{
  if (__sl.isMacroID())
  {
    return __rewrite.getSourceMgr().getSpellingLoc(__sl);
  }
  else
  {
    return __sl;
  }
}
/*********************************************************************************************************************/
/*********************************************************************************************************************/
/*********************************************************************************************************************/
/*the first argument is the option SysHeader from the mutator-lvl0 cl.*/
bool IsTheMatchInSysHeader(bool SysHeaderFlag, const ast_matchers::MatchFinder::MatchResult &MR, SourceLocation SL)
{
  ASTContext *const ASTC = MR.Context;
  const SourceManager &SM = ASTC->getSourceManager();

  if (SM.isInSystemHeader(SL) && !SysHeaderFlag)
  {
    return true;
  }
  else
  {
    return false;
  }
}

bool IsTheMatchInSysHeader(bool SysHeaderFlag, const SourceManager &SM, SourceLocation SL)
{
  if (SM.isInSystemHeader(SL) && !SysHeaderFlag)
  {
    return true;
  }
  else
  {
    return false;
  }
}

bool IsTheMatchInSysHeader(bool SysHeaderFlag, bool SysHeader, SourceLocation SL)
{
  if (SysHeader && !SysHeaderFlag)
  {
    return true;
  }
  else
  {
    return false;
  }
}

bool IsTheMatchInSysHeader(bool SysHeaderFlag, bool SysHeader)
{
  if (SysHeader && !SysHeaderFlag)
  {
    return true;
  }
  else
  {
    return false;
  }
}
/*********************************************************************************************************************/
/*********************************************************************************************************************/
/*********************************************************************************************************************/
bool IsTheMatchInMainFile(bool MainFileFlag, const ast_matchers::MatchFinder::MatchResult &MR, SourceLocation SL)
{
  ASTContext *const ASTC = MR.Context;
  const SourceManager &SM = ASTC->getSourceManager();

  if (SM.isInMainFile(SL) || (!SM.isInMainFile(SL) && !MainFileFlag))
  {
    return true;
  }
  else
  {
    return false;
  }
}

bool IsTheMatchInMainFile(bool MainFileFlag, const SourceManager &SM, SourceLocation SL)
{
  if (SM.isInMainFile(SL) || (!SM.isInMainFile(SL) && !MainFileFlag))
  {
    return true;
  }
  else
  {
    return false;
  }
}

bool IsTheMatchInMainFile(bool MainFileFlag, bool MainFile, SourceLocation SL)
{
  if (MainFile || (!MainFile && !MainFileFlag))
  {
    return true;
  }
  else
  {
    return false;
  }
}

bool IsTheMatchInMainFile(bool MainFileFlag, bool MainFile)
{
  if (MainFile || (!MainFile && !MainFileFlag))
  {
    return true;
  }
  else
  {
    return false;
  }
}
/*********************************************************************************************************************/
/*End of namespace Devi*/
}
/*********************************************************************************************************************/
/*last line intentionally left blank.*/

