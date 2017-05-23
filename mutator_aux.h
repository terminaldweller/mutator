
/***************************************************Project Mutator****************************************************/
//-*-c++-*-
/*********************************************************************************************************************/
/*first line intentionally left blank*/
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
/*inclusion guard*/
#ifndef MUTATOR_AUX_H
#define MUTATOR_AUX_H
/*********************************************************************************************************************/
/*inclusion directives*/
#include <string>
#include <fstream>
#include "clang/AST/AST.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Rewrite/Core/Rewriter.h"
/*********************************************************************************************************************/
/*Macros and definitions*/
#define CheckSLValidity(SL) \
  do {\
  if (!SL.isValid()) {return void();}}\
  while(0);
/*********************************************************************************************************************/
using namespace clang;
/*********************************************************************************************************************/
namespace Devi {
enum class NodeKind {NoValue, VarDecl, FieldDecl, RecordDecl, LabelDecl, FunctionDecl, TypedefDecl, ParmVarDecl, EnumDecl, EnumConstDecl};

enum class Scope {NoValue, TU, Block};

enum class FunctionDeclKind {NoValue, Definition, Declaration};
/*********************************************************************************************************************/
SourceLocation SourceLocationHasMacro(SourceLocation SL, Rewriter &Rewrite, std::string Kind);

SourceLocation SourceLocationHasMacro(SourceLocation __sl, Rewriter &__rewrite);
/*********************************************************************************************************************/
bool IsTheMatchInSysHeader(bool SysHeaderFlag, const ast_matchers::MatchFinder::MatchResult &MR, SourceLocation SL);

bool IsTheMatchInSysHeader(bool SysHeaderFlag, const SourceManager &SM, SourceLocation SL);

bool IsTheMatchInSysHeader(bool SysHeaderFlag, bool SysHeader, SourceLocation SL);

bool IsTheMatchInSysHeader(bool SysHeaderFlag, bool SysHeader);
/*********************************************************************************************************************/
bool IsTheMatchInMainFile(bool MainFileFlag, const ast_matchers::MatchFinder::MatchResult &MR, SourceLocation SL);

bool IsTheMatchInMainFile(bool MainFileFlag, const SourceManager &SM, SourceLocation SL);

bool IsTheMatchInMainFile(bool MainFileFlag, bool MainFile, SourceLocation SL);

bool IsTheMatchInMainFile(bool MainFileFlag, bool MainFile);
/*********************************************************************************************************************/
/*end of namespace Devi*/
}
#endif
/*********************************************************************************************************************/
/*last line intentionally left blank.*/

