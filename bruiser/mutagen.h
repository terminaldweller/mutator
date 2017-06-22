
/***************************************************Project Mutator****************************************************/
//-*-c++-*-
/*first line intentionally left blank.*/
/*the header for bruiser's mutation facilities.*/
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
/*code structure inspired by Eli Bendersky's tutorial on Rewriters.*/
/**********************************************************************************************************************/
/*inclusion guard*/
#ifndef MUTAGEN_H
#define MUTAGEN_H
/**********************************************************************************************************************/
/*included modules*/
/*project headers*/
/*standard library headers*/
#include <vector>
/*clang headers*/
/*llvm headers*/

/**********************************************************************************************************************/
/*using*/
/**********************************************************************************************************************/
namespace mutagen
{
/*enums*/
  /**
   * @brief Gene Kinds.
   */
  enum class DoomKind {UnidentifiedGene, 
    CStyleCastExpr, ExplicitCastExpr, ImplicitCastExpr, 
    Stmt, CompoundStmt, LoopStmt, ForStmt, WhileStmt, IfStmt, ElseStmt, DoStmt, SwitchStmt, CaseStmt,
    Decl, FunctionDecl, ClassDecl, VarDecl, RecordDecl, ArrayDecl, StructDecl, UnionDecl, VectorDecl,
    MapDecl, UnorderedMapDecl, MemberFunctionDecl, TemplateDecl, ClassTemplateDecl, ParmVarDecl
  };
/**********************************************************************************************************************/
class Amino
{
  public:

  private:
};
/**********************************************************************************************************************/
/**
 * @brief A container class for the bad genes
 */
class DoomedStrain
{
  public:
  DoomedStrain() {}
  ~DoomedStrain() {}

  private:
  std::vector<Amino> Aminos;
};
/**********************************************************************************************************************/
class Ancestry
{
  public:
    Ancestry() {}
    ~Ancestry();

  private:
    
};
/**********************************************************************************************************************/
} //end of namespace mutagen
/**********************************************************************************************************************/
#endif //end of mutagen header
/*last line intentionally left balnk*/

