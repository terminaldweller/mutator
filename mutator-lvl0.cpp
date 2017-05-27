
/***************************************************Project Mutator****************************************************/
//-*-c++-*-
/*first line intentionally left blank.*/
/*the source code for the static checks(Misra-C,...)*/
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
/*included modules*/
/*project headers*/
#include "mutator-lvl0.h"
#include "mutator_aux.h"
#include "mutator_report.h"
/*standard headers*/
#include <cassert>
#include <exception>
#include <fstream>
#include <iostream>
#include <map>
#include <regex>
#include <string>
#include <vector>
/*Clang headers*/
#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ASTTypeTraits.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Expr.h"
#include "clang/AST/Type.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/OperatorKinds.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Basic/IdentifierTable.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Lex/Lexer.h"
#include "clang/Lex/MacroArgs.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Lex/PPCallbacks.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Core/QualTypeNames.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Rewrite/Core/Rewriter.h"
/*LLVM headers*/
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/APInt.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Function.h"
/**********************************************************************************************************************/
/*used namespaces*/
using namespace llvm;
using namespace clang;
using namespace clang::ast_matchers;
using namespace clang::driver;
using namespace clang::tooling;
/**********************************************************************************************************************/
/*macros and defs*/

/*@DEVI-disbale debugs info printouts.*/
#define _MUT0_TEST
#if 1
#undef _MUT0_TEST
#endif

/*@DEVI-disbale all matchers.*/
#define _MUT0_EN_MATCHERS
#if 0
#undef _MUT0_EN_MATCHERS
#endif
/**********************************************************************************************************************/
/*global vars*/
Devi::XMLReport XMLDocOut;
Devi::JSONReport JSONDocOUT;
MutagenExtraction ME;

std::vector<SourceLocation> MacroDefSourceLocation;
std::vector<SourceLocation> MacroUndefSourceLocation;
std::vector<std::string> MacroNameString;
std::vector<std::string> IncludeFileArr;

/**********************************************************************************************************************/
struct MutExHeaderNotFound : public std::exception
{
public:
  MutExHeaderNotFound(std::string FileName) : FName(FileName) {}

  const char* what () const throw()
  {
    return "Header Not Found";
  }

  std::string getFileName() const
  {
    return FName;
  }

private:
  std::string FName;
};
/**********************************************************************************************************************/
/*@DEVI-struct for nullstmt*/
struct NullStmtInfo
{
  NullStmtInfo (unsigned iColumn, unsigned iLine, std::string iFileName, bool iIsInMainFile, bool iIsInSysHeader)
  {
    Column = iColumn;
    Line = iLine;
    FileName = iFileName;
    IsInMainFile = iIsInMainFile;
    IsInSysHeader = iIsInSysHeader;
  }

  unsigned Column;
  unsigned Line;
  std::string FileName;
  bool IsInMainFile;
  bool IsInSysHeader;
};

std::vector<NullStmtInfo> NullStmtProto;
/**********************************************************************************************************************/
/*@DEVI-struct used for 8.8*/
struct ExternObjInfo
{
  ExternObjInfo(unsigned int iLineNumber, unsigned int iColumnNumber, std::string iFileName\
                , std::string iXObjSLStr, std::string iXObjNameStr, FileID iXObjFID \
                , bool iHasMoreThanOneDefinition, bool iIsDefinition, bool iIsDeclaration)
  {
    LineNumber = iLineNumber;
    ColumnNumber = iColumnNumber;
    FileName = iFileName;
    XObjSLStr = iXObjSLStr;
    XObjNameStr = iXObjNameStr;
    XObjFID = iXObjFID;
    HasMoreThanOneDefinition = iHasMoreThanOneDefinition;
    IsDefinition = iIsDefinition;
    IsDeclaration = iIsDeclaration;
  }

  unsigned int LineNumber;
  unsigned int ColumnNumber;
  std::string FileName;
  std::string XObjSLStr;
  std::string XObjNameStr;
  FileID XObjFID;
  bool HasMoreThanOneDefinition;
  bool IsDefinition;
  bool IsDeclaration;
};

std::vector<ExternObjInfo> ExternObjInfoProto;
/*@DEVI-end*/
/**********************************************************************************************************************/
/*@DEVI-struct used for rules 5.x*/
struct IdentInfo
{
  IdentInfo(unsigned int iLine, unsigned int iColumn, std::string iFileName, std::string iName, \
            std::string iSLString, Devi::NodeKind iNK, bool iIsIncomplete, Devi::FunctionDeclKind IFDKind, \
            Devi::Scope iScope, std::string iScopeFuncitonName, bool iIsValid, bool iIsStatic)
  {
    Line = iLine;
    Column = iColumn;
    FileName = iFileName;
    Name = iName;
    SLString = iSLString;
    NK = iNK;
    IsIncomplete = iIsIncomplete;
    FDKind = IFDKind;
    Scope = iScope;
    ScopeFunctionName = iScopeFuncitonName;
    IsValid = iIsValid;
    IsStatic = iIsStatic;
  }

  unsigned int Line;
  unsigned int Column;
  std::string FileName;
  std::string Name;
  std::string SLString;
  Devi::NodeKind NK;
  bool IsIncomplete;
  Devi::FunctionDeclKind FDKind;
  Devi::Scope Scope;
  std::string ScopeFunctionName;
  bool IsValid;
  bool IsStatic;
};

std::vector<IdentInfo> IdentInfoProto;

std::unordered_map<std::string, bool> umRuleList;
/*@DEVI-end*/
/**********************************************************************************************************************/
/*mutator-lvl0 executable options*/
enum MisraC
{
  NA=(0x1<<6), MisraC98=(0x1<<0), MisraC2004=(0x1<<2), MisraC2012=(0x1<<4), C1=(0x1<<1), C2=(0x1<<3), C3=(0x1<<5)
};

static llvm::cl::OptionCategory MutatorLVL0Cat("mutator-lvl0 options category");
/*@DEVI-the option has been added since gcc does it.its as simple as that.*/
cl::opt<bool> CheckSystemHeader("SysHeader", cl::desc("mutator-lvl0 will run through System Headers"), cl::init(false), cl::cat(MutatorLVL0Cat), cl::ZeroOrMore);
cl::opt<bool> MainFileOnly("MainOnly", cl::desc("mutator-lvl0 will only report the results that reside in the main file"), cl::init(false), cl::cat(MutatorLVL0Cat), cl::ZeroOrMore);
cl::opt<MisraC> MisraCVersion("MCV", cl::desc("choose the MisraC version to check against"), \
                              cl::values(clEnumVal(MisraC98, "Misrac-1998"), clEnumVal(MisraC2004, "Misra-C:2004"), clEnumVal(MisraC2012, "Misra-C:2012"), \
                                  clEnumVal(C1, "Misra-C:1998"), clEnumVal(C2, "Misra-C:2004"), clEnumVal(C3, "Misra-C:2012")), cl::init(MisraC2004), cl::cat(MutatorLVL0Cat), cl::Optional);
cl::opt<std::string> MCE("MCE", cl::desc("MisraC switches to enable specific rule checks"), cl::init("10.1 "), cl::cat(MutatorLVL0Cat), cl::Optional);
cl::opt<std::string> MCD("MCD", cl::desc("MisraC switches to disable specific rule checks"), cl::init(" 9.3"), cl::cat(MutatorLVL0Cat), cl::Optional);
cl::opt<bool> MCEA("MCEA", cl::desc("MisraC switch to enable all rule checks"), cl::init(true), cl::cat(MutatorLVL0Cat), cl::Optional);
cl::opt<bool> MCDA("MCDA", cl::desc("MisraC switches to disable all rule checks"), cl::init(false), cl::cat(MutatorLVL0Cat), cl::Optional);
cl::opt<bool> SFRCPP("SFRCPP", cl::desc("Enables SaferCPlusPlus rule checks"), cl::init(true), cl::cat(MutatorLVL0Cat), cl::Optional);
cl::opt<bool> mutagen("mutagen", cl::desc("runs mutagen after running the static tests"), cl::init(false), cl::cat(MutatorLVL0Cat), cl::Optional);
/**********************************************************************************************************************/
class StringOptionsParser
{
friend class MutatorLVL0Tests;

public:
  StringOptionsParser() {}

  bool MC2Parser(void)
  {
    if (MCDA)
    {
      PopulateRuleList(false);
    }
    else if (MCEA)
    {
      PopulateRuleList(true);
    }
    
    ParseString();

    UpdateRuleList();

    return true;
  }

  void Dump(bool InArg)
  {
    if (InArg)
    {
      for (auto &iter : umRuleList)
      {
        std::cout<< "Debug-umRuleList: " << "RLKey: " << iter.first << " " << "RLValue: " << iter.second << "\n";
      }

      std::cout << "\n";

      for (auto &iter : ParsedString)
      {
        std::cout << "Debug: " << "PSKey: " << iter.first << " " << "PSValue: " << iter.second << "\n";
      }
    }
  }

private:
  void PopulateRuleList(bool PopValue)
  {
    if (MisraCVersion < 0x4)
    {
      // C1 
      umRuleList.insert({"0", PopValue});
      
      typedef std::multimap<std::string,std::string>::const_iterator Iter;
      for (Iter iter = MC1EquivalencyMap.begin(), iterE = MC1EquivalencyMap.end(); iter != iterE; ++iter)
      {
        if (iter->first != std::prev(iter)->first)
        {
          umRuleList.insert({iter->first, PopValue});
        }
      }
    }

    if (MisraCVersion < 0x10)
    {
      // C2
      typedef std::map<std::string, bool>::const_iterator Iter;
      for (Iter iter = MC2OptsMap.begin(), iterE = MC2OptsMap.end(); iter != iterE; ++iter)
      {
        umRuleList.insert({iter->first, PopValue});
      }

    }

    if (MisraCVersion < 0x40)
    {
      // C3
    }
  }
 
  void ParseString(void)
  {
#if 0
    std::cout << "MCD:" << MCD << "\n";
    std::cout << "MCE:" << MCE << "\n";
#endif

    bool Disenable;
    std::string TempString;

    if (MCDA)
    {
      Disenable = true;
      TempString = MCE;
    }
    else if (MCEA)
    {
      Disenable = false;
      TempString = MCD;
    }
    
    size_t WhiteSpacePos = TempString.find(" ", 0U);
    size_t OldPosition = 0U;

    if (WhiteSpacePos == std::string::npos)
    {
      ParsedString.push_back(std::make_pair(TempString, false));

      return void();
    }

    while(WhiteSpacePos != std::string::npos)
    {
      OldPosition = WhiteSpacePos;
      WhiteSpacePos = TempString.find(" ", WhiteSpacePos + 1U);

      if (WhiteSpacePos != std::string::npos)
      {
        ParsedString.push_back(std::make_pair(TempString.substr(OldPosition + 1U, WhiteSpacePos - OldPosition - 1U), Disenable));
      }
    }
  }

  void UpdateRuleList(void)
  {
    for (auto &iter : umRuleList)
    {
      for (auto &yaiter : ParsedString)
      {
        if (iter.first == yaiter.first)
        {
          iter.second = yaiter.second;
          break;
        }
      }
    }
  }

  std::vector<std::pair<std::string, bool>> ParsedString;

  std::vector<std::pair<std::string, bool>> RuleList [[deprecated("now using umRuleList")]];
};
/**********************************************************************************************************************/
/**************************************************ASTMatcher Callbacks************************************************/
class [[deprecated("replaced by a more efficient class"), maybe_unused]] MCForCmpless : public MatchFinder::MatchCallback {
public:
  MCForCmpless (Rewriter &Rewrite) : Rewrite (Rewrite) {}

  virtual void run(const MatchFinder::MatchResult &MR)
  {
    if (MR.Nodes.getNodeAs<clang::ForStmt>("mcfor") != nullptr)
    {
      const ForStmt *FS = MR.Nodes.getNodeAs<clang::ForStmt>("mcfor");

      SourceLocation SL = FS->getLocStart();
      CheckSLValidity(SL);
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

#if 0
      std::cout << "14.8 : " << "\"For\" statement has no braces {}: " << "\n";
      std::cout << SL.printToString(*MR.SourceManager) << "\n" << "\n";
#endif
    }
    else
    {
      std::cout << "matcher -mcfor- returned nullptr." << "\n";
    }
  }

private:
  Rewriter &Rewrite;
};
/**********************************************************************************************************************/
class [[deprecated("replaced by a more efficient class"), maybe_unused]] MCWhileCmpless : public MatchFinder::MatchCallback {
public:
  MCWhileCmpless (Rewriter &Rewrite) : Rewrite (Rewrite) {}

  virtual void run(const MatchFinder::MatchResult &MR)
  {
    if (MR.Nodes.getNodeAs<clang::WhileStmt>("mcwhile") != nullptr)
    {
      const WhileStmt *WS = MR.Nodes.getNodeAs<clang::WhileStmt>("mcwhile");

      SourceLocation SL = WS->getLocStart();
      CheckSLValidity(SL);
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

#if 0
      std::cout << "14.8 : " << "\"While\" statement has no braces {}: " << "\n";
      std::cout << SL.printToString(*MR.SourceManager) << "\n" << "\n";
#endif
    }
    else
    {
      std::cout << "matcher -mcwhile- returned nullptr." << "\n";
    }
  }

private:
  Rewriter &Rewrite;
};
/**********************************************************************************************************************/
class MCElseCmpless : public MatchFinder::MatchCallback {
public:
  MCElseCmpless (Rewriter &Rewrite) : Rewrite (Rewrite) {}

  virtual void run(const MatchFinder::MatchResult &MR)
  {
    if (MR.Nodes.getNodeAs<clang::IfStmt>("mcelse") != nullptr)
    {
      const IfStmt *IS = MR.Nodes.getNodeAs<clang::IfStmt>("mcelse");

      if (mutagen)
      {
        ME.ExtractAncestry(ast_type_traits::DynTypedNode::create(*IS), *MR.Context);
      }

      SourceLocation SL = IS->getLocStart();
      CheckSLValidity(SL);
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

      if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
      {
        return void();
      }

      if (Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
      {
        std::cout << "14.9:" << "\"Else\" statement has no braces {}:";
        std::cout << SL.printToString(*MR.SourceManager) << ":" << "\n";

        XMLDocOut.XMLAddNode(MR.Context, SL, "14.9", "\"Else\" statement has no braces {}: ");
        JSONDocOUT.JSONAddElement(MR.Context, SL, "14.9", "\"Else\" statement has no braces {}: ");

        if (mutagen)
        {
          ME.ExtractAncestry(ast_type_traits::DynTypedNode::create(*IS), *MR.Context);
        }
      }
    }
    else
    {
      std::cout << "matcher -mcelse- returned nullptr." << "\n";
    }
  }

private:
  Rewriter &Rewrite;
};
/**********************************************************************************************************************/
class MCIfCmpless : public MatchFinder::MatchCallback {
public:
  MCIfCmpless (Rewriter &Rewrite) : Rewrite (Rewrite) {}

  virtual void run(const MatchFinder::MatchResult &MR)
  {
    if (MR.Nodes.getNodeAs<clang::IfStmt>("mcif") != nullptr)
    {
      const IfStmt *IS = MR.Nodes.getNodeAs<clang::IfStmt>("mcif");

      if (mutagen)
      {
        ME.ExtractAncestry(ast_type_traits::DynTypedNode::create(*IS), *MR.Context);
      }

      SourceLocation SL = IS->getLocStart();
      CheckSLValidity(SL);
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

      if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
      {
        return void();
      }

      if (Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
      {
        std::cout << "14.9:" << "\"If\" statement has no braces {}:";
        std::cout << SL.printToString(*MR.SourceManager) << ":" << "\n";

        XMLDocOut.XMLAddNode(MR.Context, SL, "14.9", "\"If\" statement has no braces {}: ");
        JSONDocOUT.JSONAddElement(MR.Context, SL, "14.9", "\"If\" statement has no braces {}: ");

        if (mutagen)
        {
          ME.ExtractAncestry(ast_type_traits::DynTypedNode::create(*IS), *MR.Context);
        }
      }
    }
    else
    {
      std::cout << "matcher -mcif- returned nullptr." << "\n";
    }
  }

private:
  Rewriter &Rewrite;
};
/**********************************************************************************************************************/
class IfElseMissingFixer : public MatchFinder::MatchCallback
{
public:
  IfElseMissingFixer (Rewriter &Rewrite) : Rewrite (Rewrite) {}

  virtual void run(const MatchFinder::MatchResult &MR)
  {
    if (MR.Nodes.getNodeAs<clang::IfStmt>("mcifelse") != nullptr)
    {
      const IfStmt *ElseIf = MR.Nodes.getNodeAs<clang::IfStmt>("mcifelse");

      SourceLocation IFESL = ElseIf->getLocStart();
      CheckSLValidity(IFESL);
      IFESL = Devi::SourceLocationHasMacro(IFESL, Rewrite, "start");
      
      if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, IFESL))
      {
        return void();
      }

      if (Devi::IsTheMatchInMainFile(MainFileOnly, MR, IFESL))
      {
        std::cout << "14.10:" << "\"If-Else If\" statement has no ending Else:";
        std::cout << IFESL.printToString(*MR.SourceManager) << ":" << "\n";

        XMLDocOut.XMLAddNode(MR.Context, IFESL, "14.10", "\"If-Else If\" statement has no ending Else: ");
        JSONDocOUT.JSONAddElement(MR.Context, IFESL, "14.10", "\"If-Else If\" statement has no ending Else: ");

        if (mutagen)
        {
          ME.ExtractAncestry(ast_type_traits::DynTypedNode::create(*ElseIf), *MR.Context);
        }
      }
    }
    else
    {
      std::cout << "matcher -mcifelse- returned nullptr." << "\n";
    }
  }


private:
  Rewriter &Rewrite;
};
/**********************************************************************************************************************/
class MCSwitchBrkless : public MatchFinder::MatchCallback
{
public:
  MCSwitchBrkless (Rewriter &Rewrite) : Rewrite (Rewrite) {}

  virtual void run(const MatchFinder::MatchResult &MR)
  {
    if (MR.Nodes.getNodeAs<clang::SwitchStmt>("mcswitchbrk") != nullptr)
    {
      const SwitchStmt *SS = MR.Nodes.getNodeAs<clang::SwitchStmt>("mcswitchbrk");

      SourceLocation SL = SS->getLocStart();
      CheckSLValidity(SL);
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

      if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
      {
        return void();
      }

      if (Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
      {
        std::cout << "15.2:" << "\"SwitchStmt\" has a caseStmt that's missing a breakStmt:";
        std::cout << SL.printToString(*MR.SourceManager) << ":" << "\n";

        XMLDocOut.XMLAddNode(MR.Context, SL, "15.2", "\"SwitchStmt\" has a caseStmt that's missing a breakStmt: ");
        JSONDocOUT.JSONAddElement(MR.Context, SL, "15.2", "\"SwitchStmt\" has a caseStmt that's missing a breakStmt: ");

        if (mutagen)
        {
          ME.ExtractAncestry(ast_type_traits::DynTypedNode::create(*SS), *MR.Context);
        }
      }
    }
    else
    {
      std::cout << "matcher -mcswitchbrk- returned nullptr." << "\n";
    }
  }

private:
  Rewriter &Rewrite;
};
/**********************************************************************************************************************/
class MCSwitchDftLess : public MatchFinder::MatchCallback
{
public:
  MCSwitchDftLess (Rewriter &Rewrite) : Rewrite (Rewrite) {}

  virtual void run(const MatchFinder::MatchResult &MR)
  {
    if (MR.Nodes.getNodeAs<clang::SwitchStmt>("mcswitchdft") != nullptr)
    {
      const SwitchStmt *SS = MR.Nodes.getNodeAs<clang::SwitchStmt>("mcswitchdft");

      SourceLocation SL = SS->getLocStart();
      CheckSLValidity(SL);
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

      if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
      {
        return void();
      }

      if (Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
      {
        std::cout << "15.3:" << "\"SwitchStmt\" does not have a defaultStmt:";
        std::cout << SL.printToString(*MR.SourceManager) << ":" << "\n";

        XMLDocOut.XMLAddNode(MR.Context, SL, "15.3", "\"SwitchStmt\" does not have a defaultStmt: ");
        JSONDocOUT.JSONAddElement(MR.Context, SL, "15.3", "\"SwitchStmt\" does not have a defaultStmt: ");

        if (mutagen)
        {
          ME.ExtractAncestry(ast_type_traits::DynTypedNode::create(*SS), *MR.Context);
        }
      }
    }
    else
    {
      std::cout << "matcher -mcswitchdft- returned nullptr." << "\n";
    }
  }

private:
  Rewriter &Rewrite;
};
/**********************************************************************************************************************/
/*misra-c 2004:15.1*/
class MCSwitch151 : public MatchFinder::MatchCallback
{
public:
  MCSwitch151 (Rewriter &Rewrite) : Rewrite (Rewrite) {}

  virtual void run(const MatchFinder::MatchResult &MR)
  {
    if (MR.Nodes.getNodeAs<clang::CompoundStmt>("mccmp151") != nullptr && MR.Nodes.getNodeAs<clang::CaseStmt>("mccase151") != nullptr)
    {
      const CompoundStmt *CS = MR.Nodes.getNodeAs<clang::CompoundStmt>("mccmp151");
      const CaseStmt *SS = MR.Nodes.getNodeAs<clang::CaseStmt>("mccase151");

      SourceLocation SL = SS->getLocStart();
      CheckSLValidity(SL);
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

      if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
      {
        return void();
      }

      ASTContext *const ASTC = MR.Context;

      ASTContext::DynTypedNodeList NodeList = ASTC->getParents(*CS);

      /*@DEVI-assumptions:nothing has more than one parent in C.*/
      ast_type_traits::DynTypedNode ParentNode = NodeList[0];

      ast_type_traits::ASTNodeKind ParentNodeKind = ParentNode.getNodeKind();

      std::string StringKind = ParentNodeKind.asStringRef().str();

      if (StringKind != "SwitchStmt")
      {
        if (Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
        {
          std::cout << "15.1:" << "\"CaseStmt\" has a CompoundStmt ancestor that is not the child of the SwitchStmt:";
          std::cout << SL.printToString(*MR.SourceManager) << ":" << "\n";

          XMLDocOut.XMLAddNode(MR.Context, SL, "15.1", "\"CaseStmt\" has a CompoundStmt ancestor that is not the child of the SwitchStmt: ");
          JSONDocOUT.JSONAddElement(MR.Context, SL, "15.1", "\"CaseStmt\" has a CompoundStmt ancestor that is not the child of the SwitchStmt: ");

          if (mutagen)
          {
            ME.ExtractAncestry(ast_type_traits::DynTypedNode::create(*CS), *MR.Context);
          }
        }
      }
    }
    else
    {
      std::cout << "matcher -mccmp151- or -mccase151- returned nullptr." << "\n";
    }
  }

private:
  Rewriter &Rewrite;
};
/**********************************************************************************************************************/
class MCSwitch155 : public MatchFinder::MatchCallback
{
public:
  MCSwitch155 (Rewriter &Rewrite) : Rewrite (Rewrite) {}

  virtual void run(const MatchFinder::MatchResult &MR)
  {
    if (MR.Nodes.getNodeAs<clang::SwitchStmt>("mcswitch155") != nullptr)
    {
      const SwitchStmt *SS = MR.Nodes.getNodeAs<clang::SwitchStmt>("mcswitch155");

      SourceLocation SL = SS->getLocStart();
      CheckSLValidity(SL);
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

      if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
      {
        return void();
      }

      if (Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
      {
        std::cout << "15.5:" << "\"SwitchStmt\" does not have a CaseStmt:";
        std::cout << SL.printToString(*MR.SourceManager) << ":" << "\n";

        XMLDocOut.XMLAddNode(MR.Context, SL, "15.5", "\"SwitchStmt\" does not have a CaseStmt: ");
        JSONDocOUT.JSONAddElement(MR.Context, SL, "15.5", "\"SwitchStmt\" does not have a CaseStmt: ");

        if (mutagen)
        {
          ME.ExtractAncestry(ast_type_traits::DynTypedNode::create(*SS), *MR.Context);
        }
      }
    }
    else
    {
      std::cout << "matcher -mcswitch155- returned nullptr." << "\n";
    }
  }

private:
  Rewriter &Rewrite;
};
/**********************************************************************************************************************/
class MCFunction161 : public MatchFinder::MatchCallback
{
public:
  MCFunction161 (Rewriter &Rewrite) : Rewrite (Rewrite) {}

  virtual void run(const MatchFinder::MatchResult &MR)
  {
    if (MR.Nodes.getNodeAs<clang::FunctionDecl>("mcfunction161") != nullptr)
    {
      const FunctionDecl *FD = MR.Nodes.getNodeAs<clang::FunctionDecl>("mcfunction161");

      if (FD->isVariadic())
      {
        SourceLocation SL = FD->getLocStart(); 
        CheckSLValidity(SL);
        SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

        if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
        {
          return void();
        }

        if (Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
        {
          std::cout << "16.1:" << "\"FunctionDecl\" is variadic:";
          std::cout << SL.printToString(*MR.SourceManager) << ":" << "\n";

          XMLDocOut.XMLAddNode(MR.Context, SL, "16.1", "\"FunctionDecl\" is variadic: ");
          JSONDocOUT.JSONAddElement(MR.Context, SL, "16.1", "\"FunctionDecl\" is variadic: ");

          if (mutagen)
          {
            ME.ExtractAncestry(ast_type_traits::DynTypedNode::create(*FD), *MR.Context);
          }
        }
      }
    }
    else
    {
      std::cout << "matcher -mcfunction161- returned nullptr." << "\n";
    }
  }

private:
  Rewriter &Rewrite;
};
/**********************************************************************************************************************/
class MCFunction162 : public MatchFinder::MatchCallback
{
public:
  MCFunction162 (Rewriter &Rewrite) : Rewrite (Rewrite) {}

  virtual void run(const MatchFinder::MatchResult &MR)
  {
    if (MR.Nodes.getNodeAs<clang::CallExpr>("mc162callexpr") != nullptr && MR.Nodes.getNodeAs<clang::FunctionDecl>("mc162funcdec") != nullptr)
    {
      const FunctionDecl *FD = MR.Nodes.getNodeAs<clang::FunctionDecl>("mc162funcdec");
      const CallExpr *CE = MR.Nodes.getNodeAs<clang::CallExpr>("mc162callexpr");

      SourceLocation SL = FD->getLocStart(); 
      CheckSLValidity(SL);
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

      if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
      {
        return void();
      }

      if (Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
      {
        /*intentionally left blank*/
      }
      else
      {
        return void();
      }


      std::string FuncNameStr = FD->getNameInfo().getAsString();

      if (CE->getDirectCallee())
      {
        const FunctionDecl *FDCalled = CE->getDirectCallee();
        std::string CalledFuncNameStr = FDCalled->getNameInfo().getAsString();

        if (FuncNameStr == CalledFuncNameStr)
        {
          std::cout << "16.2:" << "\"FunctionDecl\" is recursive:";
          std::cout << SL.printToString(*MR.SourceManager) << ":" << "\n";

          XMLDocOut.XMLAddNode(MR.Context, SL, "16.2", "\"FunctionDecl\" is recursive: ");
          JSONDocOUT.JSONAddElement(MR.Context, SL, "16.2", "\"FunctionDecl\" is recursive: ");

          if (mutagen)
          {
            ME.ExtractAncestry(ast_type_traits::DynTypedNode::create(*CE), *MR.Context);
          }
        }
        else
        {
          /*intentionally left blank.*/
        }
      }
      else
      {
        /*intentionally left blank.*/
      }
    }
    else
    {
      std::cout << "matcher -mc162funcdec- and/or -mc162callexpr- returned nullptr." << "\n";
    }
  }

private:
  Rewriter &Rewrite;
};
/**********************************************************************************************************************/
class MCFunction164 : public MatchFinder::MatchCallback
{
public:
  MCFunction164 (Rewriter &Rewrite) : Rewrite (Rewrite) {}

  virtual void run(const MatchFinder::MatchResult &MR)
  {
    if (MR.Nodes.getNodeAs<clang::FunctionDecl>("mcfunc164") != nullptr)
    {
      const FunctionDecl *FD = MR.Nodes.getNodeAs<clang::FunctionDecl>("mcfunc164");
      const FunctionDecl *FDcl = FD->getDefinition();

      /*to guard against function that have a declaration that is not a definition only.*/
      if (FDcl != nullptr)
      {
        SourceLocation SL = FD->getLocStart(); 
        CheckSLValidity(SL);
        SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

        if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
        {
          return void();
        }

        if (Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
        {
          /*intentionally left blank*/
        }
        else
        {
          return void();
        }

        SourceLocation SLDcl = FDcl->getLocStart();
        SLDcl = Devi::SourceLocationHasMacro(SLDcl, Rewrite, "start");

        ArrayRef<ParmVarDecl*> FDParmList = FD->parameters();

        ArrayRef<ParmVarDecl*> FDclParmList = FDcl->parameters();

        if ( FD->getNumParams() != FDcl->getNumParams())
        {
          std::cout << "numparam of functiondefinition and functionDecl dont match! : " << SL.printToString(*MR.SourceManager) << "\n" << "\n";
        }
        else
        {
          if (FD->getNumParams() != 0)
          {
            for (unsigned x = 0; x < FD->getNumParams(); ++x)
            {
              if (FDParmList[x]->getNameAsString() != FDclParmList[x]->getNameAsString())
              {
                std::cout << "16.4:" << "FunctionDecl parameter names are not the same as function definition parameter names:";
                std::cout << SL.printToString(*MR.SourceManager) << ":" << "\n";

                XMLDocOut.XMLAddNode(MR.Context, SL, "16.4", "FunctionDecl parameter names are not the same as function definition parameter names: ");
                JSONDocOUT.JSONAddElement(MR.Context, SL, "16.4", "FunctionDecl parameter names are not the same as function definition parameter names: ");

                break;
              }
              else
              {
                /*intentionally left blank.*/
              }
            }
          }
        }
      }
    }
    else
    {
      std::cout << "matcher -mcfunc164- returned nullptr." << "\n";
    }
  }

private:
  Rewriter &Rewrite;
};
/**********************************************************************************************************************/
class MCFunction166 : public MatchFinder::MatchCallback
{
public:
  MCFunction166 (Rewriter &Rewrite) : Rewrite (Rewrite) {}

  virtual void run(const MatchFinder::MatchResult &MR)
  {
    if (MR.Nodes.getNodeAs<clang::CallExpr>("mcfunc166") != nullptr)
    {
      const CallExpr *CE = MR.Nodes.getNodeAs<clang::CallExpr>("mcfunc166");

      SourceLocation SL = CE->getLocStart(); 
      CheckSLValidity(SL);
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

      const FunctionDecl *FD = CE->getDirectCallee();

      DeclarationNameInfo DNI = FD->getNameInfo();

      std::string FuncNameString = DNI.getAsString();

      ASTContext *const ASTC = MR.Context;

      const SourceManager &SM = ASTC->getSourceManager();

      /*start of 20.4*/
      if ((FuncNameString == "malloc" || FuncNameString == "calloc" || FuncNameString == "free" || FuncNameString == "realloc") && SM.isInSystemHeader(FD->getLocStart()))
      {
        if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
        {
          /*intentionally left blank*/
        }
        else
        {
          if (Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
          {
            std::cout << "20.4:" << "Dynamic heap memory allocation used:";
            std::cout << SL.printToString(*MR.SourceManager) << ":" << "\n";

            XMLDocOut.XMLAddNode(MR.Context, SL, "20.4", "Dynamic heap memory allocation used: ");
            JSONDocOUT.JSONAddElement(MR.Context, SL, "20.4", "Dynamic heap memory allocation used: ");

          }
        }
      }
      /*end of 20.4*/

      /*start of 20.7*/
      if ((FuncNameString == "longjmp") && SM.isInSystemHeader(FD->getLocStart()))
      {
        if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
        {
          /*intentionally left blank*/
        }
        else
        {
          if (Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
          {
            std::cout << "20.7:" << "Use of lonjmp is illegal:";
            std::cout << SL.printToString(*MR.SourceManager) << ":" << "\n";

            XMLDocOut.XMLAddNode(MR.Context, SL, "20.7", "Use of longjmp is illegal: ");
            JSONDocOUT.JSONAddElement(MR.Context, SL, "20.7", "Use of longjmp is illegal: ");
          }
        }
      }
      /*end of 20.7*/

      /*start of 20.10*/
      if ((FuncNameString == "atof" || FuncNameString == "atoi" || FuncNameString == "atol") && SM.isInSystemHeader(FD->getLocStart()))
      {
        if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
        {
          /*intentionally left blank*/
        }
        else
        {
          if (Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
          {
            std::cout << "20.10:" << "Use of atof,atoi and atol is illegal:";
            std::cout << SL.printToString(*MR.SourceManager) << ":" << "\n";

            XMLDocOut.XMLAddNode(MR.Context, SL, "20.10", "Use of atof,atoi and atol is illegal: ");
            JSONDocOUT.JSONAddElement(MR.Context, SL, "20.10", "Use of atof,atoi and atol is illegal: ");
          }
        }
      }
      /*end of 20.10*/

      /*start of 20.11*/
      if ((FuncNameString == "abort" || FuncNameString == "exit" || FuncNameString == "getenv" || FuncNameString == "system") && SM.isInSystemHeader(FD->getLocStart()))
      {
        if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
        {
          /*intentionally left blank*/
        }
        else
        {
          if (Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
          {
            std::cout << "20.11:" << "Use of abort,exit,getenv and system is illegal:";
            std::cout << SL.printToString(*MR.SourceManager) << ":" << "\n";

            XMLDocOut.XMLAddNode(MR.Context, SL, "20.11", "Use of abort,exit,getenv and system is illegal : ");
            JSONDocOUT.JSONAddElement(MR.Context, SL, "20.11", "Use of abort,exit,getenv and system is illegal : ");
          }
        }
      }
      /*end of 20.11*/

      if (CE->getNumArgs() != FD->getNumParams())
      {
        if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
        {
          /*intentionally left blank*/
        }
        else
        {
          if (Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
          {
            std::cout << "16.6:" << "CallExpr number of arguments does not equal the number of parameters in the declaration:";
            std::cout << SL.printToString(*MR.SourceManager) << ":" << "\n";

            XMLDocOut.XMLAddNode(MR.Context, SL, "16.6", "CallExpr number of arguments does not equal the number of parameters in the declaration: ");
            JSONDocOUT.JSONAddElement(MR.Context, SL, "16.6", "CallExpr number of arguments does not equal the number of parameters in the declaration: ");
          }
        }
      }
    }
    else
    {
      std::cout << "matcher -mcfunc166- returned nullptr." << "\n";
    }
  }

private:
  Rewriter &Rewrite;
};
/**********************************************************************************************************************/
/*the clang parser does not allow for such constructs.*/
class [[maybe_unused]] MCFunction168 : public MatchFinder::MatchCallback
{
public:
  MCFunction168 (Rewriter &Rewrite) : Rewrite (Rewrite) {}

  virtual void run(const MatchFinder::MatchResult &MR)
  {
    if (MR.Nodes.getNodeAs<clang::ReturnStmt>("mcfunc168") != nullptr)
    {
      const ReturnStmt *RT = MR.Nodes.getNodeAs<clang::ReturnStmt>("mcfunc168");

      const Expr *RE [[maybe_unused]] = RT->getRetValue();

      SourceLocation SL = RT->getLocStart(); 
      CheckSLValidity(SL);
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

      if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
      {
        return void();
      }

      if (!Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
      {
        return void();
      }

      SourceLocation SLE = RT->getLocEnd();
      SLE = Devi::SourceLocationHasMacro(SLE, Rewrite, "end");

      SourceRange SR;
      SR.setBegin(SL);
      SR.setEnd(SLE);

      std::string RetType = Rewrite.getRewrittenText(SR);

#if 0
      std::cout << RetType << "\n" << "\n";
#endif
    }
    else
    {
      std::cout << "matcher -mcfunc168- returned nullptr." << "\n";
    }
  }

private:
  Rewriter &Rewrite;
};
/**********************************************************************************************************************/
class MCFunction169 : public MatchFinder::MatchCallback
{
public:
  MCFunction169 (Rewriter &Rewrite) : Rewrite (Rewrite) {}

  virtual void run(const MatchFinder::MatchResult &MR)
  {
    if (MR.Nodes.getNodeAs<clang::ImplicitCastExpr>("mcfunc169") != nullptr)
    {
      const ImplicitCastExpr* ICE = MR.Nodes.getNodeAs<clang::ImplicitCastExpr>("mcfunc169");

      SourceLocation SL = ICE->getLocStart(); 
      CheckSLValidity(SL);
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

      if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
      {
        return void();
      }

      if (!Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
      {
        return void();
      }

      CastKind CK = ICE->getCastKind();

      if (CK == CK_FunctionToPointerDecay)
      {
        std::cout << "16.9:" << "FunctionToPointerDecay:";
        std::cout << SL.printToString(*MR.SourceManager) << ":" << "\n";

        XMLDocOut.XMLAddNode(MR.Context, SL, "16.9", "FunctionToPointerDecay: ");
        JSONDocOUT.JSONAddElement(MR.Context, SL, "16.9", "FunctionToPointerDecay: ");

        if (mutagen)
        {
          ME.ExtractAncestry(ast_type_traits::DynTypedNode::create(*ICE), *MR.Context);
        }
      }
    }
  }

private:
  Rewriter &Rewrite;
};
/**********************************************************************************************************************/
/*@DEVI-what is correct: match a pointer then run matcher for implicitcastexpressions of type arraytopointerdecay
that have unary(--,++) and binary(-,+) operators as parents*/
class [[deprecated("replaced by something that actually works"), maybe_unused]] MCPA171 : public MatchFinder::MatchCallback
{
public:
  MCPA171 (Rewriter &Rewrite) : Rewrite(Rewrite) {}

  virtual void run(const MatchFinder::MatchResult &MR)
  {
    if (MR.Nodes.getNodeAs<clang::VarDecl>("mcpa171") != nullptr)
    {
      const VarDecl *VD = MR.Nodes.getNodeAs<clang::VarDecl>("mcpa171");

      QualType QT [[maybe_unused]] = VD->getType();

#if 0
      std::cout << QT.getAsString() << "\n" << "\n";
#endif
    }
  }

private:
  Rewriter &Rewrite [[maybe_unused]];
};
/**********************************************************************************************************************/
/*18.1 has false positives. incomplete types that have the same name as another incomplete
type in another scope are unrecognizable by this code.*/
class MCSU184 : public MatchFinder::MatchCallback
{
public:
  MCSU184 (Rewriter &Rewrite) : Rewrite(Rewrite)
  {
    /*@DEVI-these push-backs generate garbage entries*/
    UnionInfoProto.push_back(UnionInfo());

    StructInfoProto.push_back(StructInfo());

    StructCounter = 0U;
    UnionCounter = 0U;
  }

  virtual void run(const MatchFinder::MatchResult &MR)
  {
    if (MR.Nodes.getNodeAs<clang::RecordDecl>("mcsu184") != nullptr)
    {
      alreadymatched = false;

      const RecordDecl *RD = MR.Nodes.getNodeAs<clang::RecordDecl>("mcsu184");

      SourceLocation SL = RD->getLocStart(); 
      CheckSLValidity(SL);
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

      ASTContext* const ASTC = MR.Context;
      FullSourceLoc FSL = ASTC->getFullLoc(SL);

      if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
      {
        /*intentionally left blank*/
      }
      else
      {
        if (Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
        {
          std::cout << "18.4:" << "Union declared:";
          std::cout << SL.printToString(*MR.SourceManager) << ":" << "\n";

          XMLDocOut.XMLAddNode(MR.Context, SL, "18.4", "Union declared: ");
          JSONDocOUT.JSONAddElement(MR.Context, SL, "18.4", "Union declared: ");
        }
      }

      std::string MatchedName = RD->getNameAsString();

      for (unsigned x = 0; x < UnionCounter; ++x)
      {
        if (UnionInfoProto[x].UnionName == MatchedName)
        {
          alreadymatched = true;

          if (RD->isCompleteDefinition())
          {
            UnionInfoProto[x].IsIncompleteType = false;
          }
        }
      }

      if (alreadymatched == false)
      {
        UnionInfoProto.push_back(UnionInfo());
        UnionInfoProto[UnionCounter].UnionName = MatchedName;
        UnionInfoProto[UnionCounter].UnionSL = SL.printToString(*MR.SourceManager);
        UnionInfoProto[UnionCounter].FSL = FSL;
        UnionInfoProto[UnionCounter].SL = SL;

        if (RD->isCompleteDefinition())
        {
          /*this function has a declaration that is not a definition.*/
          UnionInfoProto[UnionCounter].IsIncompleteType = false;
        }

        UnionCounter++;
      }
    }

    if (MR.Nodes.getNodeAs<clang::RecordDecl>("mcsu181struct") != nullptr)
    {
      alreadymatched = false;

      const RecordDecl* RD = MR.Nodes.getNodeAs<clang::RecordDecl>("mcsu181struct");

      SourceLocation SL = RD->getLocStart(); 
      CheckSLValidity(SL);
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

      ASTContext* const ASTC = MR.Context;
      FullSourceLoc FSL = ASTC->getFullLoc(SL);

      std::string MatchedName = RD->getNameAsString();

      for (unsigned x = 0; x < StructCounter; ++x)
      {
        if (StructInfoProto[x].StructName == MatchedName)
        {
          alreadymatched = true;

          if (RD->isCompleteDefinition())
          {
            StructInfoProto[x].IsIncompleteType = false;
          }
        }
      }

      if (alreadymatched == false)
      {
        StructInfoProto.push_back(StructInfo());
        StructInfoProto[StructCounter].StructName = MatchedName;
        StructInfoProto[StructCounter].StructSL = SL.printToString(*MR.SourceManager);
        StructInfoProto[StructCounter].FSL = FSL;
        StructInfoProto[StructCounter].SL = SL;

        if (RD->isCompleteDefinition())
        {
          StructInfoProto[StructCounter].IsIncompleteType = false;
        }

        StructCounter++;
      }
    }
  }

  virtual void onEndOfTranslationUnit()
  {
    for (unsigned x = 0; x < StructCounter; ++x)
    {
      if (StructInfoProto[x].IsIncompleteType)
      {
        if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, StructInfoProto[x].FSL.isInSystemHeader(), StructInfoProto[x].SL))
        {
          /*intentionally left blank*/
        }
        else
        {
          if (Devi::IsTheMatchInMainFile(MainFileOnly, StructInfoProto[x].FSL.getManager().isInMainFile(StructInfoProto[x].SL), StructInfoProto[x].SL))
          {
            std::cout << "18.1:" << "Incomplete struct declared:";
            std::cout << StructInfoProto[x].StructSL << ":" << "\n";

            XMLDocOut.XMLAddNode(StructInfoProto[x].FSL, StructInfoProto[x].SL, "18.1", "Incomplete struct declared: ");
            JSONDocOUT.JSONAddElement(StructInfoProto[x].FSL, StructInfoProto[x].SL, "18.1", "Incomplete struct declared: ");
          }
        }
      }
    }

    for (unsigned x = 0; x < UnionCounter; ++x)
    {
      if (UnionInfoProto[x].IsIncompleteType)
      {
        if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, UnionInfoProto[x].FSL.isInSystemHeader(), UnionInfoProto[x].SL))
        {
          /*intentionally left blank*/
        }
        else
        {
          if (Devi::IsTheMatchInMainFile(MainFileOnly, UnionInfoProto[x].FSL.getManager().isInMainFile(UnionInfoProto[x].SL), UnionInfoProto[x].SL))
          {
            std::cout << "18.1:" << "Incomplete union declared:";
            std::cout << UnionInfoProto[x].UnionSL << ":" << "\n";

            XMLDocOut.XMLAddNode(UnionInfoProto[x].FSL, UnionInfoProto[x].SL, "18.1", "Incomplete union declared: ");
            JSONDocOUT.JSONAddElement(UnionInfoProto[x].FSL, UnionInfoProto[x].SL, "18.1", "Incomplete union declared: ");
          }
        }
      }
    }
  }


private:
  struct UnionInfo
  {
    std::string UnionSL;
    FullSourceLoc FSL;
    SourceLocation SL;
    std::string UnionName;
    bool IsIncompleteType = true;
  };

  unsigned int UnionCounter;

  std::vector<UnionInfo> UnionInfoProto;

  struct StructInfo
  {
    std::string StructSL;
    FullSourceLoc FSL;
    SourceLocation SL;
    std::string StructName;
    bool IsIncompleteType = true;
  };

  unsigned StructCounter;

  bool alreadymatched = false;

  std::vector<StructInfo> StructInfoProto;

  Rewriter &Rewrite;
};
/**********************************************************************************************************************/
class MCTypes6465 : public MatchFinder::MatchCallback
{
public:
  MCTypes6465 (Rewriter &Rewrite) : Rewrite(Rewrite) {}

  virtual void run(const MatchFinder::MatchResult &MR)
  {
    if (MR.Nodes.getNodeAs<clang::FieldDecl>("mctype6465") != nullptr)
    {
      const FieldDecl *FD = MR.Nodes.getNodeAs<clang::FieldDecl>("mctype6465");

      SourceLocation SL = FD->getLocStart(); 
      CheckSLValidity(SL);
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

      QualType QT = FD->getType();
      const clang::Type* TP = QT.getTypePtr();

      if ( !(TP->hasUnsignedIntegerRepresentation() || TP->hasSignedIntegerRepresentation()))
      {
        if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
        {
          /*intentionally left blank*/
        }
        else
        {
          if (Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
          {
            /*@DEVI-this part is ueless since the clang parser wont let such a bitfield through.*/
            std::cout << "6.4:" << "BitField has a type other than int or unsigned int:";
            std::cout << SL.printToString(*MR.SourceManager) << ":" << "\n";

            XMLDocOut.XMLAddNode(MR.Context, SL, "6.4", "BitField has a type other than int or unsigned int: ");
            JSONDocOUT.JSONAddElement(MR.Context, SL, "6.4", "BitField has a type other than int or unsigned int: ");

            if (mutagen)
            {
              ME.ExtractAncestry(ast_type_traits::DynTypedNode::create(*FD), *MR.Context);
            }
          }
        }
      }

      ASTContext *const ASTC = MR.Context;
      unsigned int BitWidth = FD->getBitWidthValue(*ASTC);

      if (TP->hasSignedIntegerRepresentation())
      {
        if (BitWidth < 2U)
        {
          if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
          {
            /*intentionally left blank*/
          }
          else
          {
            if (Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
            {
              std::cout << "6.5:" << "BitField of type signed integer has a length of less than 2 in bits:";
              std::cout << SL.printToString(*MR.SourceManager) << ":" << "\n";

              XMLDocOut.XMLAddNode(MR.Context, SL, "6.5", "BitField of type signed integer has a length of less than 2 in bits : ");
              JSONDocOUT.JSONAddElement(MR.Context, SL, "6.5", "BitField of type signed integer has a length of less than 2 in bits : ");

              if (mutagen)
              {
                ME.ExtractAncestry(ast_type_traits::DynTypedNode::create(*FD), *MR.Context);
              }
            }
          }
        }
      }
    }
  }

private:
  Rewriter &Rewrite;
};
/**********************************************************************************************************************/
class MCDCDF81 : public MatchFinder::MatchCallback
{
public:
  MCDCDF81 (Rewriter &Rewrite) : Rewrite(Rewrite)
  {
    /*@DEVI-the pushback generates garbage entries.*/
    FuncInfoProto.push_back(FuncInfo());

    VecC = 0U;
  };

  virtual void run(const MatchFinder::MatchResult &MR)
  {
    if (MR.Nodes.getNodeAs<clang::FunctionDecl>("mcdcdf81") != nullptr)
    {
      alreadymatched = false;

      const FunctionDecl* FD = MR.Nodes.getNodeAs<clang::FunctionDecl>("mcdcdf81");
      DeclarationNameInfo DNI = FD->getNameInfo();
      std::string MatchedName = DNI.getAsString();

      SourceLocation SL = FD->getLocStart(); 
      CheckSLValidity(SL);
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

      SourceLocation SLE = FD->getLocEnd();
      SLE = Devi::SourceLocationHasMacro(SLE, Rewrite, "start");

      ASTContext* const ASTC = MR.Context;
      const SourceManager &SM = ASTC->getSourceManager();
      FullSourceLoc FSL = ASTC->getFullLoc(SL);
      FullSourceLoc FSLE = ASTC->getFullLoc(SLE);

      /*start of 8.5*/
      bool FunctionDeclaredInsideHeader = false;

      if (FD->isThisDeclarationADefinition())
      {
        for (unsigned x = 0; x < IncludeFileArr.size(); ++x)
        {
          if (SM.getFilename(SL).str() == IncludeFileArr[x])
          {
            FunctionDeclaredInsideHeader = true;
          }
        }
      }

      if (FunctionDeclaredInsideHeader)
      {
        if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
        {
          /*intentionally left blank*/
        }
        else
        {
          if (Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
          {
            std::cout << "8.5:" << "Function definition inside a header file:";
            std::cout << SL.printToString(*MR.SourceManager) << ":" << "\n";

            XMLDocOut.XMLAddNode(MR.Context, SL, "8.5", "Function definition inside a header file : ");
            JSONDocOUT.JSONAddElement(MR.Context, SL, "8.5", "Function definition inside a header file : ");
          }
        }
      }
      /*end of 8.5*/


      /*start of checks for 19.5*/
      /*has false positives. false positives go away if the main.c is not included(main.c includes another header)*/
      if (FD->isThisDeclarationADefinition())
      {
        for (unsigned x = 0; x < MacroDefSourceLocation.size(); ++x)
        {
          if (FSL.isBeforeInTranslationUnitThan(MacroDefSourceLocation[x]) && \
              !FSLE.isBeforeInTranslationUnitThan(MacroDefSourceLocation[x]) && \
              SM.isInMainFile(MacroDefSourceLocation[x]) && !SM.isInSystemHeader(MacroDefSourceLocation[x]))
          {
            if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
            {
              /*intentionally left blank*/
            }
            else
            {
              if (Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
              {
                std::cout << "19.5:" << "Macro defined inside a block:";
#if 0
                std::cout << MacroDefSourceLocation[x].printToString(*MR.SourceManager) << " " << MacroNameString[x] << "\n" << "\n";
#endif
                std::cout << MacroDefSourceLocation[x].printToString(*MR.SourceManager) << ":" << "\n";

                XMLDocOut.XMLAddNode(MR.Context, MacroDefSourceLocation[x], "19.5", "Macro defined inside a block : ");
                JSONDocOUT.JSONAddElement(MR.Context, MacroDefSourceLocation[x], "19.5", "Macro defined inside a block : ");
              }
            }
          }
        }

        for (unsigned x = 0; x < MacroUndefSourceLocation.size(); ++x)
        {
          if (FSL.isBeforeInTranslationUnitThan(MacroUndefSourceLocation[x]) && \
              !FSLE.isBeforeInTranslationUnitThan(MacroUndefSourceLocation[x]) && \
              SM.isInMainFile(MacroUndefSourceLocation[x]) && !SM.isInSystemHeader(MacroUndefSourceLocation[x]))
          {
            if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
            {
              /*intentionally left blank*/
            }
            else
            {
              if (Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
              {
                std::cout << "19.5:" << "Macro undefined inside a block:";
                std::cout << MacroUndefSourceLocation[x].printToString(*MR.SourceManager) << ":" << "\n";

                XMLDocOut.XMLAddNode(MR.Context, MacroUndefSourceLocation[x], "19.5", "Macro undefined inside a block : ");
                JSONDocOUT.JSONAddElement(MR.Context, MacroUndefSourceLocation[x], "19.5", "Macro undefined inside a block : ");
              }
            }
          }
        }
      }
      /*end of checks for 19.5*/

      /*going through the already matched functions,making sure we are not adding duplicates.*/
      for (unsigned x = 0; x < VecC; ++x)
      {
        if (FuncInfoProto[x].FuncNameString == MatchedName)
        {
          alreadymatched = true;

          if (!FD->isThisDeclarationADefinition())
          {
            FuncInfoProto[x].HasDecThatisNotDef = true;
          }
        }
      }

      if (alreadymatched == false)
      {
        FuncInfoProto.push_back(FuncInfo());
        FuncInfoProto[VecC].FuncNameString = MatchedName;

        if (!FD->isThisDeclarationADefinition())
        {
          /*this function has a declaration that is not a definition.*/
          FuncInfoProto[VecC].HasDecThatisNotDef = true;
        }
        else
        {
          /*save the sourcelocation only if the functiondecl is a definition.*/
          FuncInfoProto[VecC].StrcSL = SL.printToString(*MR.SourceManager);
          FuncInfoProto[VecC].FuncSL = SL;
          FuncInfoProto[VecC].FuncFSL = MR.Context->getFullLoc(SL);
        }

        VecC++;
      }
    }
  }

  virtual void onEndOfTranslationUnit()
  {

    for (unsigned x = 0; x < VecC; ++x)
    {
      if (FuncInfoProto[x].HasDecThatisNotDef == false)
      {
        if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, FuncInfoProto[x].FuncFSL.isInSystemHeader(), FuncInfoProto[x].FuncSL))
        {
          /*intentionally left blank*/
        }
        else
        {
          if (Devi::IsTheMatchInMainFile(MainFileOnly, FuncInfoProto[x].FuncFSL.getManager().isInMainFile(FuncInfoProto[x].FuncSL), FuncInfoProto[x].FuncSL))
          {
            std::cout << "8.1:" << "Function does not have a FunctionDecl that is not a definition:";
            std::cout << FuncInfoProto[x].StrcSL << ":" << "\n";

            XMLDocOut.XMLAddNode(FuncInfoProto[x].FuncFSL, FuncInfoProto[x].FuncSL, "8.1", "Function does not have a FunctionDecl that is not a definition : ");
            JSONDocOUT.JSONAddElement(FuncInfoProto[x].FuncFSL, FuncInfoProto[x].FuncSL, "8.1", "Function does not have a FunctionDecl that is not a definition : ");
          }
        }
      }
    }

  }

private:
  struct FuncInfo {
    std::string FuncNameString;
    std::string StrcSL;
    bool HasDecThatisNotDef = false;
    SourceLocation FuncSL;
    FullSourceLoc FuncFSL;
  };

  std::vector<FuncInfo> FuncInfoProto;

  unsigned int VecC;

  bool alreadymatched = false;

  Rewriter &Rewrite;
};
/**********************************************************************************************************************/
/*Notes:clang does not let 8.2 and 8.3 through.*/
/*clang gives the implicitly-typed vardecl and functiondecl a default type in the AST so we cant use that.
we should just get the rewritten text and do string searches inside. thats the only way i can think of.*/
class [[maybe_unused]] MCDCDF82 : public MatchFinder::MatchCallback
{
public:
  MCDCDF82 (Rewriter &Rewrite) : Rewrite (Rewrite) {}

  virtual void run(const MatchFinder::MatchResult &MR)
  {
    if (MR.Nodes.getNodeAs<clang::VarDecl>("mcdcdf82") != nullptr)
    {
      const VarDecl* VD = MR.Nodes.getNodeAs<clang::VarDecl>("mcdcdf82");

      std::string QualifiedName = VD->getQualifiedNameAsString();

      QualType QT [[maybe_unused]] = VD->getType();

#if 0
      std::cout << QualifiedName << "\n" << "\n";
#endif
    }
  }

private:
  Rewriter &Rewrite [[maybe_unused]];
};
/**********************************************************************************************************************/
/*this class also matches aggregate types. a simple aggregate check should fix that, if need be.*/
class MCInit91 : public MatchFinder::MatchCallback
{
public:
  MCInit91 (Rewriter &Rewrite) : Rewrite(Rewrite) {}

  virtual void run(const MatchFinder::MatchResult &MR)
  {
    if (MR.Nodes.getNodeAs<clang::VarDecl>("mcinit91") != nullptr)
    {
      const VarDecl* VD = MR.Nodes.getNodeAs<clang::VarDecl>("mcinit91");

      SourceLocation SL = VD->getLocStart(); 
      CheckSLValidity(SL);
      SourceLocation SLMID;

      if (SL.isMacroID())
      {
        SLMID = Devi::SourceLocationHasMacro(SL, Rewrite, "start");
      }

      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

      SourceLocation SLE = VD->getLocEnd();
      SourceLocation SLEMID;

      if (SLE.isMacroID())
      {
        SLEMID = Devi::SourceLocationHasMacro(SLE, Rewrite, "end");
      }

      SLE = Devi::SourceLocationHasMacro(SLE, Rewrite, "end");

      QualType QT = VD->getType();

      const clang::Type* TP = QT.getTypePtr();

      ASTContext *const ASTC = MR.Context;

      SourceManager &SM = ASTC->getSourceManager();

      /*start of 8.5*/
      bool VarDeclaredInsideHeader = false;

      if (VD->isThisDeclarationADefinition(*ASTC) && !(!VD->isLocalVarDecl() && VD->isLocalVarDeclOrParm()))
      {
#if 0
        std::cout << "XXXXXXXXXXXXXXXXXXXXXXXX" << " " << IncludeFileArr.size() << "\n";
#endif
        for (unsigned x = 0; x < IncludeFileArr.size(); ++x)
        {
          if (SM.getFilename(SL).str() == IncludeFileArr[x])
          {
            VarDeclaredInsideHeader = true;
          }
        }
      }

      if (VarDeclaredInsideHeader)
      {
        if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
        {
          /*intentionally left blank*/
        }
        else
        {
          if (Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
          {
            std::cout << "8.5:" << "Variable definition inside a header file:";
            std::cout << SL.printToString(*MR.SourceManager) << ":" << "\n";

            XMLDocOut.XMLAddNode(MR.Context, SL, "8.5", "Variable definition inside a header file : ");
            JSONDocOUT.JSONAddElement(MR.Context, SL, "8.5", "Variable definition inside a header file : ");
          }
        }
      }
      /*end of 8.5*/

      /*start of 8.12*/
      if (!VD->hasInit())
      {
        if (VD->hasExternalStorage())
        {
          if (TP->isIncompleteArrayType())
          {
            if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
            {
              /*intentionally left blank*/
            }
            else
            {
              if (Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
              {
                /*end of 8.12*/
                std::cout << "8.12:" << "External array type is incomplete and has no initialization:";
                std::cout << SL.printToString(*MR.SourceManager) << ":" << "\n";

                XMLDocOut.XMLAddNode(MR.Context, SL, "8.12", "External array type is incomplete and has no initialization : ");
                JSONDocOUT.JSONAddElement(MR.Context, SL, "8.12", "External array type is incomplete and has no initialization : ");

                if (mutagen)
                {
                  ME.ExtractAncestry(ast_type_traits::DynTypedNode::create(*VD), *MR.Context);
                }
              }
            }
          }
        }
      }
      /*start of 9.2*/
      else
      {
        if (TP->isArrayType() || TP->isStructureType())
        {
          /*JANKY*/
          const Expr* InitExpr [[maybe_unused]] = VD->getInit();
          SourceRange InitExprSR;
          SourceLocation IESL = InitExpr->getLocStart(); 
          CheckSLValidity(IESL);
          IESL = Devi::SourceLocationHasMacro(IESL, Rewrite, "start");
      
          CheckSLValidity(IESL);

          SourceLocation IESLE = InitExpr->getLocEnd();
          IESLE = Devi::SourceLocationHasMacro(IESLE, Rewrite, "end");
          InitExprSR.setBegin(IESL);
          InitExprSR.setEnd(IESLE);

          std::string InitExprString = Rewrite.getRewrittenText(InitExprSR);
          size_t openingcbraces = InitExprString.find("{", 0);
          size_t closingcbraces = InitExprString.find("}", 0);

          if (openingcbraces == std::string::npos || closingcbraces == std::string::npos)
          {
            if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
            {
              /*intentionally left blank*/
            }
            else
            {
              if (Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
              {
#if 0
                std::cout << "9.2:" << "Curly braces not used:";
                std::cout << SL.printToString(*MR.SourceManager) << ":" << "\n";

                XMLDocOut.XMLAddNode(MR.Context, SL, "9.2", "Curly braces not used : ");
                JSONDocOUT.JSONAddElement(MR.Context, SL, "9.2", "Curly braces not used : ");
#endif
              }
            }
          }
        }
      }
      /*end of 9.2*/

      /*we only check for local static since global static is meaningless.*/
      if (!VD->isStaticLocal() && VD->isLocalVarDecl())
      {
        if (!VD->hasInit())
        {
          if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
          {
            /*intentionally left blank*/
          }
          else
          {
            if (Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
            {
              std::cout << "9.1:" << "staic local variable does not have initialization:";
              std::cout << SL.printToString(*MR.SourceManager) << ":" << "\n";

              XMLDocOut.XMLAddNode(MR.Context, SL, "9.1", "staic local variable does not have initialization : ");
              JSONDocOUT.JSONAddElement(MR.Context, SL, "9.1", "staic local variable does not have initialization : ");
            }
          }
        }
      }
    }

  }

private:
  Rewriter &Rewrite;
};
/**********************************************************************************************************************/
class [[maybe_unused]] MCInit92 : public MatchFinder::MatchCallback
{
public:
  MCInit92 (Rewriter &Rewrite) : Rewrite(Rewrite) {}

  virtual void run(const MatchFinder::MatchResult &MR)
  {
    if (MR.Nodes.getNodeAs<clang::InitListExpr>("mcinit92") != nullptr)
    {
      const InitListExpr* ILE = MR.Nodes.getNodeAs<clang::InitListExpr>("mcinit92");
      const VarDecl* VD = MR.Nodes.getNodeAs<clang::VarDecl>("mcinit92daddy");

      SourceLocation SL = VD->getLocStart(); 
      CheckSLValidity(SL);
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

      if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
      {
        return void();
      }

      if (!Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
      {
        return void();
      }

      unsigned int NumInits [[maybe_unused]] = ILE->getNumInits();

#if 0
      std::cout << NumInits << "\n" << "\n";
#endif
    }
  }

private:
  Rewriter &Rewrite;
};
/**********************************************************************************************************************/
class MCInit93 : public MatchFinder::MatchCallback
{
public:
  MCInit93 (Rewriter &Rewrite) : Rewrite(Rewrite) {}

  virtual void run(const MatchFinder::MatchResult &MR)
  {
    if (MR.Nodes.getNodeAs<clang::EnumConstantDecl>("mcinit93") != nullptr && MR.Nodes.getNodeAs<clang::EnumDecl>("mcinit93daddy") != nullptr)
    {
      const EnumConstantDecl * ECD [[maybe_unused]] = MR.Nodes.getNodeAs<clang::EnumConstantDecl>("mcinit93");
      const EnumDecl* ED = MR.Nodes.getNodeAs<clang::EnumDecl>("mcinit93daddy");
      /*do note that this pointer might very well be nullptr. we are actually counting on that.
      it tells us we could not match an integer initialization for this enumconstantdecl.*/
      const IntegerLiteral* IL = MR.Nodes.getNodeAs<clang::IntegerLiteral>("mcinit93kiddy");

      SourceLocation SL = ED->getLocStart(); 
      CheckSLValidity(SL);
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");
      newSourceLocation = SL;

      if (oldSourceLocation != newSourceLocation)
      {
        someoneHasInit = false;
        everyoneHasInit = true;
        isFirstElement = true;
        if (IL == nullptr)
        {
          doesFirstElementHaveInit = false;
          everyoneHasInit = false;
        }
        else
        {
          doesFirstElementHaveInit = true;
        }
      }
      else
      {
        isFirstElement = false;
      }

      if (oldSourceLocation == newSourceLocation)
      {
        if (IL == nullptr)
        {
          everyoneHasInit = false;
        }
        else
        {
          someoneHasInit = true;
        }

        if (doesFirstElementHaveInit)
        {
          if (!everyoneHasInit && someoneHasInit)
          {
            if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
            {
              /*intentionally left blank*/
            }
            else
            {
              if (Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
              {
                /*in breach of misrac*/
                std::cout << "9.3:" << "first enumeration has integerliteral initialization but not all enumerations do:";
                std::cout << SL.printToString(*MR.SourceManager) << ":" << "\n";

                XMLDocOut.XMLAddNode(MR.Context, SL, "9.3", "first enumeration has integerliteral initialization but not all enumerations do : ");
                JSONDocOUT.JSONAddElement(MR.Context, SL, "9.3", "first enumeration has integerliteral initialization but not all enumerations do : ");

                if (mutagen)
                {
                  ME.ExtractAncestry(ast_type_traits::DynTypedNode::create(*ECD), *MR.Context);
                }
              }
            }
          }
          else
          {
            /*doesnt mean anything*/
          }
        }
        else
        {
          if (IL != nullptr)
          {
            if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
            {
              /*intentionally left blank*/
            }
            else
            {
              if (Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
              {
                /*in breach of misrac*/
                std::cout << "9.3:" << "first enumeration does not have integerliteral initialization but at least one other enumeration does:";
                std::cout << SL.printToString(*MR.SourceManager) << ":" << "\n";

                XMLDocOut.XMLAddNode(MR.Context, SL, "9.3", "first enumeration does not have integerliteral initialization but at least one other enumeration does : ");
                JSONDocOUT.JSONAddElement(MR.Context, SL, "9.3", "first enumeration does not have integerliteral initialization but at least one other enumeration does : ");

                if (mutagen)
                {
                  ME.ExtractAncestry(ast_type_traits::DynTypedNode::create(*ECD), *MR.Context);
                }
              }
            }
          }
          else
          {
            /*doesnt mean anything*/
          }
        }
      }

      oldSourceLocation = newSourceLocation;
    }
  }

private:
  /*doing this instead of saving everything and then running onendoftranslationunit is faster and less memory-expensive.
  needless to say, for this to work, we are counting on clang's matching pattern.*/
  SourceLocation oldSourceLocation;
  SourceLocation newSourceLocation;

  bool isFirstElement = false;
  bool doesFirstElementHaveInit = false;
  bool someoneHasInit = false;
  bool everyoneHasInit = true;

  Rewriter &Rewrite;
};

/**********************************************************************************************************************/
class MCExpr123 : public MatchFinder::MatchCallback
{
public:
  MCExpr123 (Rewriter &Rewrite) : Rewrite(Rewrite) {}

  virtual void run(const MatchFinder::MatchResult &MR)
  {
    if (MR.Nodes.getNodeAs<clang::Expr>("mcexpr123kiddy") != nullptr)
    {
      const Expr* EXP = MR.Nodes.getNodeAs<clang::Expr>("mcexpr123kiddy");

      SourceLocation SL = EXP->getLocStart();
      CheckSLValidity(SL);
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

      if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
      {
        return void();
      }

      if (!Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
      {
        return void();
      }

      ASTContext *const ASTC = MR.Context;

      if (EXP->HasSideEffects(*ASTC, true))
      {
        std::cout << "12.3:" << "sizeof working on an expr with a side-effect:";
        std::cout << SL.printToString(*MR.SourceManager) << ":" << "\n";

        XMLDocOut.XMLAddNode(MR.Context, SL, "12.3", "sizeof working on an expr with a side-effect : ");
        JSONDocOUT.JSONAddElement(MR.Context, SL, "12.3", "sizeof working on an expr with a side-effect : ");

        if (mutagen)
        {
          ME.ExtractAncestry(ast_type_traits::DynTypedNode::create(*EXP), *MR.Context);
        }
      }
    }
  }

private:
  Rewriter &Rewrite;
};
/**********************************************************************************************************************/
class MCExpr124 : public MatchFinder::MatchCallback
{
public:
  MCExpr124 (Rewriter &Rewrite) : Rewrite(Rewrite) {}

  virtual void run(const MatchFinder::MatchResult &MR)
  {
    if (MR.Nodes.getNodeAs<clang::Expr>("mcexpr124") != nullptr)
    {
      const Expr* EXP = MR.Nodes.getNodeAs<clang::Expr>("mcexpr124");

      SourceLocation SL = EXP->getLocStart(); 
      CheckSLValidity(SL);
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

      if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
      {
        return void();
      }

      if (!Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
      {
        return void();
      }

      ASTContext *const ASTC = MR.Context;

      if (EXP->HasSideEffects(*ASTC, true))
      {
        std::cout << "12.4:" << "Righ-hand expr has side-effect:";
        std::cout << SL.printToString(*MR.SourceManager) << ":" << "\n";

        XMLDocOut.XMLAddNode(MR.Context, SL, "12.4", "Righ-hand expr has side-effect");
        JSONDocOUT.JSONAddElement(MR.Context, SL, "12.4", "Righ-hand expr has side-effect");

        if (mutagen)
        {
          ME.ExtractAncestry(ast_type_traits::DynTypedNode::create(*EXP), *MR.Context);
        }
      }
    }
  }

private:
  Rewriter &Rewrite;
};
/**********************************************************************************************************************/
/*DEVI-if all operands are boolean, this class will still tag em as inconsistent(with misrac).*/
class MCExpr125 : public MatchFinder::MatchCallback
{
public:
  MCExpr125 (Rewriter &Rewrite) : Rewrite(Rewrite) {}

  virtual void run(const MatchFinder::MatchResult &MR)
  {
    if (MR.Nodes.getNodeAs<clang::Expr>("lrhs") != nullptr)
    {
      const Expr* EXP = MR.Nodes.getNodeAs<clang::Expr>("lrhs");

      SourceLocation SL = EXP->getLocStart(); 
      CheckSLValidity(SL);
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

      ASTContext *const ASTC [[maybe_unused]] = MR.Context;

      QualType QT [[maybe_unused]] = EXP->getType();

      SourceLocation SLE = EXP->getLocEnd();
      SLE = Devi::SourceLocationHasMacro(SLE, Rewrite, "end");

      SourceRange SR;
      SR.setBegin(SL);
      SR.setEnd(SLE);

      std::string StrText = Rewrite.getRewrittenText(SR);
      if (StrText[0] == '(' && StrText[StrText.length() - 1U] == ')')
      {
        hasParantheses = true;
      }
      else
      {
        hasParantheses = false;
      }

      if (hasParantheses || SL.isMacroID())
      {
        /*intentionally left blank.*/
      }
      else
      {
        if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
        {
          /*intentionally left blank*/
        }
        else
        {
          if (Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
          {
            std::cout << "12.5:" << "RHS and/or LHS operands are not primary expressions:";
            std::cout << SL.printToString(*MR.SourceManager) << ":" << "\n";

            XMLDocOut.XMLAddNode(MR.Context, SL, "12.5", "RHS and/or LHS operands are not primary expressions : ");
            JSONDocOUT.JSONAddElement(MR.Context, SL, "12.5", "RHS and/or LHS operands are not primary expressions : ");

            if (mutagen)
            {
              ME.ExtractAncestry(ast_type_traits::DynTypedNode::create(*EXP), *MR.Context);
            }
          }
        }
      }
    }
  }

private:
  bool hasParantheses = false;

  Rewriter &Rewrite;
};
/**********************************************************************************************************************/
class MCExpr126 : public MatchFinder::MatchCallback
{
public:
  MCExpr126 (Rewriter &Rewrite) : Rewrite(Rewrite) {}

  virtual void run(const MatchFinder::MatchResult &MR)
  {
    if (MR.Nodes.getNodeAs<clang::Expr>("mcexpr126rl") != nullptr)
    {
      const Expr* EXP = MR.Nodes.getNodeAs<clang::Expr>("mcexpr126rl");

      SourceLocation SL = EXP->getLocStart(); 
      CheckSLValidity(SL);
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

      if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
      {
        return void();
      }

      if (!Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
      {
        return void();
      }

      if (!EXP->isKnownToHaveBooleanValue())
      {
        std::cout << "12.6:" << "RHS and/or LHS operands are not effectively-boolean values:";
        std::cout << SL.printToString(*MR.SourceManager) << ":" << "\n";

        XMLDocOut.XMLAddNode(MR.Context, SL, "12.6", "RHS and/or LHS operands are not effectively-boolean values : ");
        JSONDocOUT.JSONAddElement(MR.Context, SL, "12.6", "RHS and/or LHS operands are not effectively-boolean values : ");

        if (mutagen)
        {
          ME.ExtractAncestry(ast_type_traits::DynTypedNode::create(*EXP), *MR.Context);
        }
      }
    }
  }

private:
  Rewriter &Rewrite;
};
/**********************************************************************************************************************/
class MCExpr127 : public MatchFinder::MatchCallback
{
public:
  MCExpr127 (Rewriter &Rewrite) : Rewrite(Rewrite) {}

  virtual void run(const MatchFinder::MatchResult &MR)
  {
    if (MR.Nodes.getNodeAs<clang::Expr>("mcexpr127rl") != nullptr)
    {
      const Expr* EXP = MR.Nodes.getNodeAs<clang::Expr>("mcexpr127rl");

      SourceLocation SL = EXP->getLocStart(); 
      CheckSLValidity(SL);
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

      if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
      {
        return void();
      }

      if (!Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
      {
        return void();
      }

      QualType QT = EXP->getType();

      const clang::Type* TP = QT.getTypePtr();

      if (TP->hasSignedIntegerRepresentation() && TP->isIntegerType())
      {
        std::cout << "12.7:" << "Bitwise operator has signed RHS and/or LHS operands:";
        std::cout << SL.printToString(*MR.SourceManager) << ":" << "\n";

        XMLDocOut.XMLAddNode(MR.Context, SL, "12.7", "Bitwise operator has signed RHS and/or LHS operands: ");
        JSONDocOUT.JSONAddElement(MR.Context, SL, "12.7", "Bitwise operator has signed RHS and/or LHS operands: ");

        if (mutagen)
        {
          ME.ExtractAncestry(ast_type_traits::DynTypedNode::create(*EXP), *MR.Context);
        }
      }
    }
  }

private:
  Rewriter &Rewrite;
};
/**********************************************************************************************************************/
class [[maybe_unused]] MCExpr128 : public MatchFinder::MatchCallback
{
public:
  MCExpr128 (Rewriter &Rewrite) : Rewrite(Rewrite) {}

  virtual void run(const MatchFinder::MatchResult &MR)
  {
    if (MR.Nodes.getNodeAs<clang::Expr>("mcexpr128lhs") != nullptr && MR.Nodes.getNodeAs<clang::Expr>("mcexpr128rhs") != nullptr)
    {
      const Expr* RHS = MR.Nodes.getNodeAs<clang::Expr>("mcexpr128rhs");
      const Expr* LHS = MR.Nodes.getNodeAs<clang::Expr>("mcexpr128lhs");

      SourceLocation SL = RHS->getLocStart(); 
      CheckSLValidity(SL);
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

      if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
      {
        return void();
      }

      if (!Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
      {
        return void();
      }

      QualType RQT = RHS->getType();
      QualType LQT = LHS->getType();

      ASTContext *const ASTC = MR.Context;

      const clang::Type* RTP [[maybe_unused]] = RQT.getTypePtr();
      const clang::Type* LTP = LQT.getTypePtr();

      const clang::Type* CanonType = ASTC->getCanonicalType(LTP);

      uint64_t LHSSize = ASTC->getTypeSize(CanonType);

      llvm::APSInt Result;

      if (RHS->isIntegerConstantExpr(Result, *ASTC, nullptr, true))
      {
        if ((Result >= (LHSSize - 1U)) || (Result <= 0))
        {
          std::cout << "12.8:" << "shift size should be between zero and one less than the size of the LHS operand:";
          std::cout << SL.printToString(*MR.SourceManager) << ":" << "\n";

          XMLDocOut.XMLAddNode(MR.Context, SL, "12.8", "shift size should be between zero and one less than the size of the LHS operand: ");
          JSONDocOUT.JSONAddElement(MR.Context, SL, "12.8", "shift size should be between zero and one less than the size of the LHS operand: ");

          /*@DEVI-FIXME-cant extract this one correctly*/
          if (mutagen)
          {
            ME.ExtractAncestry(ast_type_traits::DynTypedNode::create(*RHS), *MR.Context);
          }
        }
      }

    }
  }

private:
  Rewriter &Rewrite;
};
/**********************************************************************************************************************/
class MCExpr129 : public MatchFinder::MatchCallback
{
public:
  MCExpr129 (Rewriter &Rewrite) : Rewrite(Rewrite) {}

  virtual void run(const MatchFinder::MatchResult &MR)
  {
    if (MR.Nodes.getNodeAs<clang::Expr>("mcexpr129") != nullptr)
    {
      const Expr* EXP = MR.Nodes.getNodeAs<clang::Expr>("mcexpr129");

      SourceLocation SL = EXP->getLocStart(); 
      CheckSLValidity(SL);
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

      if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
      {
        return void();
      }

      if (!Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
      {
        return void();
      }

      QualType QT = EXP->getType();

      const clang::Type* TP = QT.getTypePtr();

      if (TP->isIntegerType() && TP->hasUnsignedIntegerRepresentation())
      {
        std::cout << "12.9:" << "UnaryOperator - has an expr with an unsigned underlying type:";
        std::cout << SL.printToString(*MR.SourceManager) << ":" << "\n";

        XMLDocOut.XMLAddNode(MR.Context, SL, "12.9", "UnaryOperator - has an expr with an unsigned underlying type: ");
        JSONDocOUT.JSONAddElement(MR.Context, SL, "12.9", "UnaryOperator - has an expr with an unsigned underlying type: ");

        if (mutagen)
        {
          ME.ExtractAncestry(ast_type_traits::DynTypedNode::create(*EXP), *MR.Context);
        }
      }
    }
  }

private:
  Rewriter &Rewrite;
};
/**********************************************************************************************************************/
class MCExpr1210 : public MatchFinder::MatchCallback
{
public:
  MCExpr1210 (Rewriter &Rewrite) : Rewrite(Rewrite) {}

  virtual void run(const MatchFinder::MatchResult &MR)
  {
    if (MR.Nodes.getNodeAs<clang::Expr>("mcexpr1210") != nullptr)
    {
      const Expr* EXP = MR.Nodes.getNodeAs<clang::Expr>("mcexpr1210");

      SourceLocation SL = EXP->getLocStart(); 
      CheckSLValidity(SL);
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

      if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
      {
        return void();
      }

      if (!Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
      {
        return void();
      }

      std::cout << "12.10:" << "Comma used:";
      std::cout << SL.printToString(*MR.SourceManager) << ":" << "\n";

      XMLDocOut.XMLAddNode(MR.Context, SL, "12.10", "Comma used: ");
      JSONDocOUT.JSONAddElement(MR.Context, SL, "12.10", "Comma used: ");

      if (mutagen)
      {
        ME.ExtractAncestry(ast_type_traits::DynTypedNode::create(*EXP), *MR.Context);
      }
    }
  }

private:
  Rewriter &Rewrite;
};
/**********************************************************************************************************************/
class MCExpr1213 : public MatchFinder::MatchCallback
{
public:
  MCExpr1213 (Rewriter &Rewrite) : Rewrite(Rewrite) {}

  virtual void run(const MatchFinder::MatchResult &MR)
  {
    if (MR.Nodes.getNodeAs<clang::UnaryOperator>("mcexpr1213") != nullptr)
    {
      const UnaryOperator* UO = MR.Nodes.getNodeAs<clang::UnaryOperator>("mcexpr1213");

      SourceLocation SL = UO->getOperatorLoc(); 
      CheckSLValidity(SL);
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

      if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
      {
        return void();
      }

      if (!Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
      {
        return void();
      }

      std::cout << "12.13:" << "Unary ++ or -- have been used in an expr with other operators:";
      std::cout << SL.printToString(*MR.SourceManager) << ":" << "\n";

      XMLDocOut.XMLAddNode(MR.Context, SL, "12.13", "Unary ++ or -- have been used in an expr with other operators: ");
      JSONDocOUT.JSONAddElement(MR.Context, SL, "12.13", "Unary ++ or -- have been used in an expr with other operators: ");

      if (mutagen)
      {
        ME.ExtractAncestry(ast_type_traits::DynTypedNode::create(*UO), *MR.Context);
      }
    }
  }

private:
  Rewriter &Rewrite;
};
/**********************************************************************************************************************/
class MCCSE131 : public MatchFinder::MatchCallback
{
public:
  MCCSE131 (Rewriter &Rewrite) : Rewrite(Rewrite) {}

  virtual void run(const MatchFinder::MatchResult &MR)
  {
    if (MR.Nodes.getNodeAs<clang::Expr>("cse131rlhs") != nullptr)
    {
      const Expr* EXP = MR.Nodes.getNodeAs<clang::Expr>("cse131rlhs");

      SourceLocation SL = EXP->getLocStart(); 
      CheckSLValidity(SL);
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

      if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
      {
        return void();
      }

      if (!Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
      {
        return void();
      }

      if (EXP->isKnownToHaveBooleanValue())
      {
        std::cout << "13.1:" << "assignment operator used in an expr that is known to return boolean:";
        std::cout << SL.printToString(*MR.SourceManager) << ":" << "\n";

        XMLDocOut.XMLAddNode(MR.Context, SL, "13.1", "assignment operator used in an expr that is known to return boolean: ");
        JSONDocOUT.JSONAddElement(MR.Context, SL, "13.1", "assignment operator used in an expr that is known to return boolean: ");

        if (mutagen)
        {
          ME.ExtractAncestry(ast_type_traits::DynTypedNode::create(*EXP), *MR.Context);
        }
      }
    }
  }

private:
  Rewriter &Rewrite;
};
/**********************************************************************************************************************/
class MCCSE132 : public MatchFinder::MatchCallback
{
public:
  MCCSE132 (Rewriter &Rewrite) : Rewrite(Rewrite) {}

  virtual void run(const MatchFinder::MatchResult &MR)
  {
    if (MR.Nodes.getNodeAs<clang::Expr>("mccse132") != nullptr)
    {
      const Expr* EXP = MR.Nodes.getNodeAs<clang::Expr>("mccse132");

      SourceLocation SL = EXP->getLocStart(); 
      CheckSLValidity(SL);
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

      if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
      {
        return void();
      }

      if (!Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
      {
        return void();
      }

      if (!EXP->isKnownToHaveBooleanValue())
      {
        std::cout << "13.2:" << "Implicit test of an expr against zero which is not known to return a boolean result:";
        std::cout << SL.printToString(*MR.SourceManager) << ":" << "\n";

        XMLDocOut.XMLAddNode(MR.Context, SL, "13.2", "Implicit test of an expr against zero which is not known to return a boolean result: ");
        JSONDocOUT.JSONAddElement(MR.Context, SL, "13.2", "Implicit test of an expr against zero which is not known to return a boolean result: ");

        if (mutagen)
        {
          ME.ExtractAncestry(ast_type_traits::DynTypedNode::create(*EXP), *MR.Context);
        }
      }
    }
  }

private:
  Rewriter &Rewrite;
};
/**********************************************************************************************************************/
class MCCSE1332 : public MatchFinder::MatchCallback
{
public:
  MCCSE1332 (Rewriter &Rewrite) : Rewrite (Rewrite) {}

  virtual void run(const MatchFinder::MatchResult &MR)
  {
    if (MR.Nodes.getNodeAs<clang::Expr>("mccse1332rl") != nullptr)
    {
      const Expr* EXP = MR.Nodes.getNodeAs<clang::Expr>("mccse1332rl");
      const BinaryOperator* BO = MR.Nodes.getNodeAs<clang::BinaryOperator>("mccse1332daddy");

      SourceLocation SLD = BO->getLocStart(); 
      CheckSLValidity(SLD);
      SLD = Devi::SourceLocationHasMacro(SLD, Rewrite, "start");
      NewSL = SLD;

      SourceLocation SL = EXP->getLocStart(); 
      CheckSLValidity(SL);
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

      QualType QT = EXP->getType();

      const clang::Type* TP = QT.getTypePtr();

      if (OldSL != NewSL)
      {
        if (TP->hasFloatingRepresentation())
        {
          if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
          {
            /*intentionally left blank*/
          }
          else
          {
            if (Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
            {
              std::cout << "13.3:" << "Float type expression checked for equality/inequality:";
              std::cout << SL.printToString(*MR.SourceManager) << ":" << "\n";

              XMLDocOut.XMLAddNode(MR.Context, SL, "13.3", "Float type expression checked for equality/inequality: ");
              JSONDocOUT.JSONAddElement(MR.Context, SL, "13.3", "Float type expression checked for equality/inequality: ");

              if (mutagen)
              {
                ME.ExtractAncestry(ast_type_traits::DynTypedNode::create(*EXP), *MR.Context);
              }
            }
          }
        }
      }

      OldSL = NewSL;
    }
  }

private:
  SourceLocation NewSL;
  SourceLocation OldSL;

  Rewriter &Rewrite;
};
/**********************************************************************************************************************/
class MCCSE134 : public MatchFinder::MatchCallback
{
public:
  MCCSE134 (Rewriter &Rewrite) : Rewrite(Rewrite) {}

  virtual void run(const MatchFinder::MatchResult &MR)
  {
    if (MR.Nodes.getNodeAs<clang::ForStmt>("mccse134") != nullptr)
    {
      AlreadyHaveAHit = false;

      const ForStmt* FS = MR.Nodes.getNodeAs<clang::ForStmt>("mccse134");

      SourceLocation SL = FS->getLocStart(); 
      CheckSLValidity(SL);
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

      const Expr* FSCond = FS->getCond();
      const Expr* FSInc = FS->getInc();

#if 0
      if (FSCond != nullptr)
      {
        std::string multix = Rewrite.getRewrittenText(FSCond->getSourceRange());
        std::cout << "diagnostic" << ":" << multix << "\n";
      }
#endif

      if (FSCond != nullptr)
      {
        QualType QTCond = FSCond->getType();

        const clang::Type* TPCond = QTCond.getTypePtr();

        if (TPCond->hasFloatingRepresentation())
        {
          if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
          {
            /*intentionally left blank*/
          }
          else
          {
            if (Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
            {
              std::cout << "13.4:" << "Float type used in the controlling expression of a forstmt:";
              std::cout << SL.printToString(*MR.SourceManager) << ":" << "\n";
              AlreadyHaveAHit = true;

              XMLDocOut.XMLAddNode(MR.Context, SL, "13.4", "Float type used in the controlling expression of a forstmt: ");
              JSONDocOUT.JSONAddElement(MR.Context, SL, "13.4", "Float type used in the controlling expression of a forstmt: ");

              if (mutagen)
              {
                ME.ExtractAncestry(ast_type_traits::DynTypedNode::create(*FS), *MR.Context);
              }
            }
          }
        }
      }

      if (FSInc != nullptr && !AlreadyHaveAHit)
      {
        QualType QTInc = FSInc->getType();

        const clang::Type* TPInc = QTInc.getTypePtr();

        if (TPInc->hasFloatingRepresentation())
        {
          if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
          {
            /*intentionally left blank*/
          }
          else
          {
            if (Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
            {
              std::cout << "13.4:" << "Float type used in the controlling expression of a forstmt:";
              std::cout << SL.printToString(*MR.SourceManager) << ":" << "\n";
              AlreadyHaveAHit = true;

              XMLDocOut.XMLAddNode(MR.Context, SL, "13.4", "Float type used in the controlling expression of a forstmt: ");
              JSONDocOUT.JSONAddElement(MR.Context, SL, "13.4", "Float type used in the controlling expression of a forstmt: ");

              if (mutagen)
              {
                ME.ExtractAncestry(ast_type_traits::DynTypedNode::create(*FS), *MR.Context);
              }
            }
          }
        }
      }
    }
  }

private:
  bool AlreadyHaveAHit = false;

  Rewriter &Rewrite;
};
/**********************************************************************************************************************/
/*JANKY*/
/*if a for controlling var is modified in the body using a pointer, then this class wont find it.*/
/*the class will only work properly only if there is one controlling loop variable.
the behavior is undefined for more than one variable.*/
class MCCSE136 : public MatchFinder::MatchCallback
{
public:
  MCCSE136 (Rewriter &Rewrite) : Rewrite(Rewrite) {}

  virtual void run(const MatchFinder::MatchResult &MR)
  {
    if (MR.Nodes.getNodeAs<clang::DeclRefExpr>("mccse136kiddo") != nullptr)
    {
      const DeclRefExpr* DRE = MR.Nodes.getNodeAs<clang::DeclRefExpr>("mccse136kiddo");
      const ForStmt* FS = MR.Nodes.getNodeAs<clang::ForStmt>("mccse136daddy");

      const Stmt* FSInit = FS->getInit();
      const Expr* FSInc = FS->getInc();
      const Expr* FSCond [[maybe_unused]] = FS->getCond();

      /*underdev*/
      if (FSCond != nullptr)
      {
        SourceLocation CSL = FSCond->getLocStart(); 
        CheckSLValidity(CSL);
        SourceLocation CSLE = FSCond->getLocEnd();
        SourceRange CSR;
        CSR.setBegin(CSL);
        CSR.setEnd(CSLE);

        std::string outstring = Rewrite.getRewrittenText(CSR);

#if 0
        std::cout << "XXXXXXXXXXXXXXXXXXXXXX" << outstring << "\n";
#endif
      }


      SourceLocation SLD = FS->getLocStart(); 
      CheckSLValidity(SLD);
      SLD = Devi::SourceLocationHasMacro(SLD, Rewrite, "start");
      SourceLocation SL = DRE->getLocStart(); 
      CheckSLValidity(SL);
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

      if (FSInit != nullptr && FSInc != nullptr)
      {
        SourceLocation SLFSInit = FSInit->getLocStart();
        SLFSInit = Devi::SourceLocationHasMacro(SLFSInit, Rewrite, "start");
        SourceLocation SLFSInc = FSInc->getLocStart();
        SLFSInc = Devi::SourceLocationHasMacro(SLFSInc, Rewrite, "start");

        DeclarationNameInfo DNI = DRE->getNameInfo();

        std::string NameString = DNI.getAsString();

        /*JANKY*/
        /*@DEVI-the third condition is put in place to accomodate the prefix unary increment or decrement operator.*/
        if (SLFSInit == SL || SLFSInc == SL || SLFSInc.getLocWithOffset(2) == SL)
        {
          ControlVarName = NameString;
        }
        else
        {
          if (ControlVarName == NameString)
          {
            if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
            {
              /*intentionally left blank*/
            }
            else
            {
              if (Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
              {
                std::cout << "13.6:" << "ForStmt controlling variable modified in the body of the loop:";
                std::cout << SL.printToString(*MR.SourceManager) << ":" << "\n";

                XMLDocOut.XMLAddNode(MR.Context, SL, "13.6", "ForStmt controlling variable modified in the body of the loop: ");
                JSONDocOUT.JSONAddElement(MR.Context, SL, "13.6", "ForStmt controlling variable modified in the body of the loop: ");

                if (mutagen)
                {
                  ME.ExtractAncestry(ast_type_traits::DynTypedNode::create(*DRE), *MR.Context);
                }
              }
            }
          }
        }

      }
    }
  }

private:
  std::string ControlVarName;

  Rewriter &Rewrite;
};
/**********************************************************************************************************************/
class MCCF144 : public MatchFinder::MatchCallback
{
public:
  MCCF144 (Rewriter &Rewrite) : Rewrite(Rewrite) {}

  virtual void run(const MatchFinder::MatchResult &MR)
  {
    if (MR.Nodes.getNodeAs<clang::GotoStmt>("mccf144") != nullptr)
    {
      const GotoStmt* GS = MR.Nodes.getNodeAs<clang::GotoStmt>("mccf144");

      SourceLocation SL = GS->getLocStart(); 
      CheckSLValidity(SL);
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

      if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
      {
        return void();
      }

      if (!Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
      {
        return void();
      }

      std::cout << "14.4:" << "GotoStmt used:";
      std::cout << SL.printToString(*MR.SourceManager) << ":" << "\n";

      XMLDocOut.XMLAddNode(MR.Context, SL, "14.4", "GotoStmt used: ");
      JSONDocOUT.JSONAddElement(MR.Context, SL, "14.4", "GotoStmt used: ");
    }
  }

private:
  Rewriter &Rewrite;
};
/**********************************************************************************************************************/
class MCCF145 : public MatchFinder::MatchCallback
{
public:
  MCCF145 (Rewriter &Rewrite) : Rewrite(Rewrite) {}

  virtual void run(const MatchFinder::MatchResult &MR)
  {
    if (MR.Nodes.getNodeAs<clang::ContinueStmt>("mccf145") != nullptr)
    {
      const ContinueStmt* CS = MR.Nodes.getNodeAs<clang::ContinueStmt>("mccf145");

      SourceLocation SL = CS->getLocStart(); 
      CheckSLValidity(SL);
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

      if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
      {
        return void();
      }

      if (!Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
      {
        return void();
      }

      std::cout << "14.5:" << "ContinueStmt used:";
      std::cout << SL.printToString(*MR.SourceManager) << ":" << "\n";

      XMLDocOut.XMLAddNode(MR.Context, SL, "14.5", "ContinueStmt used: ");
      JSONDocOUT.JSONAddElement(MR.Context, SL, "14.5", "ContinueStmt used: ");
    }
  }

private:
  Rewriter &Rewrite;
};
/**********************************************************************************************************************/
class MCCF146 : public MatchFinder::MatchCallback
{
public:
  MCCF146 (Rewriter &Rewrite) : Rewrite(Rewrite) {}

  virtual void run(const MatchFinder::MatchResult &MR)
  {
    if (MR.Nodes.getNodeAs<clang::ForStmt>("mccffofo") != nullptr)
    {
      const ForStmt* FS =  MR.Nodes.getNodeAs<clang::ForStmt>("mccffofo");

      SL = FS->getLocStart(); 
      CheckSLValidity(SL);
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");
    }

    if (MR.Nodes.getNodeAs<clang::WhileStmt>("mccfwuwu") != nullptr)
    {
      const WhileStmt* WS =  MR.Nodes.getNodeAs<clang::WhileStmt>("mccfwuwu");

      SL = WS->getLocStart(); 
      CheckSLValidity(SL);
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");
    }

    if (MR.Nodes.getNodeAs<clang::DoStmt>("mccfdodo") != nullptr)
    {
      const DoStmt* DS =  MR.Nodes.getNodeAs<clang::DoStmt>("mccfdodo");

      SL = DS->getLocStart(); 
      CheckSLValidity(SL);
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");
    }

    NewSL = SL;

    if (OldSL != NewSL)
    {
      AlreadyTagged = false;
      BreakCounter = 1U;
    }

    if (OldSL == NewSL)
    {
      BreakCounter++;
    }

    if (BreakCounter >= 2U && !AlreadyTagged)
    {
      if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
      {
        /*intentionally left blank*/
      }
      else
      {
        if (Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
        {
          std::cout << "14.6:" << "More than one BreakStmt used in the loop counter:";
          std::cout << SL.printToString(*MR.SourceManager) << ":" << "\n";
          AlreadyTagged = true;

          XMLDocOut.XMLAddNode(MR.Context, SL, "14.6", "More than one BreakStmt used in the loop counter: ");
          JSONDocOUT.JSONAddElement(MR.Context, SL, "14.6", "More than one BreakStmt used in the loop counter: ");
        }
      }
    }

    OldSL = NewSL;
  }

private:
  SourceLocation OldSL;
  SourceLocation NewSL;
  bool AlreadyTagged = false;
  unsigned int BreakCounter = 0U;
  SourceLocation SL;

  Rewriter &Rewrite;
};
/**********************************************************************************************************************/
class MCCF147 : public MatchFinder::MatchCallback
{
public:
  MCCF147 (Rewriter &Rewrite) : Rewrite(Rewrite) {}

  virtual void run(const MatchFinder::MatchResult &MR)
  {
    if (MR.Nodes.getNodeAs<clang::FunctionDecl>("mccf147") != nullptr)
    {
      const FunctionDecl* FD = MR.Nodes.getNodeAs<clang::FunctionDecl>("mccf147");

      SourceLocation SL = FD->getLocStart(); 
      CheckSLValidity(SL);
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

      NewSL = SL;

      if (OldSL != NewSL)
      {
        AlreadyTagged = false;
        ReturnCounter = 1U;
      }

      if (OldSL == NewSL)
      {
        ReturnCounter++;
      }

      if (ReturnCounter >= 2U && !AlreadyTagged)
      {
        if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
        {
          /*intentionally left blank*/
        }
        else
        {
          if (Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
          {
            std::cout << "14.7:" << "More than one ReturnStmt used in the body of FunctionDecl:";
            std::cout << SL.printToString(*MR.SourceManager) << ":" << "\n";
            AlreadyTagged = true;

            XMLDocOut.XMLAddNode(MR.Context, SL, "14.7", "More than one ReturnStmt used in the body of FunctionDecl: ");
            JSONDocOUT.JSONAddElement(MR.Context, SL, "14.7", "More than one ReturnStmt used in the body of FunctionDecl: ");
          }
        }
      }

      OldSL = NewSL;
    }
  }

private:
  SourceLocation NewSL;
  SourceLocation OldSL;
  unsigned int ReturnCounter = 0U;
  bool AlreadyTagged = false;

  Rewriter &Rewrite;
};
/**********************************************************************************************************************/
class MCCF148 : public MatchFinder::MatchCallback
{
public:
  MCCF148 (Rewriter &Rewrite) : Rewrite(Rewrite) {}

  virtual void run(const MatchFinder::MatchResult &MR)
  {

    if (MR.Nodes.getNodeAs<clang::ForStmt>("mccf148for") != nullptr)
    {
      const ForStmt* FS = MR.Nodes.getNodeAs<clang::ForStmt>("mccf148for");

      SL = FS->getLocStart(); 
      CheckSLValidity(SL);
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

      if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
      {
        /*intentionally left blank*/
      }
      else
      {
        if (Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
        {
          std::cout << "14.8:" << "ForStmt does not have a child CompoundStmt:";
          std::cout << SL.printToString(*MR.SourceManager) << ":" << "\n";

          XMLDocOut.XMLAddNode(MR.Context, SL, "14.8", "ForStmt does not have a child CompoundStmt: ");
          JSONDocOUT.JSONAddElement(MR.Context, SL, "14.8", "ForStmt does not have a child CompoundStmt: ");
        }
      }
    }

    if (MR.Nodes.getNodeAs<clang::WhileStmt>("mccf148while") != nullptr)
    {
      const WhileStmt* WS = MR.Nodes.getNodeAs<clang::WhileStmt>("mccf148while");

      SL = WS->getLocStart(); 
      CheckSLValidity(SL);
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

      if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
      {
        /*intentionally left blank*/
      }
      else
      {
        if (Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
        {
          std::cout << "14.8:" << "WhileStmt does not have a child CompoundStmt:";
          std::cout << SL.printToString(*MR.SourceManager) << ":" << "\n";

          XMLDocOut.XMLAddNode(MR.Context, SL, "14.8", "WhileStmt does not have a child CompoundStmt: ");
          JSONDocOUT.JSONAddElement(MR.Context, SL, "14.8", "WhileStmt does not have a child CompoundStmt: ");
        }
      }
    }

    if (MR.Nodes.getNodeAs<clang::DoStmt>("mccf148do") != nullptr)
    {
      const DoStmt* DS = MR.Nodes.getNodeAs<clang::DoStmt>("mccf148do");

      SL = DS->getLocStart(); 
      CheckSLValidity(SL);
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

      if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
      {
        /*intentionally left blank*/
      }
      else
      {
        if (Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
        {
          std::cout << "14.8:" << "DoStmt does not have a child CompoundStmt:";
          std::cout << SL.printToString(*MR.SourceManager) << ":" << "\n";

          XMLDocOut.XMLAddNode(MR.Context, SL, "14.8", "DoStmt does not have a child CompoundStmt: ");
          JSONDocOUT.JSONAddElement(MR.Context, SL, "14.8", "DoStmt does not have a child CompoundStmt: ");
        }
      }
    }

    if (MR.Nodes.getNodeAs<clang::SwitchStmt>("mccf148switch") != nullptr)
    {
      const SwitchStmt* SS = MR.Nodes.getNodeAs<clang::SwitchStmt>("mccf148switch");

      SL = SS->getLocStart(); 
      CheckSLValidity(SL);
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

      if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
      {
        /*intentionally left blank*/
      }
      else
      {
        if (Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
        {
          std::cout << "14.8:" << "SwitchStmt does not have a child CompoundStmt:";
          std::cout << SL.printToString(*MR.SourceManager) << ":" << "\n";

          XMLDocOut.XMLAddNode(MR.Context, SL, "14.8", "SwitchStmt does not have a child CompoundStmt: ");
          JSONDocOUT.JSONAddElement(MR.Context, SL, "14.8", "SwitchStmt does not have a child CompoundStmt: ");
        }
      }
    }
  }

private:
  SourceLocation SL;

  Rewriter &Rewrite;
};
/**********************************************************************************************************************/
class MCSwitch154 : public MatchFinder::MatchCallback\
{
public:
  MCSwitch154 (Rewriter &Rewrite) : Rewrite(Rewrite) {}

  virtual void run(const MatchFinder::MatchResult &MR)
  {
    const Expr* EXP = MR.Nodes.getNodeAs<clang::Expr>("mcswitch154");

    SourceLocation SL = EXP->getLocStart(); 
      CheckSLValidity(SL);
    SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

    if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
    {
      return void();
    }

    if (!Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
    {
      return void();
    }

    if (EXP->isKnownToHaveBooleanValue())
    {
      std::cout << "15.4:" << "Switch expression is effectively boolean:";
      std::cout << SL.printToString(*MR.SourceManager) << ":" << "\n";

      XMLDocOut.XMLAddNode(MR.Context, SL, "15.4", "Switch expression is effectively boolean: ");
      JSONDocOUT.JSONAddElement(MR.Context, SL, "15.4", "Switch expression is effectively boolean: ");

      if (mutagen)
      {
        ME.ExtractAncestry(ast_type_traits::DynTypedNode::create(*EXP), *MR.Context);
      }
    }
  }

private:
  Rewriter &Rewrite;
};
/**********************************************************************************************************************/
/*@DEVI-the current implementation of 11.3 is very strict.*/
class MCPTC111 : public MatchFinder::MatchCallback
{
public:
  MCPTC111 (Rewriter &Rewrite) : Rewrite(Rewrite) {}

  virtual void run(const MatchFinder::MatchResult &MR)
  {
    if (MR.Nodes.getNodeAs<clang::ImplicitCastExpr>("mcptc111") != nullptr)
    {
      const ImplicitCastExpr* ICE = MR.Nodes.getNodeAs<clang::ImplicitCastExpr>("mcptc111");

      SourceLocation SL = ICE->getLocStart(); 
      CheckSLValidity(SL);
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

      QualType QT = ICE->getType();

      const clang::Type* TP = QT.getTypePtr();

#if 0
      ASTContext *const ASTC = MR.Context;

      const ASTContext::DynTypedNodeList NodeList = ASTC->getParents(*ICE);

      /*assumptions:implicitcastexpr does not have more than one parent in C.*/
      const ast_type_traits::DynTypedNode ParentNode = NodeList[0];

      ast_type_traits::ASTNodeKind ParentNodeKind = ParentNode.getNodeKind();

      std::string StringKind = ParentNodeKind.asStringRef().str();

      const ImplicitCastExpr* ParentICE = ParentNode.get<clang::ImplicitCastExpr>();
#endif

      CastKind CK = ICE->getCastKind();

      bool ShouldBeTagged111 = false;

      if (TP->isFunctionPointerType())
      {
        if (((CK != CK_IntegralToPointer) && (CK != CK_PointerToIntegral) && \
             (CK != CK_LValueToRValue) && (CK != CK_FunctionToPointerDecay) && \
             (CK != CK_ArrayToPointerDecay)))
        {
          ShouldBeTagged111 = true;
        }

        if (CK == CK_BitCast)
        {
          ShouldBeTagged111 = true;
        }

        if (ShouldBeTagged111)
        {
          if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
          {
            /*intentionally left blank*/
          }
          else
          {
            if (Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
            {
              std::cout << "11.1:" << "ImplicitCastExpr - FunctionPointerType converted to or from a type other than IntegralType:";
              std::cout << SL.printToString(*MR.SourceManager) << ":" << "\n";

              XMLDocOut.XMLAddNode(MR.Context, SL, "11.1", "ImplicitCastExpr - FunctionPointerType converted to or from a type other than IntegralType: ");
              JSONDocOUT.JSONAddElement(MR.Context, SL, "11.1", "ImplicitCastExpr - FunctionPointerType converted to or from a type other than IntegralType: ");

              if (mutagen)
              {
                ME.ExtractAncestry(ast_type_traits::DynTypedNode::create(*ICE), *MR.Context);
              }
            }
          }
        }
      }

      if (CK == CK_IntegralToFloating || CK == CK_FloatingToIntegral)
      {
        if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
        {
          /*intentionally left blank*/
        }
        else
        {
          if (Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
          {
            std::cout << "10.1/2:" << "ImplicitCastExpr - Conversion of FloatingType to or from IntegralType is recommended against:";
            std::cout << SL.printToString(*MR.SourceManager) << ":" << "\n";

            XMLDocOut.XMLAddNode(MR.Context, SL, "10.1/2", "ImplicitCastExpr - Conversion of FloatingType to or from IntegralType is recommended against: ");
            JSONDocOUT.JSONAddElement(MR.Context, SL, "10.1/2", "ImplicitCastExpr - Conversion of FloatingType to or from IntegralType is recommended against: ");

            if (mutagen)
            {
              ME.ExtractAncestry(ast_type_traits::DynTypedNode::create(*ICE), *MR.Context);
            }
          }
        }
      }

      if ((CK == CK_IntegralToPointer) || (CK == CK_PointerToIntegral))
      {
        if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
        {
          /*intentionally left blank*/
        }
        else
        {
          if (Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
          {
            std::cout << "11.3:" << "ImplicitCastExpr - Conversion of PointerType to or from IntegralType is recommended against:";
            std::cout << SL.printToString(*MR.SourceManager) << ":" << "\n";

            XMLDocOut.XMLAddNode(MR.Context, SL, "11.3", "ImplicitCastExpr - Conversion of PointerType to or from IntegralType is recommended against: ");
            JSONDocOUT.JSONAddElement(MR.Context, SL, "11.3", "ImplicitCastExpr - Conversion of PointerType to or from IntegralType is recommended against: ");

            if (mutagen)
            {
              ME.ExtractAncestry(ast_type_traits::DynTypedNode::create(*ICE), *MR.Context);
            }
          }
        }
      }

      if (CK == CK_BitCast || CK == CK_PointerToBoolean || CK == CK_AnyPointerToBlockPointerCast)
      {
        if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
        {
          /*intentionally left blank*/
        }
        else
        {
          if (Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
          {
            std::cout << "11.x:" << "ImplicitCastExpr - PointerType has implicit BitCast. This could be caused by a cast removing const or volatile qualifier from the type addressed by a pointer or by a cast to a different function or object type:";
            std::cout << SL.printToString(*MR.SourceManager) << ":" << "\n";

            XMLDocOut.XMLAddNode(MR.Context, SL, "11.x", "ImplicitCastExpr - PointerType has implicit BitCast. This could be caused by a cast removing const or volatile qualifier from the type addressed by a pointer or by a cast to a different function or object type: ");
            JSONDocOUT.JSONAddElement(MR.Context, SL, "11.x", "ImplicitCastExpr - PointerType has implicit BitCast. This could be caused by a cast removing const or volatile qualifier from the type addressed by a pointer or by a cast to a different function or object type: ");

            if (mutagen)
            {
              ME.ExtractAncestry(ast_type_traits::DynTypedNode::create(*ICE), *MR.Context);
            }
          }
        }
      }
    }
  }

private:
  Rewriter &Rewrite;
};
/**********************************************************************************************************************/
class MCCSE137 : public MatchFinder::MatchCallback
{
public:
  MCCSE137 (Rewriter &Rewrite) : Rewrite(Rewrite) {}

  virtual void run(const MatchFinder::MatchResult &MR)
  {
    if (MR.Nodes.getNodeAs<clang::Expr>("mccse137") != nullptr)
    {
      const Expr* EXP = MR.Nodes.getNodeAs<clang::Expr>("mccse137");

      SourceLocation SL = EXP->getLocStart(); 
      CheckSLValidity(SL);
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

      ASTContext *const ASTC = MR.Context;

      if (EXP->isKnownToHaveBooleanValue())
      {
        if (EXP->isEvaluatable(*ASTC, Expr::SE_NoSideEffects))
        {
          if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
          {
            /*intentionally left blank*/
          }
          else
          {
            if (Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
            {
              std::cout << "13.7:" << "EffectivelyBooleanExpr's result is known at compile-time:";
              std::cout << SL.printToString(*MR.SourceManager) << ":" << "\n";

              XMLDocOut.XMLAddNode(MR.Context, SL, "13.7", "EffectivelyBooleanExpr's result is known at compile-time: ");
              JSONDocOUT.JSONAddElement(MR.Context, SL, "13.7", "EffectivelyBooleanExpr's result is known at compile-time: ");

              if (mutagen)
              {
                ME.ExtractAncestry(ast_type_traits::DynTypedNode::create(*EXP), *MR.Context);
              }
            }
          }
        }
      }

      QualType QT = EXP->getType();
      const clang::Type* TP = QT.getTypePtr();

      if (TP->isIntegerType())
      {
        ASTContext::DynTypedNodeList NodeList = ASTC->getParents(*EXP);

        /*assumptions:nothing has more than one parent in C.*/
        ast_type_traits::DynTypedNode ParentNode = NodeList[0];

        ast_type_traits::ASTNodeKind ParentNodeKind = ParentNode.getNodeKind();

        std::string StringKind = ParentNodeKind.asStringRef().str();

        if (StringKind == "ImplicitCastExpr")
        {

        }
      }
    }
  }

private:
  Rewriter &Rewrite;
};
/**********************************************************************************************************************/
/*if the call is coming from another file, getdirectcalle returns the definition,
but if its coming from the same file, it returns the declaration that is not a definition.*/
/*if youve already matched the definition, getdefinition returns null.*/
class MCDCDF810 : public MatchFinder::MatchCallback
{
public:
  MCDCDF810 (Rewriter &Rewrite) : Rewrite(Rewrite)
  {
    /*@DEVI-the pushback here generates garbage entries.*/
    FuncScopeProto.push_back(FuncScope());

    VecC = 0U;
  }

  virtual void run(const MatchFinder::MatchResult &MR)
  {
    if (MR.Nodes.getNodeAs<clang::CallExpr>("mcdcdf810") != nullptr && MR.Nodes.getNodeAs<clang::FunctionDecl>("mcdcdf810daddy") != nullptr)
    {
      AlreadyTagged = false;

      const CallExpr* CE = MR.Nodes.getNodeAs<clang::CallExpr>("mcdcdf810");
      const FunctionDecl* FDDad = MR.Nodes.getNodeAs<clang::FunctionDecl>("mcdcdf810daddy");
      const FunctionDecl* FD = CE->getDirectCallee();
      const FunctionDecl* FDDef = FD->getDefinition();;

      if (FDDef == nullptr)
      {
        FDDef = CE->getDirectCallee();
      }

      SourceLocation CESL = CE->getLocStart(); 
      CheckSLValidity(CESL);
      CESL = Devi::SourceLocationHasMacro(CESL, Rewrite, "start");

      SourceLocation FDDadSL = FDDad->getLocStart(); 
      CheckSLValidity(FDDadSL);
      FDDadSL = Devi::SourceLocationHasMacro(FDDadSL, Rewrite, "start");

      SourceLocation FDSL = FDDef->getLocStart(); 
      CheckSLValidity(FDSL);
      FDSL = Devi::SourceLocationHasMacro(FDSL, Rewrite, "start");

      ASTContext *const ASTC = MR.Context;

      FullSourceLoc FDDadFullSL = ASTC->getFullLoc(FDDadSL);
      FullSourceLoc FDFullSL = ASTC->getFullLoc(FDSL);

      DeclarationNameInfo DNI = FDDef->getNameInfo();
      std::string MatchedName = DNI.getAsString();

      /*going through the already matched functions,making sure we are not adding duplicates.*/
      for (unsigned x = 0; x < VecC; ++x)
      {
        if (FuncScopeProto[x].FuncNameString == MatchedName && FuncScopeProto[x].DefinitionSL == FDSL.printToString(*MR.SourceManager))
        {
          AlreadyTagged = true;

          if (FDDef->isExternC())
          {
            if (FDDadFullSL.getFileID() != FDFullSL.getFileID())
            {
              FuncScopeProto[x].hasExternalCall = true;
            }
          }
        }
      }

      if (AlreadyTagged == false && FDDef->isExternC())
      {
        FuncScopeProto.push_back(FuncScope());
        FuncScopeProto[VecC].FuncNameString = MatchedName;
        FuncScopeProto[VecC].DefinitionSL = FDSL.printToString(*MR.SourceManager);
        FuncScopeProto[VecC].FuncScopeSL = FDSL;
        FuncScopeProto[VecC].FuncScopeFSL = MR.Context->getFullLoc(FDSL);

        if (FDDef->isExternC())
        {
          if (FDDadFullSL.getFileID() != FDFullSL.getFileID())
          {
            FuncScopeProto[VecC].hasExternalCall = true;
          }
        }

        VecC++;
      }
    }
  }

  virtual void onEndOfTranslationUnit()
  {
    for (unsigned x = 0; x < VecC; ++x)
    {
      if (FuncScopeProto[x].hasExternalCall == false)
      {
        if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, FuncScopeProto[x].FuncScopeFSL.isInSystemHeader(), FuncScopeProto[x].FuncScopeSL))
        {
          /*intentionally left blank*/
        }
        else
        {
          if (Devi::IsTheMatchInMainFile(MainFileOnly, FuncScopeProto[x].FuncScopeFSL.getManager().isInMainFile(FuncScopeProto[x].FuncScopeSL), FuncScopeProto[x].FuncScopeSL))
          {
            std::cout << "8.11:" << "Function does not have any external calls but is not declared as static:";
            std::cout << FuncScopeProto[x].DefinitionSL << ":" << "\n";

            XMLDocOut.XMLAddNode(FuncScopeProto[x].FuncScopeFSL, FuncScopeProto[x].FuncScopeSL, "8.11", "Function does not have any external calls but is not declared as static : ");
            JSONDocOUT.JSONAddElement(FuncScopeProto[x].FuncScopeFSL, FuncScopeProto[x].FuncScopeSL, "8.11", "Function does not have any external calls but is not declared as static : ");
          }
        }
      }
    }
  }

private:
  struct FuncScope {
    std::string FuncNameString;
    bool hasExternalCall = false;
    std::string DefinitionSL;
    SourceLocation FuncScopeSL;
    FullSourceLoc FuncScopeFSL;
  };

  bool AlreadyTagged = false;

  unsigned VecC;

  std::vector<FuncScope> FuncScopeProto;

  Rewriter &Rewrite;
};
/**********************************************************************************************************************/
/*@DEVI-also flags the main.*/
class MCFunction165 : public MatchFinder::MatchCallback
{
public:
  MCFunction165 (Rewriter &Rewrite) : Rewrite(Rewrite) {}

  virtual void run(const MatchFinder::MatchResult &MR)
  {
    if (MR.Nodes.getNodeAs<clang::FunctionDecl>("mcfunction165") != nullptr)
    {
      const FunctionDecl* FD = MR.Nodes.getNodeAs<clang::FunctionDecl>("mcfunction165");

      SourceLocation SL = FD->getLocStart(); 
      CheckSLValidity(SL);
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

      if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
      {
        return void();
      }

      if (!Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
      {
        return void();
      }

      std::cout << "16.5:" << "Function does not return anything but is missing the void keyword for the return type:";
      std::cout << SL.printToString(*MR.SourceManager) << ":" << "\n";

      XMLDocOut.XMLAddNode(MR.Context, SL, "16.5", "Function does not return anything but is missing the void keyword for the return type : ");
      JSONDocOUT.JSONAddElement(MR.Context, SL, "16.5", "Function does not return anything but is missing the void keyword for the return type : ");
    }
  }

private:
  Rewriter &Rewrite;
};
/**********************************************************************************************************************/
class MCFunction1652 : public MatchFinder::MatchCallback
{
public:
  MCFunction1652 (Rewriter &Rewrite) : Rewrite(Rewrite) {}

  virtual void run(const MatchFinder::MatchResult &MR)
  {
    if (MR.Nodes.getNodeAs<clang::FunctionDecl>("mcfunction1652") != nullptr)
    {
      const FunctionDecl* FD = MR.Nodes.getNodeAs<clang::FunctionDecl>("mcfunction1652");

      SourceLocation SL = FD->getLocStart(); 
      CheckSLValidity(SL);
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");
      SourceLocation SLE = FD->getBody()->getLocStart(); 
      CheckSLValidity(SLE);
      SLE = Devi::SourceLocationHasMacro(SLE, Rewrite, "end");

      if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
      {
        return void();
      }

      if (!Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
      {
        return void();
      }

      SourceRange SR;
      SR.setBegin(SL);
      SR.setEnd(SLE);

      std::string FunctionSigAsString = Rewrite.getRewrittenText(SR);

      DeclarationNameInfo DNI = FD->getNameInfo();

      std::string NameAsString = DNI.getAsString();

      size_t voidposition = FunctionSigAsString.find(NameAsString, 0U);
      unsigned lengthofstring = NameAsString.length();
      size_t voidposition2 = FunctionSigAsString.find("void", voidposition + lengthofstring - 1U);

      if (voidposition2 == std::string::npos)
      {
        std::cout << "16.5:" << "Function does not take any parameters but is not using the void keyword:";
        std::cout << SL.printToString(*MR.SourceManager) << ":" << "\n";

        XMLDocOut.XMLAddNode(MR.Context, SL, "16.5", "Function does not take any parameters but is not using the void keyword : ");
        JSONDocOUT.JSONAddElement(MR.Context, SL, "16.5", "Function does not take any parameters but is not using the void keyword : ");
      }
    }
  }

private:
  Rewriter &Rewrite;
};
/**********************************************************************************************************************/
/*@DEVI-has false-positives*/
class MCPointer171 : public MatchFinder::MatchCallback
{
public:
  MCPointer171 (Rewriter &Rewrite) : Rewrite(Rewrite) {}

  virtual void run(const MatchFinder::MatchResult &MR)
  {
    if (MR.Nodes.getNodeAs<clang::DeclRefExpr>("mcpointer171") != nullptr)
    {
      const DeclRefExpr* DRE = MR.Nodes.getNodeAs<clang::DeclRefExpr>("mcpointer171") ;

      SourceLocation SL = DRE->getLocStart(); 
      CheckSLValidity(SL);
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

      if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
      {
        return void();
      }

      if (!Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
      {
        return void();
      }

      QualType QT = DRE->getType();

      const clang::Type* TP = QT.getTypePtr();

      if (TP->isAnyPointerType())
      {
        std::cout << "17.1:" << "Pointer arithmatic for non-array pointers:";
        std::cout << SL.printToString(*MR.SourceManager) << ":" << "\n";

        XMLDocOut.XMLAddNode(MR.Context, SL, "17.1", "Pointer arithmatic for non-array pointers : ");
        JSONDocOUT.JSONAddElement(MR.Context, SL, "17.1", "Pointer arithmatic for non-array pointers : ");

        if (mutagen)
        {
          ME.ExtractAncestry(ast_type_traits::DynTypedNode::create(*DRE), *MR.Context);
        }
      }
    }
  }

private:
  Rewriter &Rewrite;
};
/**********************************************************************************************************************/
/*@DEVI-has a lot of false positives. now works based on array types not the array itself.*/
class MCPointer1723 : public MatchFinder::MatchCallback
{
public:
  MCPointer1723 (Rewriter &Rewrite) : Rewrite(Rewrite) {}

  virtual void run(const MatchFinder::MatchResult &MR)
  {
    if (MR.Nodes.getNodeAs<clang::DeclRefExpr>("mcpointer1723rhs") != nullptr && MR.Nodes.getNodeAs<clang::DeclRefExpr>("mcpointer1723lhs"))
    {
      const DeclRefExpr* DRER = MR.Nodes.getNodeAs<clang::DeclRefExpr>("mcpointer1723rhs");
      const DeclRefExpr* DREL = MR.Nodes.getNodeAs<clang::DeclRefExpr>("mcpointer1723lhs");
      const BinaryOperator* BO = MR.Nodes.getNodeAs<clang::BinaryOperator>("mcpointer1723daddy");

      SourceLocation SL = BO->getLocStart(); 
      CheckSLValidity(SL);
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

      if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
      {
        return void();
      }

      if (!Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
      {
        return void();
      }

      QualType QTR = DRER->getType();
      QualType QTL = DREL->getType();

      const clang::Type* TPR = QTR.getTypePtr();
      const clang::Type* TPL = QTL.getTypePtr();

      if (TPR->getPointeeType() != TPL->getPointeeType())
      {
        std::cout << "17.2 | 17.3:" << "Pointer-type operands to BinaryOperator dont point to the same array:";
        std::cout << SL.printToString(*MR.SourceManager) << ":" << "\n";

        XMLDocOut.XMLAddNode(MR.Context, SL, "17.2 | 17.3", "Pointer-type operands to BinaryOperator dont point to the same array : ");
        JSONDocOUT.JSONAddElement(MR.Context, SL, "17.2 | 17.3", "Pointer-type operands to BinaryOperator dont point to the same array : ");

        /*@DEVI-FIXME-cant extract mutagen correctly*/
        if (mutagen)
        {
          ME.ExtractAncestry(ast_type_traits::DynTypedNode::create(*BO), *MR.Context);
        }
      }
    }
  }

private:
  Rewriter &Rewrite;
};
/**********************************************************************************************************************/
class MCPointer174 : public MatchFinder::MatchCallback
{
public:
  MCPointer174 (Rewriter &Rewrite) : Rewrite(Rewrite) {}

  virtual void run(const MatchFinder::MatchResult &MR)
  {
    if (MR.Nodes.getNodeAs<clang::CastExpr>("mcpointer174") != nullptr)
    {
      const CastExpr* CE = MR.Nodes.getNodeAs<clang::CastExpr>("mcpointer174");

      SourceLocation SL = CE->getLocStart(); 
      CheckSLValidity(SL);
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

      if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
      {
        /*intentionally left blank*/
      }
      else
      {
        if (Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
        {
          std::cout << "17.4:" << "The only allowed form of pointer arithmetic is array indexing:";
          std::cout << SL.printToString(*MR.SourceManager) << ":" << "\n";

          XMLDocOut.XMLAddNode(MR.Context, SL, "17.4", "The only allowed form of pointer arithmetic is array indexing : ");
          JSONDocOUT.JSONAddElement(MR.Context, SL, "17.4", "The only allowed form of pointer arithmetic is array indexing : ");

          if (mutagen)
          {
            ME.ExtractAncestry(ast_type_traits::DynTypedNode::create(*CE), *MR.Context);
          }
        }
      }
    }

    if (MR.Nodes.getNodeAs<clang::DeclRefExpr>("mcpointer1742") != nullptr)
    {
      const DeclRefExpr* DRE = MR.Nodes.getNodeAs<clang::DeclRefExpr>("mcpointer1742");

      SourceLocation SL = DRE->getLocStart(); 
      CheckSLValidity(SL);
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

      if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
      {
        /*intentionally left blank*/
      }
      else
      {
        if (Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
        {
          std::cout << "17.4:" << "The only allowed form of pointer arithmetic is array indexing:";
          std::cout << SL.printToString(*MR.SourceManager) << ":" << "\n";

          XMLDocOut.XMLAddNode(MR.Context, SL, "17.4", "The only allowed form of pointer arithmetic is array indexing : ");
          JSONDocOUT.JSONAddElement(MR.Context, SL, "17.4", "The only allowed form of pointer arithmetic is array indexing : ");

          if (mutagen)
          {
            ME.ExtractAncestry(ast_type_traits::DynTypedNode::create(*DRE), *MR.Context);
          }
        }
      }
    }
  }

private:
  Rewriter &Rewrite;
};
/**********************************************************************************************************************/
/*@DEVI-in case of function pointers, where an argument has more than two levels of indirection,
the argument and the function pointer both get tagged. technically, it is a defendable interpretation of the rule.*/
class MCPointer175 : public MatchFinder::MatchCallback
{
public:
  MCPointer175 (Rewriter &Rewrite) : Rewrite(Rewrite) {}

  virtual void run(const MatchFinder::MatchResult &MR)
  {
    const VarDecl* VD;
    const FieldDecl* FD;
    SourceLocation SL;
    QualType QT;

    if (MR.Nodes.getNodeAs<clang::VarDecl>("mcpointer175") != nullptr)
    {
      VD = MR.Nodes.getNodeAs<clang::VarDecl>("mcpointer175");

      SL = VD->getLocStart(); 
      CheckSLValidity(SL);
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

      QT = VD->getType();
    }

    if (MR.Nodes.getNodeAs<clang::FieldDecl>("mcpointer175field") != nullptr)
    {
      FD = MR.Nodes.getNodeAs<clang::FieldDecl>("mcpointer175field");

      SL = FD->getSourceRange().getBegin(); 
      CheckSLValidity(SL);
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

      QT = FD->getType();
    }


    QualType CQT = QT.getCanonicalType();

    std::string CQTAsString = CQT.getAsString();

    const clang::Type* TP [[maybe_unused]] = CQT.getTypePtr();

    unsigned starCounter = 0U;
    size_t StarPos = 0U;
    size_t OpenParens = 0U;
    size_t NextOpenParens = 0U;
    size_t CommaPos = 0U;
    size_t NextCommaPos = 0U;
    bool FoundAMatch [[maybe_unused]] = false;

    while (StarPos != std::string::npos)
    {
      StarPos = CQTAsString.find("*", StarPos + 1);
      OpenParens = CQTAsString.find("(", NextOpenParens + 1);
      CommaPos = CQTAsString.find(",", NextCommaPos + 1);

      if (OpenParens != std::string::npos)
      {
        if (StarPos > OpenParens)
        {
          starCounter = 0U;
          NextOpenParens = OpenParens;
        }

      }

      if (CommaPos != std::string::npos)
      {
        if (StarPos > CommaPos)
        {
          starCounter = 0U;
          NextCommaPos = CommaPos;
        }
      }

      if (StarPos != std::string::npos)
      {
        starCounter++;
      }

      if (starCounter >= 3U)
      {
        if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
        {
          /*intentionally left blank*/
        }
        else
        {
          if (Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
          {
            std::cout << "17.5:" << "Pointer has more than 2 levels of indirection:";
            std::cout << SL.printToString(*MR.SourceManager) << ":" << "\n";

            XMLDocOut.XMLAddNode(MR.Context, SL, "17.5", "Pointer has more than 2 levels on indirection : ");
            JSONDocOUT.JSONAddElement(MR.Context, SL, "17.5", "Pointer has more than 2 levels on indirection : ");
          }
        }

        break;
      }
    }

  }

private:
  Rewriter &Rewrite;
};
/**********************************************************************************************************************/
/*@DEVI-the simple char type can be singed or unsigned. its implementation-defined. the values appear
folded on AST and currently i dont know of a method to differentiate between singed/unsigned char and simple char.
sure, we can get the text of the vardecl and do string search in there but that still does not solve our problem.
we could do string search for the RHS expression but thats limited and then there is flagging cases that are of char
type because of a macro expansion(the macro adding signed or unsinged to the char type). we could flag those macros
in a PPCallback::MacroDefined but that leaves us with the problem of CStyleCasts. for example when a
simple char type is assigned a numeric values cast explicitly cast to simple char, misra-c says it does not
break rule 6.1(see https://www.misra.org.uk/forum/viewtopic.php?t=1020 for reference). the bottom line is,
there is a way to implement this but the implementation will be janky and its too much trouble for a janky
implementation that later on will not be modifiable much.*/
class MCTypes61 : public MatchFinder::MatchCallback
{
public:
  MCTypes61 (Rewriter &Rewrite) : Rewrite(Rewrite) {}

  virtual void run(const MatchFinder::MatchResult &MR)
  {
    if ((MR.Nodes.getNodeAs<clang::Expr>("mctypes6rhs") != nullptr) \
        && (MR.Nodes.getNodeAs<clang::VarDecl>("mctypes6origin") != nullptr) \
        && (MR.Nodes.getNodeAs<clang::BinaryOperator>("mctypes6dous") != nullptr))
    {
      const Expr* EXP = MR.Nodes.getNodeAs<clang::Expr>("mctypes6rhs");
      const VarDecl* VD = MR.Nodes.getNodeAs<clang::VarDecl>("mctypes6origin");

      QualType QT = VD->getType();
      const clang::Type* TP = QT.getTypePtr();

      QualType QTEXP = EXP->getType();
      const clang::Type* TPEXP = QTEXP.getTypePtr();

      SourceLocation SL = EXP->getLocStart(); 
      CheckSLValidity(SL);
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");
      SourceLocation SLE = EXP->getLocEnd();
      SLE = Devi::SourceLocationHasMacro(SLE, Rewrite, "start");

      SourceRange SR;
      SR.setBegin(SL);
      SR.setEnd(SLE);

#if 0
      std::string RHSString = Rewrite.getRewrittenText(SR);

      //std::cout << RHSString << "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << "\n";

      size_t singleQuoteLoc = RHSString.find("'", 0U);
      size_t doubleQuoteLoc = RHSString.find("\"", 0U);
      size_t singleQuoteLocE = RHSString.rfind("'", 0U);
      size_t doubleQuoteLocE = RHSString.rfind("\"", 0U);
#endif

      /*@DEVI-the logic here is that we know we have matched a chartype. if its not either a singedinteger or
      unsingedinteger, then it is a simple char. otherwise it is signed or unsigned char.*/
#if 1
      if (TP->isSignedIntegerType() || TP->isUnsignedIntegerType())
      {
        //std::cout << RHSString << "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << SL.printToString(*MR.SourceManager) << "\n";

        if (!TPEXP->isSignedIntegerType() && !TPEXP->isUnsignedIntegerType())
        {
          if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
          {
            /*intentionally left blank*/
          }
          else
          {
            if (Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
            {
              std::cout << "6.2:" << "Sgined or unsigned char type holds characterLiterals:";
              std::cout << SL.printToString(*MR.SourceManager) << ":" << "\n";

              XMLDocOut.XMLAddNode(MR.Context, SL, "6.2", "Sgined or unsigned char type holds characterLiterals : ");
              JSONDocOUT.JSONAddElement(MR.Context, SL, "6.2", "Sgined or unsigned char type holds characterLiterals : ");
            }
          }
        }
      }
      else
      {

      }

      if (!TP->isSignedIntegerType() && !TP->isUnsignedIntegerType())
      {
        //std::cout << RHSString << "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << SL.printToString(*MR.SourceManager) << "\n";

        if (TPEXP->isSignedIntegerType() || TPEXP->isUnsignedIntegerType())
        {
          if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
          {
            /*intentionally left blank*/
          }
          else
          {
            if (Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
            {
              std::cout << "6.1:" << "Simple char type holds numeric values:";
              std::cout << SL.printToString(*MR.SourceManager) << ":" << "\n";

              XMLDocOut.XMLAddNode(MR.Context, SL, "6.1", "Simple char type holds numeric values : ");
              JSONDocOUT.JSONAddElement(MR.Context, SL, "6.1", "Simple char type holds numeric values : ");
            }
          }
        }
      }
#endif
    }
  }

private:
  Rewriter &Rewrite;
};
/**********************************************************************************************************************/
class MCSU181 : public MatchFinder::MatchCallback
{
public:
  MCSU181 (Rewriter &Rewrite) : Rewrite(Rewrite) {}

  virtual void run(const MatchFinder::MatchResult &MR)
  {
    if (MR.Nodes.getNodeAs<clang::VarDecl>("mcsu181arr") != nullptr)
    {
      const VarDecl* VD = MR.Nodes.getNodeAs<clang::VarDecl>("mcsu181arr");

      SourceLocation SL = VD->getLocStart(); 
      CheckSLValidity(SL);
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

      if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
      {
        return void();
      }

      if (!Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
      {
        return void();
      }

      std::cout << "18.1:" << "ArrayType incomplete at the end of the translation unit:";
      std::cout << SL.printToString(*MR.SourceManager) << ":" << "\n";

      XMLDocOut.XMLAddNode(MR.Context, SL, "18.1", "ArrayType incomplete at the end of the translation unit : ");
      JSONDocOUT.JSONAddElement(MR.Context, SL, "18.1", "ArrayType incomplete at the end of the translation unit : ");
    }
  }

private:
  Rewriter &Rewrite;
};
/**********************************************************************************************************************/
class MCPTC11CSTYLE : public MatchFinder::MatchCallback
{
public:
  MCPTC11CSTYLE (Rewriter &Rewrite) : Rewrite(Rewrite) {}

  virtual void run(const MatchFinder::MatchResult &MR)
  {
    if (MR.Nodes.getNodeAs<clang::CStyleCastExpr>("mcptc11cstyle") != nullptr)
    {
      const CStyleCastExpr* CSCE = MR.Nodes.getNodeAs<clang::CStyleCastExpr>("mcptc11cstyle");

      SourceLocation SL = CSCE->getLocStart(); 
      CheckSLValidity(SL);
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

      QualType QT = CSCE->getType();

      const clang::Type* TP = QT.getTypePtr();

      ASTContext *const ASTC [[maybe_unused]] = MR.Context;

#if 0
      const ASTContext::DynTypedNodeList NodeList = ASTC->getParents(*CSCE);

      /*assumptions:implicitcastexpr does not have more than one parent in C.*/
      const ast_type_traits::DynTypedNode ParentNode = NodeList[0];

      ast_type_traits::ASTNodeKind ParentNodeKind = ParentNode.getNodeKind();

      std::string StringKind = ParentNodeKind.asStringRef().str();
#endif

      CastKind CK = CSCE->getCastKind();

      bool ShouldBeTagged11 = false;

      if (TP->isFunctionPointerType())
      {
        if (((CK != CK_IntegralToPointer) && (CK != CK_PointerToIntegral) && \
             (CK != CK_LValueToRValue) && (CK != CK_FunctionToPointerDecay) && \
             (CK != CK_ArrayToPointerDecay)))
        {
          ShouldBeTagged11 = true;
        }

        if (CK == CK_BitCast)
        {
          ShouldBeTagged11 = true;
        }

        if (ShouldBeTagged11)
        {
          if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
          {
            /*intentionally left blank*/
          }
          else
          {
            if (Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
            {
              std::cout << "11.1:" << "CStyleCastExpr - FunctionPointerType converted to or from a type other than IntegralType:";
              std::cout << SL.printToString(*MR.SourceManager) << ":" << "\n";

              XMLDocOut.XMLAddNode(MR.Context, SL, "11.1", "CStyleCastExpr - FunctionPointerType converted to or from a type other than IntegralType: ");
              JSONDocOUT.JSONAddElement(MR.Context, SL, "11.1", "CStyleCastExpr - FunctionPointerType converted to or from a type other than IntegralType: ");

              if (mutagen)
              {
                ME.ExtractAncestry(ast_type_traits::DynTypedNode::create(*CSCE), *MR.Context);
              }
            }
          }
        }
      }

      if ((CK == CK_IntegralToPointer) || (CK == CK_PointerToIntegral))
      {
        if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
        {
          /*intentionally left blank*/
        }
        else
        {
          if (Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
          {
            std::cout << "11.3:" << "CStyleCastExpr - Conversion of PointerType to or from IntegralType is recommended against:";
            std::cout << SL.printToString(*MR.SourceManager) << ":" << "\n";

            XMLDocOut.XMLAddNode(MR.Context, SL, "11.3", "CStyleCastExpr - Conversion of PointerType to or from IntegralType is recommended against: ");
            JSONDocOUT.JSONAddElement(MR.Context, SL, "11.3", "CStyleCastExpr - Conversion of PointerType to or from IntegralType is recommended against: ");

            if (mutagen)
            {
              ME.ExtractAncestry(ast_type_traits::DynTypedNode::create(*CSCE), *MR.Context);
            }
          }
        }
      }

      if (CK == CK_BitCast || CK == CK_PointerToBoolean || CK == CK_AnyPointerToBlockPointerCast)
      {
        if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
        {
          /*intentionally left blank*/
        }
        else
        {
          if (Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
          {
            std::cout << "11.x:" << "CStyleCastExpr - PointerType has implicit BitCast. This could be caused by a cast removing const or volatile qualifier from the type addressed by a pointer or by a cast to a different function or object type:";
            std::cout << SL.printToString(*MR.SourceManager) << ":" << "\n";

            XMLDocOut.XMLAddNode(MR.Context, SL, "11.x", "CStyleCastExpr - PointerType has implicit BitCast. This could be caused by a cast removing const or volatile qualifier from the type addressed by a pointer or by a cast to a different function or object type: ");
            JSONDocOUT.JSONAddElement(MR.Context, SL, "11.x", "CStyleCastExpr - PointerType has implicit BitCast. This could be caused by a cast removing const or volatile qualifier from the type addressed by a pointer or by a cast to a different function or object type: ");

            if (mutagen)
            {
              ME.ExtractAncestry(ast_type_traits::DynTypedNode::create(*CSCE), *MR.Context);
            }
          }
        }
      }
    }
  }

private:
  Rewriter &Rewrite;
};
/**********************************************************************************************************************/
class MCATC101 : public MatchFinder::MatchCallback
{
public:
  MCATC101 (Rewriter &Rewrite) : Rewrite(Rewrite) {}

  virtual void run(const MatchFinder::MatchResult &MR)
  {
    if (MR.Nodes.getNodeAs<clang::ImplicitCastExpr>("atcdaddy") != nullptr)
    {
      const ImplicitCastExpr* ICE = MR.Nodes.getNodeAs<clang::ImplicitCastExpr>("atcdaddy");

      if ((ICE->getCastKind() == CK_IntegralCast) || (ICE->getCastKind() == CK_FloatingCast) || \
          (ICE->getCastKind() == CK_FloatingComplexCast) || (ICE->getCastKind() == CK_IntegralComplexCast))
      {
        SourceLocation SL = ICE->getLocStart(); 
        CheckSLValidity(SL);
        SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

        ASTContext *const ASTC = MR.Context;

        QualType QTDaddy = ICE->getType();
        QualType QTKiddy;

        if (MR.Nodes.getNodeAs<clang::BinaryOperator>("atcdous") != nullptr)
        {
          const BinaryOperator* ChildNode = MR.Nodes.getNodeAs<clang::BinaryOperator>("atcdous");
          QTKiddy = ChildNode->getType();
        }

        if (MR.Nodes.getNodeAs<clang::UnaryOperator>("atcuno") != nullptr)
        {
          const UnaryOperator* ChildNode = MR.Nodes.getNodeAs<clang::UnaryOperator>("atcuno");
          QTKiddy = ChildNode->getType();
        }

        if (MR.Nodes.getNodeAs<clang::ParenExpr>("atcparens") != nullptr)
        {
          const ParenExpr* ChildNode = MR.Nodes.getNodeAs<clang::ParenExpr>("atcparens");
          QTKiddy = ChildNode->getType();
        }

        if (MR.Nodes.getNodeAs<clang::ImplicitCastExpr>("atckidice") != nullptr)
        {
          const ImplicitCastExpr* ChildNode = MR.Nodes.getNodeAs<clang::ImplicitCastExpr>("atckidice");
          QTKiddy = ChildNode->getType();
        }

        if (MR.Nodes.getNodeAs<clang::CStyleCastExpr>("atccstyle") != nullptr)
        {
          const CStyleCastExpr* ChildNode = MR.Nodes.getNodeAs<clang::CStyleCastExpr>("atccstyle");
          QTKiddy = ChildNode->getType();
        }

        const clang::Type* TPDaddy = QTDaddy.getTypePtr();
        const clang::Type* TPChild = QTKiddy.getTypePtr();

        const clang::Type* CanonTypeDaddy = ASTC->getCanonicalType(TPDaddy);
        const clang::Type* CanonTypeChild = ASTC->getCanonicalType(TPChild);

        uint64_t ICETypeSize = ASTC->getTypeSize(CanonTypeDaddy);
        uint64_t ChildTypeSize = ASTC->getTypeSize(CanonTypeChild);

        bool ICETypeIsSignedInt = CanonTypeDaddy->isSpecificBuiltinType(BuiltinType::Kind::Long) || \
                                  CanonTypeDaddy->isSpecificBuiltinType(BuiltinType::Kind::Int) || \
                                  CanonTypeDaddy->isSpecificBuiltinType(BuiltinType::Kind::Short) || \
                                  CanonTypeDaddy->isSpecificBuiltinType(BuiltinType::Kind::SChar) || \
                                  CanonTypeDaddy->isSpecificBuiltinType(BuiltinType::Kind::Char_S) || \
                                  CanonTypeDaddy->isSpecificBuiltinType(BuiltinType::Kind::LongLong) || \
                                  CanonTypeDaddy->isSpecificBuiltinType(BuiltinType::Kind::Int128) || \
                                  CanonTypeDaddy->isSpecificBuiltinType(BuiltinType::Kind::WChar_S);
        bool ChildTypeIsSignedInt = CanonTypeChild->isSpecificBuiltinType(BuiltinType::Kind::Long) || \
                                    CanonTypeDaddy->isSpecificBuiltinType(BuiltinType::Kind::Int) || \
                                    CanonTypeDaddy->isSpecificBuiltinType(BuiltinType::Kind::Short) || \
                                    CanonTypeDaddy->isSpecificBuiltinType(BuiltinType::Kind::SChar) || \
                                    CanonTypeDaddy->isSpecificBuiltinType(BuiltinType::Kind::Char_S) || \
                                    CanonTypeDaddy->isSpecificBuiltinType(BuiltinType::Kind::LongLong) || \
                                    CanonTypeDaddy->isSpecificBuiltinType(BuiltinType::Kind::Int128) || \
                                    CanonTypeDaddy->isSpecificBuiltinType(BuiltinType::Kind::WChar_S);

        bool ICETypeIsUSignedInt = CanonTypeDaddy->isSpecificBuiltinType(BuiltinType::Kind::ULong) || \
                                    CanonTypeDaddy->isSpecificBuiltinType(BuiltinType::Kind::UInt) || \
                                    CanonTypeDaddy->isSpecificBuiltinType(BuiltinType::Kind::UShort) || \
                                    CanonTypeDaddy->isSpecificBuiltinType(BuiltinType::Kind::UChar) || \
                                    CanonTypeDaddy->isSpecificBuiltinType(BuiltinType::Kind::Char_U) || \
                                    CanonTypeDaddy->isSpecificBuiltinType(BuiltinType::Kind::ULongLong) || \
                                    CanonTypeDaddy->isSpecificBuiltinType(BuiltinType::Kind::UInt128) || \
                                    CanonTypeDaddy->isSpecificBuiltinType(BuiltinType::Kind::WChar_U);
        bool ChildTypeIsUSignedInt = CanonTypeChild->isSpecificBuiltinType(BuiltinType::Kind::ULong) || \
                                    CanonTypeDaddy->isSpecificBuiltinType(BuiltinType::Kind::UInt) || \
                                    CanonTypeDaddy->isSpecificBuiltinType(BuiltinType::Kind::UShort) || \
                                    CanonTypeDaddy->isSpecificBuiltinType(BuiltinType::Kind::UChar) || \
                                    CanonTypeDaddy->isSpecificBuiltinType(BuiltinType::Kind::Char_U) || \
                                    CanonTypeDaddy->isSpecificBuiltinType(BuiltinType::Kind::ULongLong) || \
                                    CanonTypeDaddy->isSpecificBuiltinType(BuiltinType::Kind::UInt128) || \
                                    CanonTypeDaddy->isSpecificBuiltinType(BuiltinType::Kind::WChar_U);

        bool ICETypeIsInteger = ICETypeIsSignedInt || ICETypeIsUSignedInt;
        bool ChildTypeIsInteger = ChildTypeIsSignedInt || ChildTypeIsUSignedInt;

        if (ICETypeIsInteger && ChildTypeIsInteger)
        {
          if ((ICETypeIsSignedInt && ChildTypeIsUSignedInt) || (ICETypeIsUSignedInt && ChildTypeIsSignedInt))
          {
            if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
            {
              /*intentionally left blank*/
            }
            else
            {
              if (Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
              {
                std::cout << "10.1/2:" << "ImplicitCastExpr changes the signedness of the type:";
                std::cout << SL.printToString(*MR.SourceManager) << ":" << "\n";

                XMLDocOut.XMLAddNode(MR.Context, SL, "10.1/2", "ImplicitCastExpr changes the signedness of the type: ");
                JSONDocOUT.JSONAddElement(MR.Context, SL, "10.1/2", "ImplicitCastExpr changes the signedness of the type: ");
              }
            }
          }
        }

        if (ICETypeSize < ChildTypeSize && !(CanonTypeChild->isComplexIntegerType() || CanonTypeChild->isComplexType()))
        {
          if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
          {
            /*intentionally left blank*/
          }
          else
          {
            if (Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
            {
              std::cout << "10.1/2:" << "ImplicitCastExpr is narrowing:";
              std::cout << SL.printToString(*MR.SourceManager) << ":" << "\n";

              XMLDocOut.XMLAddNode(MR.Context, SL, "10.1/2", "ImplicitCastExpr is narrowing: ");
              JSONDocOUT.JSONAddElement(MR.Context, SL, "10.1/2", "ImplicitCastExpr is narrowing: ");
            }
          }
        }

        if (CanonTypeChild->isComplexIntegerType())
        {
          if (ICETypeSize > ChildTypeSize)
          {
            if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
            {
              /*intentionally left blank*/
            }
            else
            {
              if (Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
              {
                std::cout << "10.3:" << "ImplicitCastExpr is widening for complex integer type:";
                std::cout << SL.printToString(*MR.SourceManager) << ":" << "\n";

                XMLDocOut.XMLAddNode(MR.Context, SL, "10.3", "ImplicitCastExpr is widening for complex integer type: ");
                JSONDocOUT.JSONAddElement(MR.Context, SL, "10.3", "ImplicitCastExpr is widening for complex integer type: ");
              }
            }
          }

          const ComplexType* ChildCPXType = CanonTypeChild->getAsComplexIntegerType();
          const ComplexType* DaddyCPXType = CanonTypeDaddy->getAsComplexIntegerType();

          QualType DaddyCPXQT = DaddyCPXType->getElementType();
          QualType ChildCPXQT = ChildCPXType->getElementType();

          const clang::Type* DaddyCPXElementType = DaddyCPXQT.getTypePtr();
          const clang::Type * ChildCPXElementType = ChildCPXQT.getTypePtr();

          /*
          bool IsSignedCPXDaddy = DaddyCPXElementType->getAsPlaceholderType()->isSignedInteger();
          bool IsSignedCPXChild = ChildCPXElementType->getAsPlaceholderType()->isSignedInteger();
          bool IsUnsignedCPXDaddy = DaddyCPXElementType->getAsPlaceholderType()->isUnsignedInteger();
          bool IsUnsignedCPXChild = ChildCPXElementType->getAsPlaceholderType()->isUnsignedInteger();
          */

          bool IsSignedCPXDaddy = false;
          bool IsUnsignedCPXDaddy = false;
          if (DaddyCPXElementType) {
          	auto placeholderType = DaddyCPXElementType->getAsPlaceholderType();
          	if (placeholderType) {
          		IsSignedCPXDaddy = placeholderType->isSignedInteger();
          		IsUnsignedCPXDaddy = placeholderType->isUnsignedInteger();
          	}
          }

          bool IsSignedCPXChild = false;
          bool IsUnsignedCPXChild = false;
          if (ChildCPXElementType) {
          	auto placeholderType = ChildCPXElementType->getAsPlaceholderType();
          	if (placeholderType) {
          		IsSignedCPXChild = placeholderType->isSignedInteger();
          		IsUnsignedCPXChild = placeholderType->isUnsignedInteger();
          	}
          }

          if ((IsSignedCPXDaddy && IsUnsignedCPXChild) || (IsUnsignedCPXDaddy && IsSignedCPXChild))
          {
            if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
            {
              /*intentionally left blank*/
            }
            else
            {
              if (Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
              {
                std::cout << "10.3:" << "ImplicitCastExpr changes the signedness of the complex integer type:";
                std::cout << SL.printToString(*MR.SourceManager) << ":" << "\n";

                XMLDocOut.XMLAddNode(MR.Context, SL, "10.3", "ImplicitCastExpr changes the signedness of the complex integer type: ");
                JSONDocOUT.JSONAddElement(MR.Context, SL, "10.3", "ImplicitCastExpr changes the signedness of the complex integer type type: ");
              }
            }
          }
        }

        /*clang::Type::iSComplexIntegerType will not return true for the gnu extension of complex integers.*/
        if (!CanonTypeChild->isComplexIntegerType() && CanonTypeChild->isComplexType())
        {
          if (ICETypeSize > ChildTypeSize)
          {
            if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
            {
              /*intentionally left blank*/
            }
            else
            {
              if (Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
              {
                std::cout << "10.4:" << "ImplicitCastExpr is widening for complex float type:";
                std::cout << SL.printToString(*MR.SourceManager) << ":" << "\n";

                XMLDocOut.XMLAddNode(MR.Context, SL, "10.4", "ImplicitCastExpr is widening for complex float type: ");
                JSONDocOUT.JSONAddElement(MR.Context, SL, "10.4", "ImplicitCastExpr is widening for complex float type: ");
              }
            }
          }
        }
      }
    }
  }

private:
  Rewriter &Rewrite;
};
/**********************************************************************************************************************/
class MCIdent51 : public MatchFinder::MatchCallback
{
public:
  MCIdent51 (Rewriter &Rewrite) : Rewrite(Rewrite) {}

  virtual void run(const MatchFinder::MatchResult &MR)
  {
    if (MR.Nodes.getNodeAs<clang::NamedDecl>("ident5nameddecl") != nullptr)
    {
      const NamedDecl* ND = MR.Nodes.getNodeAs<clang::NamedDecl>("ident5nameddecl");

      const IdentifierInfo *II = ND->getIdentifier();

      SourceLocation SL = ND->getLocStart(); 
      CheckSLValidity(SL);
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

      ASTContext *const ASTC = MR.Context;

      const IdentifierTable &IT = ASTC->Idents;

      if (II != nullptr)
      {
        StringRef IdentStringRef = II->getName();

        for (auto &iter : IT)
        {
          /*@DEVI-only works for UTF-8. for larger sizes we need a multiple of 32. for UTF-16 we need to check against 64 and so on.*/
          if (IdentStringRef.str().size() >= 32U)
          {
            if ((iter.getValue()->getName().str().substr(0U, 32U) == IdentStringRef.str().substr(0U, 32U)) && (iter.getValue()->getName().str() != IdentStringRef.str()))
            {
              if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
              {
                /*intentionally left blank*/
              }
              else
              {
                if (Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
                {
                  std::cout << "5.1:" << "Identifier relies on the signifacance of more than 31 charcaters:";
                  std::cout << SL.printToString(*MR.SourceManager) << ":" << "\n";

                  XMLDocOut.XMLAddNode(MR.Context, SL, "5.1", "Identifier relies on the significance of more than 31 charcaters: ");
                  JSONDocOUT.JSONAddElement(MR.Context, SL, "5.1", "Identifier relies on the significance of more than 31 charcaters: ");
                }
              }
            }
          }
        }
      }
    }
  }

private:
  Rewriter &Rewrite;
};
/**********************************************************************************************************************/
class MCDCDF87 : public MatchFinder::MatchCallback
{
public:
  MCDCDF87 (Rewriter &Rewrite) : Rewrite(Rewrite)
  {
    IsNewEntry = true;
  }

  virtual void run(const MatchFinder::MatchResult &MR)
  {
    if ((MR.Nodes.getNodeAs<clang::DeclRefExpr>("mcdcdfobj") != nullptr) \
        && (MR.Nodes.getNodeAs<clang::FunctionDecl>("mcdcdf87daddy") != nullptr) && \
        (MR.Nodes.getNodeAs<clang::VarDecl>("mcdcdf87origin") != nullptr))
    {
      IsNewEntry = true;

      const FunctionDecl* FD = MR.Nodes.getNodeAs<clang::FunctionDecl>("mcdcdf87daddy");
      const VarDecl* VD = MR.Nodes.getNodeAs<clang::VarDecl>("mcdcdf87origin");

      std::string VDName = VD->getIdentifier()->getName().str();

      SourceLocation SL = VD->getLocStart(); 
      CheckSLValidity(SL);
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

      ASTContext* const ASTC = MR.Context;

      for (auto &iter : MaybeLocalObjInfoProto)
      {
        if (iter.ObjNameStr == VDName && iter.ObjSL == SL)
        {
          IsNewEntry = false;

          if ((iter.FirstDaddyName != FD->getNameInfo().getAsString()))
          {
            iter.HasMoreThanOneDaddy = true;
          }
        }
      }

      if (IsNewEntry)
      {
        MaybeLocalObjInfo Temp = {SL, ASTC->getFullLoc(SL), SL.printToString(*MR.SourceManager), VDName, FD->getNameInfo().getAsString(), false};
        MaybeLocalObjInfoProto.push_back(Temp);
      }
    }
  }

  virtual void onEndOfTranslationUnit()
  {
    for (auto &iter : MaybeLocalObjInfoProto)
    {
      if (!iter.HasMoreThanOneDaddy)
      {
        if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, iter.ObjFSL.isInSystemHeader(), iter.ObjSL))
        {
          /*intentionally left blank*/
        }
        else
        {
          if (Devi::IsTheMatchInMainFile(MainFileOnly, iter.ObjFSL.getManager().isInMainFile(iter.ObjSL), iter.ObjSL))
          {
            std::cout << "8.7:" << "Object (" + iter.ObjNameStr + ") is only being used in one block (" + iter.FirstDaddyName + ") but is not defined inside that block:";
            std::cout << iter.ObjSLStr << ":" << "\n";

            XMLDocOut.XMLAddNode(iter.ObjFSL, iter.ObjSL, "8.7", \
                                 "Object (" + iter.ObjNameStr + ") is only being used in one block (" + iter.FirstDaddyName + ") but is not defined inside that block: ");
            JSONDocOUT.JSONAddElement(iter.ObjFSL, iter.ObjSL, "8.7", \
                                      "Object (" + iter.ObjNameStr + ") is only being used in one block (" + iter.FirstDaddyName + ") but is not defined inside that block: ");
          }
        }
      }
    }
  }


private:
  struct MaybeLocalObjInfo
  {
    MaybeLocalObjInfo(SourceLocation iObjSL, FullSourceLoc iObjFSL, std::string iObjSLStr, \
                      std::string iObjNameStr, std::string iFirstDaddyName, bool iHasMoreThanOneDaddy = false)
    {
      ObjSL = iObjSL;
      ObjFSL = iObjFSL;
      ObjSLStr = iObjSLStr;
      ObjNameStr = iObjNameStr;
      FirstDaddyName = iFirstDaddyName;
      HasMoreThanOneDaddy = iHasMoreThanOneDaddy;
    }

    SourceLocation ObjSL;
    FullSourceLoc ObjFSL;
    std::string ObjSLStr;
    std::string ObjNameStr;
    std::string FirstDaddyName;
    bool HasMoreThanOneDaddy = false;
  };

  bool IsNewEntry;

  std::vector<MaybeLocalObjInfo> MaybeLocalObjInfoProto;

  Rewriter &Rewrite;
};
/**********************************************************************************************************************/
/*@DEVI-has false positives will tag incomplete types if they are later declared as complete types.*/
class [[maybe_unused]] MCDCDF88 : public MatchFinder::MatchCallback
{
public:
  MCDCDF88 (Rewriter &Rewrite) : Rewrite(Rewrite) {}

  virtual void run(const MatchFinder::MatchResult &MR)
  {
    bool IsNewEntry = true;

    if (MR.Nodes.getNodeAs<clang::VarDecl>("mcdcdf88var") != nullptr)
    {
      const VarDecl* VD = MR.Nodes.getNodeAs<clang::VarDecl>("mcdcdf88var");

      SourceLocation SL = VD->getLocStart(); 
      CheckSLValidity(SL);
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

      ASTContext* const ASTC = MR.Context;

      FullSourceLoc FSL = ASTC->getFullLoc(SL);

      const SourceManager &SM = FSL.getManager();

      if (!SM.isInMainFile(SL))
      {
        return void();
      }

      std::string NDNameString = VD->getNameAsString();

      for (auto &iter : ExternObjInfoProto)
      {
#if 0
        std::cout << "diagnostic2:" << "Variable:" << NDNameString << ":" << iter.XObjNameStr << "\n";
#endif
        if (iter.XObjNameStr == NDNameString)
        {
          IsNewEntry = false;

          iter.HasMoreThanOneDefinition = true;
        }
      }

      if (IsNewEntry)
      {
        const SourceManager &SM = FSL.getManager();

        ExternObjInfo Temp = {FSL.getSpellingLineNumber(), FSL.getSpellingColumnNumber(), \
                              SM.getFilename(SL), SL.printToString(*MR.SourceManager), NDNameString, \
                              FSL.getFileID(), false, false, false
                             };
        ExternObjInfoProto.push_back(Temp);
      }
    }

    if (MR.Nodes.getNodeAs<clang::FunctionDecl>("mcdcdf88function") != nullptr)
    {
      const FunctionDecl* FD = MR.Nodes.getNodeAs<clang::FunctionDecl>("mcdcdf88function");

      SourceLocation SL = FD->getLocStart(); 
      CheckSLValidity(SL);
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

      ASTContext* const ASTC = MR.Context;

      std::string NDNameString = FD->getNameAsString();

      FullSourceLoc FSL = ASTC->getFullLoc(SL);

      const SourceManager &SM = FSL.getManager();

      if (!SM.isInMainFile(SL))
      {
        return void();
      }

      for (auto &iter : ExternObjInfoProto)
      {
        if (iter.XObjNameStr == NDNameString)
        {
          IsNewEntry = false;


          if ((iter.IsDefinition == true && FD->isThisDeclarationADefinition()) || (iter.IsDeclaration == true && !FD->isThisDeclarationADefinition()))
          {
            iter.HasMoreThanOneDefinition = true;

            if (FD->isThisDeclarationADefinition())
            {
              iter.IsDefinition = true;
            }
            else
            {
              iter.IsDeclaration = true;
            }
          }

        }
      }

      if (IsNewEntry)
      {
        ExternObjInfo Temp = {FSL.getSpellingLineNumber(), FSL.getSpellingColumnNumber(), \
                              SM.getFilename(SL), SL.printToString(*MR.SourceManager), NDNameString, \
                              FSL.getFileID(), false, FD->isThisDeclarationADefinition(), !FD->isThisDeclarationADefinition()
                             };
        ExternObjInfoProto.push_back(Temp);
      }
    }
  }

private:
  /*@DEVI-the structure that holds the values is global since we need it to survive through all the TUs.*/

  Rewriter &Rewrite;
};
/**********************************************************************************************************************/
/*@DEVI-ASTContext doesn not have all the comments in a source file. i dunno why.*/
class MCLangX23 : public MatchFinder::MatchCallback
{
public:
  MCLangX23 (Rewriter &Rewrite) : Rewrite(Rewrite) {}

  virtual void run(const MatchFinder::MatchResult &MR)
  {
    if (MR.Nodes.getNodeAs<clang::Expr>("mclangx23") != nullptr)
    {
      ASTContext *const ASTC = MR.Context;

      const SourceManager &SM = ASTC->getSourceManager();

      RawCommentList RCL = ASTC->Comments;

      ArrayRef<RawComment*> RawCommentArrRef = RCL.getComments();

      std::string RawText;

      size_t matchLoc;

      unsigned currentLoc = 1U;

      unsigned MatchCounter = 0U;

      for (auto &iter : RawCommentArrRef)
      {
        RawText = iter->getRawText(SM);

        SourceLocation RCSL = iter->getLocStart(); 
        CheckSLValidity(RCSL);
        RCSL = Devi::SourceLocationHasMacro(RCSL, Rewrite, "start");

        while (true)
        {
          matchLoc = RawText.find("/*", currentLoc);

          if (matchLoc != std::string::npos)
          {
            currentLoc = matchLoc + 1U;

            MatchCounter++;
          }
          else
          {
            break;
          }
        }

        currentLoc = 1U;

        if (!once)
        {
          if (MatchCounter >= 1U)
          {
            if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, RCSL))
            {
              /*intentionally left blank*/
            }
            else
            {
              if (Devi::IsTheMatchInMainFile(MainFileOnly, MR, RCSL))
              {
                std::cout << "2.3:" << "character sequence \"/*\" used inside the comment:";
                std::cout << RCSL.printToString(*MR.SourceManager) << ":" << "\n";

                XMLDocOut.XMLAddNode(MR.Context, RCSL, "2.3", "character sequence \"/*\" used inside the comment : ");
                JSONDocOUT.JSONAddElement(MR.Context, RCSL, "2.3", "character sequence \"/*\" used inside the comment : ");
              }
            }
          }
        }

        MatchCounter = 0U;
      }

      once = true;

    }
  }

private:
  bool once = false;
  Rewriter &Rewrite [[maybe_unused]];
};
/**********************************************************************************************************************/
/*@DEVI-changes done to the pointee through unaryOperators ++ and -- will not be tagged by this class.
see implementation notes for the explanation.*/
class MCFunction167 : public MatchFinder::MatchCallback
{
public:
  MCFunction167 (Rewriter &Rewrite) : Rewrite(Rewrite) {}

  virtual void run(const MatchFinder::MatchResult &MR)
  {
    if (MR.Nodes.getNodeAs<clang::ParmVarDecl>("mcfunction167") != nullptr)
    {
      const ParmVarDecl* PVD = MR.Nodes.getNodeAs<clang::ParmVarDecl>("mcfunction167");

      SourceLocation SL = PVD->getLocStart(); 
      CheckSLValidity(SL);
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

      if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
      {
        return void();
      }

      QualType QT = PVD->getOriginalType();

      ASTContext *const ASTC [[maybe_unused]] = MR.Context;

      if (!QT.isConstQualified())
      {
        if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
        {
          /*intentionally left blank*/
        }
        else
        {
          if (Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
          {
            if (Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
            {
              std::cout << "16.7:" << "pointerType ParmVarDecl is not used to change the contents of the object it points to but is not declared as const:";
              std::cout << SL.printToString(*MR.SourceManager) << ":" << "\n";

              XMLDocOut.XMLAddNode(MR.Context, SL, "16.7", "pointerType ParmVarDecl is not used to change the contents of the object it points to but is not declared as const : ");
              JSONDocOUT.JSONAddElement(MR.Context, SL, "16.7", "pointerType ParmVarDecl is not used to change the contents of the object it points to but is not declared as const : ");
            }
          }
        }
      }
    }
  }

private:
  Rewriter &Rewrite [[maybe_unused]];
};
/**********************************************************************************************************************/
/*@DEVI-the match is quite simplistic. we could match for chartypes appearing as the LHS and then check the type of
the RHS expr but that leaves pointers changing the value.*/
class MCTypes612 : public MatchFinder::MatchCallback
{
public:
  MCTypes612 (Rewriter &Rewrite) : Rewrite(Rewrite) {}

  virtual void run(const MatchFinder::MatchResult &MR)
  {
    if (MR.Nodes.getNodeAs<clang::Expr>("mc612exp") != nullptr)
    {
      bool RHSIsCharLit = false;
      bool RHSIsIntLit = false;

      const Expr* LHS = MR.Nodes.getNodeAs<clang::Expr>("mc612exp");

      if (MR.Nodes.getNodeAs<clang::CharacterLiteral>("mc612charlit") != nullptr)
      {
        RHSIsCharLit = true;
      }

      if (MR.Nodes.getNodeAs<clang::IntegerLiteral>("mc612intlit") != nullptr)
      {
        RHSIsIntLit = true;
      }

      SourceLocation SL = LHS->getLocStart(); 
      CheckSLValidity(SL);
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

      if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
      {
        return void();
      }

      if (!Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
      {
        return void();
      }

      QualType QT = LHS->getType();
      const clang::Type* TP = QT.getTypePtr();

      ASTContext *const ASTC = MR.Context;

      const clang::Type* CanonTP = ASTC->getCanonicalType(TP);

      /*checking whether the unqualified type is simple char*/
      if (CanonTP->isSpecificBuiltinType(BuiltinType::Kind::Char_U) || CanonTP->isSpecificBuiltinType(BuiltinType::Kind::Char_S))
      {
        if (RHSIsIntLit)
        {
          std::cout << "6.1:" << "Simple char type should only hold character values:";
          std::cout << SL.printToString(*MR.SourceManager) << ":" << "\n";

          XMLDocOut.XMLAddNode(MR.Context, SL, "6.1", "Simple char type should only hold character values:");
          JSONDocOUT.JSONAddElement(MR.Context, SL, "6.1", "Simple char type should only hold character values:");
        }
      }

      if (CanonTP->isSpecificBuiltinType(BuiltinType::Kind::UChar) || CanonTP->isSpecificBuiltinType(BuiltinType::Kind::SChar))
      {
        if (RHSIsCharLit)
        {
          std::cout << "6.2:" << "Signed or unsigned char type should only hold numeric values:";
          std::cout << SL.printToString(*MR.SourceManager) << ":" << "\n";

          XMLDocOut.XMLAddNode(MR.Context, SL, "6.2", "Signed or unsigned char type should only hold numeric values:");
          JSONDocOUT.JSONAddElement(MR.Context, SL, "6.2", "Signed or unsigned char type should only hold numeric values:");
        }
      }
    }
  }

private:
  Rewriter &Rewrite;
};
/**********************************************************************************************************************/
class MCCF143 : public MatchFinder::MatchCallback
{
public:
  MCCF143 (Rewriter &Rewrite) : Rewrite(Rewrite) {}

  virtual void run(const MatchFinder::MatchResult &MR)
  {
    if (MR.Nodes.getNodeAs<clang::NullStmt>("mccf143nullstmt") != nullptr)
    {
      const NullStmt* NS = MR.Nodes.getNodeAs<clang::NullStmt>("mccf143nullstmt");

      SourceLocation SL = NS->getSemiLoc(); 
      CheckSLValidity(SL);
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

      ASTContext *const ASTC = MR.Context;

      FullSourceLoc FSL = ASTC->getFullLoc(SL);

      const SourceManager &SM = FSL.getManager();

      StringRef FileNameString = SM.getFilename(SL);

      NullStmtInfo Temp = {FSL.getSpellingColumnNumber(), FSL.getSpellingLineNumber(), FileNameString, SM.isInMainFile(SL), SM.isInSystemHeader(SL)};

      NullStmtProto.push_back(Temp);
    }
  }

private:
  Rewriter &Rewrite;
};
/**********************************************************************************************************************/
class MCExpr1212 : public MatchFinder::MatchCallback
{
public:
  MCExpr1212 (Rewriter &Rewrite) : Rewrite(Rewrite) {}

  virtual void run(const MatchFinder::MatchResult &MR)
  {
    if (MR.Nodes.getNodeAs<clang::RecordDecl>("mcexpr1212") != nullptr)
    {
      const RecordDecl* RD = MR.Nodes.getNodeAs<clang::RecordDecl>("mcexpr1212");

      SourceLocation SL = RD->getLocStart(); 
      CheckSLValidity(SL);
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

      if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
      {
        return void();
      }

      if (Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
      {
        std::cout << "12.12:" << "Possible violation of 12.12-access to the underlying bit representation of a floating type:";
        std::cout << SL.printToString(*MR.SourceManager) << ":" << "\n";

        XMLDocOut.XMLAddNode(MR.Context, SL, "12.12", "Possible violation of 12.12-access to the underlying bit representation of a floating type:");
        JSONDocOUT.JSONAddElement(MR.Context, SL, "12.12", "Possible violation of 12.12-access to the underlying bit representation of a floating type:");
      }
    }
  }

private:
  Rewriter &Rewrite;
};
/**********************************************************************************************************************/
class MCExpr1211 : public MatchFinder::MatchCallback
{
public:
  MCExpr1211 (Rewriter &Rewrite) : Rewrite(Rewrite) {}

  virtual void run(const MatchFinder::MatchResult &MR)
  {
    if (MR.Nodes.getNodeAs<clang::Expr>("mcexpr1211") != nullptr)
    {
      const Expr* EXP = MR.Nodes.getNodeAs<clang::Expr>("mcexpr1211");

      SourceLocation SL = EXP->getLocStart(); 
      CheckSLValidity(SL);
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

      SourceLocation SLE = EXP->getLocEnd();
      SLE = Devi::SourceLocationHasMacro(SLE, Rewrite, "start");

      if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
      {
        return void();
      }

      if (!Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
      {
        return void();
      }

      SourceRange SR;
      SR.setBegin(SL);
      SR.setEnd(SLE);

      std::string targetExpr = Rewrite.getRewrittenText(SR);

      ASTContext *const ASTC = MR.Context;

      QualType QT = EXP->getType();

      const clang::Type* TP = QT.getTypePtr();

      const clang::Type* CanonTP = ASTC->getCanonicalType(TP);

      bool TypeIsUSignedInt = CanonTP->isSpecificBuiltinType(BuiltinType::Kind::ULong) || \
                                  CanonTP->isSpecificBuiltinType(BuiltinType::Kind::UInt) || \
                                  CanonTP->isSpecificBuiltinType(BuiltinType::Kind::UShort) || \
                                  CanonTP->isSpecificBuiltinType(BuiltinType::Kind::UChar) || \
                                  CanonTP->isSpecificBuiltinType(BuiltinType::Kind::Char_U) || \
                                  CanonTP->isSpecificBuiltinType(BuiltinType::Kind::ULongLong) || \
                                  CanonTP->isSpecificBuiltinType(BuiltinType::Kind::UInt128) || \
                                  CanonTP->isSpecificBuiltinType(BuiltinType::Kind::WChar_U);

#if 0
      bool TypeIsUSignedInt = false;
      if (CanonTP) {
    	  auto placeholderType = CanonTP->getAsPlaceholderType();
    	  if (placeholderType) {
    		  TypeIsUSignedInt = placeholderType->isUnsignedInteger();
    	  }
      }
#endif

      if (TypeIsUSignedInt)
      {
        int64_t UnoFinal = 0;
        int64_t DousFinal = 0;
        bool MatchedUno = false;
        bool MatchedDous = false;

        /*@DEVI-compilers that actually treat post and pre inc or dec need more. this doesnt support that.*/
        if (MR.Nodes.getNodeAs<clang::UnaryOperator>("mcexpr1211uno") != nullptr)
        {
          MatchedUno = true;

          const UnaryOperator* UO = MR.Nodes.getNodeAs<clang::UnaryOperator>("mcexpr1211uno");

          clang::UnaryOperator::Opcode UnoOpKind = UO->getOpcode();

          const Expr* UnoSubEXP = UO->getSubExpr();

          llvm::APSInt UnoResult;

          UnoFinal = UnoResult.getExtValue();

          if (UnoSubEXP->EvaluateAsInt(UnoResult, *ASTC))
          {
            if (UnoOpKind == UO_PostInc || UnoOpKind == UO_PreInc)
            {
              UnoFinal++;
            }
            else if (UnoOpKind == UO_PostDec || UnoOpKind == UO_PreDec)
            {
              UnoFinal--;
            }
            else
            {
              /*intentionally left blank. we cant get anything else. were only matching for these two unaryoperators.*/
            }
          }
        }

        if (MR.Nodes.getNodeAs<clang::BinaryOperator>("mcexpr1211dous") != nullptr)
        {
          MatchedDous = true;

          const BinaryOperator* BO = MR.Nodes.getNodeAs<clang::BinaryOperator>("mcexpr1211dous");

          clang::BinaryOperator::Opcode DousOpKind = BO->getOpcode();

          const Expr* DousLHS = BO->getLHS();
          const Expr* DousRHS = BO->getRHS();

          llvm::APSInt DousLHSAPS;
          llvm::APSInt DousRHSAPS;

          if (DousLHS->EvaluateAsInt(DousLHSAPS, *ASTC) && DousRHS->EvaluateAsInt(DousRHSAPS, *ASTC))
          {
            int64_t DousLHSInt64 = DousLHSAPS.getExtValue();
            int64_t DousRHSInt64 = DousRHSAPS.getExtValue();

            switch (DousOpKind)
            {
            case BO_Add:
              DousFinal = DousRHSInt64 + DousLHSInt64;
              break;
            case BO_Sub:
              DousFinal = DousRHSInt64 - DousLHSInt64;
              break;
            case BO_Div:
              DousFinal = DousRHSInt64 / DousLHSInt64;
              break;
            case BO_Mul:
              DousFinal = DousRHSInt64 * DousLHSInt64;
              break;
            default:
              /*cant really happen, were not matching anything else.*/
              break;
            }
          }
        }

        llvm::APSInt OverflowCondidate;

        EXP->EvaluateAsInt(OverflowCondidate, *ASTC);

        int64_t IntExprValue = OverflowCondidate.getExtValue();

        if ((MatchedDous && (DousFinal != IntExprValue)) || (MatchedUno && (UnoFinal != IntExprValue)))
        {
          std::cout << "12.11" << ":" << "Constant Unsinged Expr evaluation resuslts in an overflow:" << SL.printToString(*MR.SourceManager) << ":" << IntExprValue << " " << DousFinal << " " << ":" << targetExpr << "\n";

          XMLDocOut.XMLAddNode(MR.Context, SL, "12.11", "Constant Unsinged Expr evaluation resuslts in an overflow:");
          JSONDocOUT.JSONAddElement(MR.Context, SL, "12.11", "Constant Unsinged Expr evaluation resuslts in an overflow:");
        }
      }
    }
  }

private:
  Rewriter &Rewrite;
};
/**********************************************************************************************************************/
class MCATC105 : public MatchFinder::MatchCallback
{
public:
  MCATC105 (Rewriter &Rewrite) : Rewrite(Rewrite) {}

  virtual void run(const MatchFinder::MatchResult &MR)
  {
    if (MR.Nodes.getNodeAs<clang::Expr>("mcatc105lhs") != nullptr)
    {
      bool ShouldBeTagged = false;
      SourceLocation SL;
      const Expr* IgnoreImplicitEXP;
      ast_type_traits::DynTypedNode DynOpNode;

      if (MR.Nodes.getNodeAs<clang::BinaryOperator>("mcatc105") != nullptr)
      {
        const BinaryOperator* BO = MR.Nodes.getNodeAs<clang::BinaryOperator>("mcatc105");
        DynOpNode = ast_type_traits::DynTypedNode::create<clang::BinaryOperator>(*BO);

        SL = BO->getLocStart(); 
        CheckSLValidity(SL);
        SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");
      }

      if (MR.Nodes.getNodeAs<clang::UnaryOperator>("mcatc105uno") != nullptr)
      {
        const UnaryOperator* UO = MR.Nodes.getNodeAs<clang::UnaryOperator>("mcatc105uno");
        DynOpNode = ast_type_traits::DynTypedNode::create<clang::UnaryOperator>(*UO);

        SL = UO->getLocStart(); 
        CheckSLValidity(SL);
        SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");
      }

      const Expr* EXP = MR.Nodes.getNodeAs<clang::Expr>("mcatc105lhs");
      IgnoreImplicitEXP = EXP->IgnoreImpCasts();

      if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
      {
        return void();
      }

      if (!Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
      {
        return void();
      }

      ASTContext *const ASTC = MR.Context;

      const TargetInfo &TI = ASTC->getTargetInfo();

      unsigned int ShortSize = TI.getShortWidth();

      QualType QT = IgnoreImplicitEXP->getType();
      const clang::Type* TP = QT.getTypePtr();

      const clang::Type* CanonTP = ASTC->getCanonicalType(TP);

      if (CanonTP->isUnsignedIntegerType() || CanonTP->isSignedIntegerType() || CanonTP->isAnyCharacterType())
      {

        /*@DEVI-assumptions:nothing has more than one parent in C.*/
        if (ShortSize == ASTC->getTypeSize(QT) || 8U == ASTC->getTypeSize(QT))
        {
          /*@DEVI-assumes there is only one parent for every node which is true only for C, not Cpp*/
          ASTContext::DynTypedNodeList NodeList = ASTC->getParents(DynOpNode);

          const ast_type_traits::DynTypedNode &ParentNode = NodeList[0U];

          ASTContext::DynTypedNodeList AncestorNodeList = ASTC->getParents(ParentNode);

          const ast_type_traits::DynTypedNode AncestorNode = AncestorNodeList[0U];

          ast_type_traits::ASTNodeKind ParentNodeKind = ParentNode.getNodeKind();
          ast_type_traits::ASTNodeKind AncestorNodeKind = AncestorNode.getNodeKind();

          std::string ParentStringKind = ParentNodeKind.asStringRef().str();
          std::string AncestorStringKind = AncestorNodeKind.asStringRef().str();

          if (ParentStringKind == "CStyleCastExpr")
          {
            const CStyleCastExpr* CSCE = ParentNode.get<clang::CStyleCastExpr>();

            if (CSCE->getType() != QT)
            {
              ShouldBeTagged = true;
            }
          }
          else if (ParentStringKind == "ParenExpr" && AncestorStringKind == "CStyleCastExpr")
          {
            const CStyleCastExpr* CSCE = AncestorNode.get<clang::CStyleCastExpr>();

            if (CSCE->getType() != QT)
            {
              ShouldBeTagged = true;
            }
          }
          else
          {
            ShouldBeTagged = true;
          }

          if (ShouldBeTagged)
          {
            std::cout << "10.5" << ":" << "Result of operands << or ~ must be explicitly cast to the type of the expression:" << SL.printToString(*MR.SourceManager) << ":" << "\n";

            XMLDocOut.XMLAddNode(MR.Context, SL, "10.5", "Result of operands << or ~ must be explicitly cast to the type of the expression:");
            JSONDocOUT.JSONAddElement(MR.Context, SL, "10.5", "Result of operands << or ~ must be explicitly cast to the type of the expression:");

            if (mutagen)
            {
              ME.ExtractAncestry(ast_type_traits::DynTypedNode::create(*EXP), *MR.Context);
            }
          }
        }
      }

#if 0
      const BuiltinType* BT = CanonTP->getAsPlaceholderType();
      const LangOptions &LO = ASTC->getLangOpts();
      std::string something = TypeName::getFullyQualifiedName(QT, *ASTC, true);
      StringRef BTName = BT->getName(PrintingPolicy(LO));
#endif
    }
  }

private:
  Rewriter &Rewrite;
};
/**********************************************************************************************************************/
class MCCSE135 : public MatchFinder::MatchCallback
{
public:
  MCCSE135 (Rewriter &Rewrite) : Rewrite(Rewrite) {}

  virtual void run(const MatchFinder::MatchResult &MR)
  {
    if (MR.Nodes.getNodeAs<clang::ForStmt>("mccse135") != nullptr)
    {
      const ForStmt* FS = MR.Nodes.getNodeAs<clang::ForStmt>("mccse135");

      SourceLocation SL = FS->getLocStart(); 
      CheckSLValidity(SL);
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

      if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
      {
        return void();
      }

      if (!Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
      {
        return void();
      }

      ASTContext *const ASTC = MR.Context;

      const Expr* FSCond = FS->getCond();
      const Expr* FSInc = FS->getInc();
      const Stmt* FSInit = FS->getInit();

      bool CondPresent = (FSCond != nullptr);
      bool DIncPresent = (FSInc != nullptr);
      bool InitPresent = (FSInit != nullptr);

      /*@DEVI-for the third one we are not checking to see whether the loop counter has been previously initialized.*/
      if (!((CondPresent && DIncPresent && InitPresent) || (!CondPresent && !DIncPresent && !InitPresent) || (CondPresent && DIncPresent && !InitPresent)))
      {
        std::cout << "13.5" << ":" << "The three expressions of a ForStmt shall either all exist or not exist at all or only the initialization can be missing:" << SL.printToString(*MR.SourceManager) << ":" << "\n";

        XMLDocOut.XMLAddNode(MR.Context, SL, "13.5", "The three expressions of a ForStmt shall either all exist or not exist at all or only the initialization can be missing:");
        JSONDocOUT.JSONAddElement(MR.Context, SL, "13.5", "The three expressions of a ForStmt shall either all exist or not exist at all or only the initialization can be missing:");
      }

      if (FSInc != nullptr)
      {
        if (!FSInc->HasSideEffects(*ASTC, true))
        {
          std::cout << "13.5" << ":" << "The increment expression in the ForStmt has no side-effects:" << SL.printToString(*MR.SourceManager) << ":" << "\n";

          XMLDocOut.XMLAddNode(MR.Context, SL, "13.5", "The increment expression in the ForStmt has no side-effects:");
          JSONDocOUT.JSONAddElement(MR.Context, SL, "13.5", "The increment expression in the ForStmt has no side-effects:");
        }
      }

      if (FSCond != nullptr)
      {
        if (FSCond->HasSideEffects(*ASTC, true))
        {
          std::cout << "13.5" << ":" << "The condition expression in the ForStmt has side-effect:" << SL.printToString(*MR.SourceManager) << ":" << "\n";

          XMLDocOut.XMLAddNode(MR.Context, SL, "13.5", "The condition expression in the ForStmt has side-effect:");
          JSONDocOUT.JSONAddElement(MR.Context, SL, "13.5", "The condition expression in the ForStmt has side-effect:");
        }

        if (!FSCond->isKnownToHaveBooleanValue())
        {
          std::cout << "13.5" << ":" << "The expression in the ForStmt condition does not return a boolean:" << SL.printToString(*MR.SourceManager) << ":" << "\n";

          XMLDocOut.XMLAddNode(MR.Context, SL, "13.5", "The expression in the ForStmt condition does not return a boolean:");
          JSONDocOUT.JSONAddElement(MR.Context, SL, "13.5", "The expression in the ForStmt condition does not return a boolean:");
        }
      }
    }
  }

private:
  Rewriter &Rewrite;
};
/**********************************************************************************************************************/
class MCConst71 : public MatchFinder::MatchCallback
{
public:
  MCConst71 (Rewriter &Rewrite) : Rewrite(Rewrite) {}

  virtual void run(const MatchFinder::MatchResult &MR)
  {
    std::string TagCandidateString;
    SourceLocation SL;

    if (MR.Nodes.getNodeAs<clang::IntegerLiteral>("mcconst71int") != nullptr)
    {
      const IntegerLiteral* IL = MR.Nodes.getNodeAs<clang::IntegerLiteral>("mcconst71int");

      SourceRange SR;
      SL = IL->getLocStart(); 
      CheckSLValidity(SL);
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");
      SR.setBegin(SL);
      SourceLocation SLE = IL->getLocEnd();
      SLE = Devi::SourceLocationHasMacro(SLE, Rewrite, "start");
      SR.setEnd(SLE);

      TagCandidateString = Rewrite.getRewrittenText(SR);
    }

    if (MR.Nodes.getNodeAs<clang::StringLiteral>("mcconst71string") != nullptr)
    {
      const clang::StringLiteral* StringLit = MR.Nodes.getNodeAs<clang::StringLiteral>("mcconst71string");

      SL = StringLit->getLocStart(); 
      CheckSLValidity(SL);
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "strat");

      TagCandidateString = StringLit->getString().str();
    }

    if (MR.Nodes.getNodeAs<clang::CharacterLiteral>("mcconst71char") != nullptr)
    {
      const CharacterLiteral* CL = MR.Nodes.getNodeAs<clang::CharacterLiteral>("mcconst71char");

      SL = CL->getLocStart(); 
      CheckSLValidity(SL);
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

      SourceRange SR;
      SourceLocation SLE = CL->getLocEnd();
      SLE = Devi::SourceLocationHasMacro(SLE, Rewrite, "start");
      SR.setBegin(SL);
      SR.setEnd(SLE);

      TagCandidateString = Rewrite.getRewrittenText(SR);
    }

    if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
    {
      return void();
    }

    if (!Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
    {
      return void();
    }

    std::regex octalconstant("\\\\[0-7]+");
    std::regex octalconstantint("^0[0-9]+$");
    std::regex hexescapesequence("\\\\x[0-9a-fA-F]+");
    std::regex otherescapesequence("\\\\[^0-9abfnrtvx'\"\\?]");

    std::smatch result;

#if 0
    std::cout << "diagnostic2:" << TagCandidateString << ":" << SL.printToString(*MR.SourceManager) << ":" << std::regex_search(TagCandidateString, result, octalconstant) << "\n";
#endif

    if (std::regex_search(TagCandidateString, result, octalconstant) || std::regex_search(TagCandidateString, result, octalconstantint))
    {
      std::cout << "7.1" << ":" << "Octal escape sequence used:" << SL.printToString(*MR.SourceManager) << ":" << TagCandidateString << "\n";

      XMLDocOut.XMLAddNode(MR.Context, SL, "7.1", "Octal escape sequence used:");
      JSONDocOUT.JSONAddElement(MR.Context, SL, "7.1", "Octal escape sequence used:");
    }

    if (std::regex_search(TagCandidateString, result, hexescapesequence))
    {
      std::cout << "4.1" << ":" << "Hexadecimal escape sequence used:" << SL.printToString(*MR.SourceManager) << ":" << TagCandidateString << "\n";

      XMLDocOut.XMLAddNode(MR.Context, SL, "4.1", "Hexadecimal escape sequence used:");
      JSONDocOUT.JSONAddElement(MR.Context, SL, "4.1", "Hexadecimal escape sequence used:");
    }

    if (std::regex_search(TagCandidateString, result, otherescapesequence))
    {
      std::cout << "4.1" << ":" << "Non-standard escape sequence used:" << SL.printToString(*MR.SourceManager) << ":" << TagCandidateString << "\n";

      XMLDocOut.XMLAddNode(MR.Context, SL, "4.1", "Non-standard escape sequence used:");
      JSONDocOUT.JSONAddElement(MR.Context, SL, "4.1", "Non-standard escape sequence used:");
    }
  }

private:
  Rewriter &Rewrite;
};
/**********************************************************************************************************************/
class MCIdent5x : public MatchFinder::MatchCallback
{
public:
  MCIdent5x (Rewriter &Rewrite) : Rewrite(Rewrite) {}

  virtual void run(const MatchFinder::MatchResult &MR)
  {
    if (MR.Nodes.getNodeAs<clang::TypedefDecl>("ident5typedef") != nullptr)
    {
      const TypedefDecl* BN = MR.Nodes.getNodeAs<clang::TypedefDecl>("ident5typedef");

      SourceLocation SL = BN->getLocStart(); 
      CheckSLValidity(SL);
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

      if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
      {
        return void();
      }

      if (!Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
      {
        return void();
      }

      const IdentifierInfo* II = BN->getIdentifier();

      std::string Name = II->getName().str();

      SourceManager *const SM = MR.SourceManager;

      IdentInfo Temp = {SM->getSpellingLineNumber(SL), SM->getSpellingColumnNumber(SL), \
                        SM->getFilename(SL).str(), Name, SL.printToString(*SM), Devi::NodeKind::TypedefDecl, \
                        false, Devi::FunctionDeclKind::NoValue, Devi::Scope::TU, "", true, false
                       };

      IdentInfoProto.push_back(Temp);
    }

    if (MR.Nodes.getNodeAs<clang::RecordDecl>("ident5record") != nullptr)
    {
      const RecordDecl* BN = MR.Nodes.getNodeAs<clang::RecordDecl>("ident5record");

      SourceLocation SL = BN->getLocStart(); 
      CheckSLValidity(SL);
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

      if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
      {
        return void();
      }

      if (!Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
      {
        return void();
      }

      const IdentifierInfo* II = BN->getIdentifier();

      std::string Name;

      if (II != nullptr)
      {
        Name = II->getName().str();
      }

      SourceManager *const SM = MR.SourceManager;

      std::string FunctionName;

      if (MR.Nodes.getNodeAs<clang::FunctionDecl>("id5funcscope") != nullptr)
      {
        const FunctionDecl* FD = MR.Nodes.getNodeAs<clang::FunctionDecl>("id5funcscope");

        FunctionName = FD->getNameAsString();
      }

      Devi::Scope tempscope = Devi::Scope::TU;

      if (FunctionName != "")
      {
        tempscope = Devi::Scope::Block;
      }

      IdentInfo Temp = {SM->getSpellingLineNumber(SL), SM->getSpellingColumnNumber(SL), \
                        SM->getFilename(SL).str(), Name, SL.printToString(*SM), Devi::NodeKind::RecordDecl, \
                        BN->getTypeForDecl()->isIncompleteType(), Devi::FunctionDeclKind::NoValue, tempscope, FunctionName, true, false
                       };

      IdentInfoProto.push_back(Temp);
    }

    if (MR.Nodes.getNodeAs<clang::FieldDecl>("ident5field") != nullptr)
    {
      const FieldDecl* BN = MR.Nodes.getNodeAs<clang::FieldDecl>("ident5field");

      SourceLocation SL = BN->getLocStart(); 
      CheckSLValidity(SL);
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

      if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
      {
        return void();
      }

      if (!Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
      {
        return void();
      }

      const IdentifierInfo* II = BN->getIdentifier();

      std::string Name;

      if (II != nullptr)
      {
        Name = II->getName().str();
      }

      SourceManager *const SM = MR.SourceManager;

      std::string FunctionName;

      if (MR.Nodes.getNodeAs<clang::FunctionDecl>("id5funcscope") != nullptr)
      {
        const FunctionDecl* FD = MR.Nodes.getNodeAs<clang::FunctionDecl>("id5funcscope");

        FunctionName = FD->getNameAsString();
      }

      Devi::Scope tempscope = Devi::Scope::TU;

      if (FunctionName != "")
      {
        tempscope = Devi::Scope::Block;
      }

      IdentInfo Temp = {SM->getSpellingLineNumber(SL), SM->getSpellingColumnNumber(SL), \
                        SM->getFilename(SL).str(), Name, SL.printToString(*SM), Devi::NodeKind::FieldDecl, \
                        false, Devi::FunctionDeclKind::NoValue, tempscope, FunctionName, true, false
                       };

      IdentInfoProto.push_back(Temp);
    }

    /*@DEVI-dunno how it will handle incomplete records passed as parmvars.*/
    if (MR.Nodes.getNodeAs<clang::ParmVarDecl>("ident5parmvar") != nullptr)
    {
      const ParmVarDecl* BN = MR.Nodes.getNodeAs<clang::ParmVarDecl>("ident5parmvar");

      SourceLocation SL = BN->getLocStart(); 
      CheckSLValidity(SL);
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

      if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
      {
        return void();
      }

      if (!Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
      {
        return void();
      }

      const IdentifierInfo* II = BN->getIdentifier();

      std::string Name;

      if (II != nullptr)
      {
        Name = II->getName().str();
      }

      SourceManager *const SM = MR.SourceManager;

      std::string FunctionName;

      if (MR.Nodes.getNodeAs<clang::FunctionDecl>("id5funcscope") != nullptr)
      {
        const FunctionDecl* FD = MR.Nodes.getNodeAs<clang::FunctionDecl>("id5funcscope");

        FunctionName = FD->getNameAsString();
      }

      Devi::Scope tempscope = Devi::Scope::TU;

      if (FunctionName != "")
      {
        tempscope = Devi::Scope::Block;
      }

      IdentInfo Temp = {SM->getSpellingLineNumber(SL), SM->getSpellingColumnNumber(SL), \
                        SM->getFilename(SL).str(), Name, SL.printToString(*SM), Devi::NodeKind::ParmVarDecl, \
                        false, Devi::FunctionDeclKind::NoValue, tempscope, FunctionName, true, BN->isStaticLocal()
                       };

      IdentInfoProto.push_back(Temp);
    }

    if (MR.Nodes.getNodeAs<clang::FunctionDecl>("ident5func") != nullptr)
    {
      const FunctionDecl* BN = MR.Nodes.getNodeAs<clang::FunctionDecl>("ident5func");

      SourceLocation SL = BN->getLocStart(); 
      CheckSLValidity(SL);
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

      if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
      {
        return void();
      }

      if (!Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
      {
        return void();
      }

      const IdentifierInfo* II = BN->getIdentifier();

      std::string Name;

      if (II != nullptr)
      {
        Name = II->getName().str();
      }

      SourceManager *const SM = MR.SourceManager;

      Devi::FunctionDeclKind tempfkind;

      if (BN->isThisDeclarationADefinition())
      {
        tempfkind = Devi::FunctionDeclKind::Definition;
      }
      else
      {
        tempfkind = Devi::FunctionDeclKind::Declaration;
      }

      IdentInfo Temp = {SM->getSpellingLineNumber(SL), SM->getSpellingColumnNumber(SL), \
                        SM->getFilename(SL).str(), Name, SL.printToString(*SM), Devi::NodeKind::FunctionDecl, \
                        false, tempfkind, Devi::Scope::TU, "", true, false
                       };

      IdentInfoProto.push_back(Temp);
    }

    if (MR.Nodes.getNodeAs<clang::VarDecl>("ident5var") != nullptr)
    {
      const VarDecl* BN = MR.Nodes.getNodeAs<clang::VarDecl>("ident5var");

      SourceLocation SL = BN->getLocStart(); 
      CheckSLValidity(SL);
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

      if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
      {
        return void();
      }

      if (!Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
      {
        return void();
      }

      const IdentifierInfo* II = BN->getIdentifier();

      std::string Name;

      if (II != nullptr)
      {
        Name = II->getName().str();
      }

      SourceManager *const SM = MR.SourceManager;

      std::string FunctionName;

      if (MR.Nodes.getNodeAs<clang::FunctionDecl>("id5funcscope") != nullptr)
      {
        const FunctionDecl* FD = MR.Nodes.getNodeAs<clang::FunctionDecl>("id5funcscope");

        FunctionName = FD->getNameAsString();
      }

      Devi::Scope tempscope = Devi::Scope::TU;

      if (FunctionName != "")
      {
        tempscope = Devi::Scope::Block;
      }

      IdentInfo Temp = {SM->getSpellingLineNumber(SL), SM->getSpellingColumnNumber(SL), \
                        SM->getFilename(SL).str(), Name, SL.printToString(*SM), Devi::NodeKind::VarDecl, \
                        false, Devi::FunctionDeclKind::NoValue, tempscope, FunctionName, true, BN->isStaticLocal()
                       };

      IdentInfoProto.push_back(Temp);
    }

    if (MR.Nodes.getNodeAs<clang::EnumDecl>("ident5enum") != nullptr)
    {
      const EnumDecl* BN = MR.Nodes.getNodeAs<clang::EnumDecl>("ident5enum");

      SourceLocation SL = BN->getLocStart(); 
      CheckSLValidity(SL);
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

      if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
      {
        return void();
      }

      if (!Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
      {
        return void();
      }

      const IdentifierInfo* II = BN->getIdentifier();

      std::string Name;

      if (II != nullptr)
      {
        Name = II->getName().str();
      }

      SourceManager *const SM = MR.SourceManager;

      std::string FunctionName;

      if (MR.Nodes.getNodeAs<clang::FunctionDecl>("id5funcscope") != nullptr)
      {
        const FunctionDecl* FD = MR.Nodes.getNodeAs<clang::FunctionDecl>("id5funcscope");

        FunctionName = FD->getNameAsString();
      }

      Devi::Scope tempscope = Devi::Scope::TU;

      if (FunctionName != "")
      {
        tempscope = Devi::Scope::Block;
      }

      IdentInfo Temp = {SM->getSpellingLineNumber(SL), SM->getSpellingColumnNumber(SL), \
                        SM->getFilename(SL).str(), Name, SL.printToString(*SM), Devi::NodeKind::EnumDecl, \
                        false, Devi::FunctionDeclKind::NoValue, tempscope, FunctionName, true, false
                       };

      IdentInfoProto.push_back(Temp);
    }

    if (MR.Nodes.getNodeAs<clang::LabelDecl>("ident5label") != nullptr)
    {
      const LabelDecl* BN = MR.Nodes.getNodeAs<clang::LabelDecl>("ident5label");

      SourceLocation SL = BN->getLocStart(); 
      CheckSLValidity(SL);
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

      if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
      {
        return void();
      }

      if (!Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
      {
        return void();
      }

      const IdentifierInfo* II = BN->getIdentifier();

      std::string Name;

      if (II != nullptr)
      {
        Name = II->getName().str();
      }

      SourceManager *const SM = MR.SourceManager;

      std::string FunctionName;

      if (MR.Nodes.getNodeAs<clang::FunctionDecl>("id5funcscope") != nullptr)
      {
        const FunctionDecl* FD = MR.Nodes.getNodeAs<clang::FunctionDecl>("id5funcscope");

        FunctionName = FD->getNameAsString();
      }

      Devi::Scope tempscope = Devi::Scope::TU;

      if (FunctionName != "")
      {
        tempscope = Devi::Scope::Block;
      }

      IdentInfo Temp = {SM->getSpellingLineNumber(SL), SM->getSpellingColumnNumber(SL), \
                        SM->getFilename(SL).str(), Name, SL.printToString(*SM), Devi::NodeKind::LabelDecl, \
                        false, Devi::FunctionDeclKind::NoValue, tempscope, FunctionName, true, false
                       };

      IdentInfoProto.push_back(Temp);
    }

    if (MR.Nodes.getNodeAs<clang::EnumConstantDecl>("ident5enumconst") != nullptr)
    {
      const EnumConstantDecl* BN = MR.Nodes.getNodeAs<clang::EnumConstantDecl>("ident5enumconst");

      SourceLocation SL = BN->getLocStart(); 
      CheckSLValidity(SL);
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

      if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
      {
        return void();
      }

      if (!Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
      {
        return void();
      }

      const IdentifierInfo* II = BN->getIdentifier();

      std::string Name;

      if (II != nullptr)
      {
        Name = II->getName().str();
      }

      SourceManager *const SM = MR.SourceManager;

      std::string FunctionName;

      if (MR.Nodes.getNodeAs<clang::FunctionDecl>("id5funcscope") != nullptr)
      {
        const FunctionDecl* FD = MR.Nodes.getNodeAs<clang::FunctionDecl>("id5funcscope");

        FunctionName = FD->getNameAsString();
      }

      Devi::Scope tempscope = Devi::Scope::TU;

      if (FunctionName != "")
      {
        tempscope = Devi::Scope::Block;
      }

      IdentInfo Temp = {SM->getSpellingLineNumber(SL), SM->getSpellingColumnNumber(SL), \
                        SM->getFilename(SL).str(), Name, SL.printToString(*SM), Devi::NodeKind::EnumConstDecl, \
                        false, Devi::FunctionDeclKind::NoValue, tempscope, FunctionName, true, false
                       };

      IdentInfoProto.push_back(Temp);
    }
  }

private:
  Rewriter &Rewrite;
};
/**********************************************************************************************************************/
/**
 * @brief Tags all array declarations and uses of arrays through indexes.
 */
class SFCPPARR01 : public MatchFinder::MatchCallback
{
  public:
    SFCPPARR01 (Rewriter &Rewrite) : Rewrite(Rewrite) {}

    /**
     * @brief The virtual method that runs the tagging.
     *
     * @param MR MatchFinder::MatchResulet 
     */
    virtual void run(const MatchFinder::MatchResult &MR)
    {
      if (MR.Nodes.getNodeAs<clang::VarDecl>("sfcpparrdecl") != nullptr)
      {
        const VarDecl* VD = MR.Nodes.getNodeAs<clang::VarDecl>("sfcpparrdecl");
        
        SourceLocation SL = VD->getLocStart();
        CheckSLValidity(SL);
        SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

        if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
        {
          return void();
        }

        if (!Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
        {
          return void();
        }

        std::cout << "SaferCPP01" << ":" << "Native CPP array declared:" << SL.printToString(*MR.SourceManager) << ":" << "\n";

        XMLDocOut.XMLAddNode(MR.Context, SL, "SaferCPP01", "Native CPP array declared:");
        JSONDocOUT.JSONAddElement(MR.Context, SL, "SaferCPP01", "Native CPP array declared:");
      }

      if (MR.Nodes.getNodeAs<clang::CastExpr>("sfcpparrcastexpr") != nullptr)
      {
        const CastExpr* CS = MR.Nodes.getNodeAs<clang::CastExpr>("sfcpparrcastexpr");

        SourceLocation SL = CS->getLocStart();
        CheckSLValidity(SL);
        SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

        if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
        {
          return void();
        }

        if (!Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
        {
          return void();
        }

        std::cout << "SaferCPP01" << ":" << "Native CPP array used:" << SL.printToString(*MR.SourceManager) << ":" << "\n";

        XMLDocOut.XMLAddNode(MR.Context, SL, "SaferCPP01", "Native CPP array used");
        JSONDocOUT.JSONAddElement(MR.Context, SL, "SaferCPP01", "Native CPP arry=ay used");
      }

      if (MR.Nodes.getNodeAs<clang::FieldDecl>("sfcpparrfield") != nullptr)
      {
        const FieldDecl* FD = MR.Nodes.getNodeAs<clang::FieldDecl>("sfcpparrfield");

        SourceLocation SL = FD->getLocStart();
        CheckSLValidity(SL);
        SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

        if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
        {
          return void();
        }

        if (!Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
        {
          return void();
        }

        std::cout << "SaferCPP01" << ":" << "Native CPP array field used:" << SL.printToString(*MR.SourceManager) << ":" << "\n";

        XMLDocOut.XMLAddNode(MR.Context, SL, "SaferCPP01", "Native CPP array field used");
        JSONDocOUT.JSONAddElement(MR.Context, SL, "SaferCPP01", "Native CPP arryay field used");
      }
    }

  private:
    Rewriter &Rewrite;
};
/**********************************************************************************************************************/
/**
 * @brief The matcher run by SFCPPARR02. This ones does all the real tagging.
 */
class SFCPPARR02SUB : public MatchFinder::MatchCallback
{
  public:
    SFCPPARR02SUB (Rewriter &Rewrite) : Rewrite(Rewrite) {}

    virtual void run(const MatchFinder::MatchResult &MR)
    {
      if (MR.Nodes.getNodeAs<clang::DeclRefExpr>("sfcpp02sub") != nullptr)
      {
        const DeclRefExpr* DRE = MR.Nodes.getNodeAs<clang::DeclRefExpr>("sfcpp02sub");

        SourceManager *const SM = MR.SourceManager;

        SourceLocation SL = DRE->getLocStart();
        CheckSLValidity(SL);
        //SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");
        SL = SM->getSpellingLoc(SL);

        if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
        {
          return void();
        }

        if (!Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
        {
          return void();
        }

        const NamedDecl* ND = DRE->getFoundDecl();

        SourceLocation OriginSL = ND->getLocStart();
        CheckSLValidity(OriginSL);
        //OriginSL = Devi::SourceLocationHasMacro(OriginSL, Rewrite, "start");
        OriginSL = SM->getSpellingLoc(OriginSL);

        StringRef OriginFileName [[maybe_unused]] = SM->getFilename(OriginSL);

#if 0
        std::cout << "GarbageOut" << ":" << "Origin:" << DRE->getFoundDecl()->getName().str() << "\n";
        std::cout << "GarbageOut" << ":" << "Origin:" << ExtOriginFileName.str() << ":" << "Proto:" << OriginFileName.str() << "\n";
        std::cout << "GarbageOut" << ":" << "Origin:" << ExtOriginSL.printToString(*SM) << ":" << "Proto:" << OriginSL.printToString(*SM) << "\n";
#endif

        if (OriginSL == ExtOriginSL && OriginFileName == ExtOriginFileName)
        {
          std::cout << "SaferCPP01" << ":" << "Native Array used - pointer points to an array:" << SL.printToString(*MR.SourceManager) << ":" << DRE->getFoundDecl()->getName().str() << "\n";
        }

        XMLDocOut.XMLAddNode(MR.Context, SL, "SaferCPP01", "Native Array used - pointer points to an array:");
        JSONDocOUT.JSONAddElement(MR.Context, SL, "SaferCPP01", "Native Array used - pointer points to an array:");
      }
    }

    void setOriginSourceLocation(SourceLocation inSL)
    {
    ExtOriginSL = inSL;
    }

    void setOriginFileName(StringRef inStrRef)
    {
      ExtOriginFileName = inStrRef;
    }

  private:
    Rewriter &Rewrite [[maybe_unused]];
    SourceLocation ExtOriginSL;
    StringRef ExtOriginFileName;
};
/**********************************************************************************************************************/
/**
 * @brief MatchCallback for safercpp matching of pointers pointing to arrays.
 */
class SFCPPARR02 : public MatchFinder::MatchCallback
{
  public:
    SFCPPARR02 (Rewriter &Rewrite) : Rewrite(Rewrite), SubHandler(Rewrite) {}

    virtual void run(const MatchFinder::MatchResult &MR)
    {
      if (MR.Nodes.getNodeAs<clang::DeclRefExpr>("sfcpparrdeep") != nullptr)
      {
        const DeclRefExpr* DRE = MR.Nodes.getNodeAs<clang::DeclRefExpr>("sfcpparrdeep");

        ASTContext *const ASTC = MR.Context;

        SourceManager *const SM = MR.SourceManager;

        SourceLocation SL = DRE->getLocStart();
        CheckSLValidity(SL);
        SL = SM->getSpellingLoc(SL);

        const NamedDecl* ND = DRE->getFoundDecl();

        StringRef NDName = ND->getName();

        SubHandler.setOriginSourceLocation(SM->getSpellingLoc(ND->getLocStart()));
        SubHandler.setOriginFileName(SM->getFilename(SM->getSpellingLoc(ND->getLocStart())));

        Matcher.addMatcher(declRefExpr(to(varDecl(hasName(NDName.str())))).bind("sfcpp02sub"), &SubHandler);

        Matcher.matchAST(*ASTC);

#if 0
        std::cout << "GarbageOutOrigin" << ":" << "GarbageOutOrigin:" << SL.printToString(*MR.SourceManager) << ":" << NDName.str() << "\n";
#endif
      }
    }

  private:
    Rewriter &Rewrite [[maybe_unused]];
    MatchFinder Matcher;
    SFCPPARR02SUB SubHandler;
};
/**********************************************************************************************************************/
/**
 * @brief The callback for the Safercpp pointer matchers. Matches the dedlarations.
 */
class SFCPPPNTR01 : public MatchFinder::MatchCallback
{
  public:
    SFCPPPNTR01 (Rewriter &Rewrite) : Rewrite(Rewrite) {}

    virtual void run(const MatchFinder::MatchResult &MR)
    {
      if (MR.Nodes.getNodeAs<clang::VarDecl>("sfcpppntr01") != nullptr)
      {
        const VarDecl* VD = MR.Nodes.getNodeAs<clang::VarDecl>("sfcpppntr01");

        SourceLocation SL = VD->clang::Decl::getLocStart();
        CheckSLValidity(SL);
        SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

        if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
        {
          return void();
        }

        if (!Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
        {
          return void();
        }

        std::cout << "SaferCPP02" << ":" << "Native pointer declared:" << SL.printToString(*MR.SourceManager) << ":" << "\n";

        XMLDocOut.XMLAddNode(MR.Context, SL, "SaferCPP02", "Native pointer declared:");
        JSONDocOUT.JSONAddElement(MR.Context, SL, "SaferCPP02", "Native pointer declared:");
      }
    }

  private:
    Rewriter &Rewrite;
};
/**********************************************************************************************************************/
/**
 * @brief The callback for the Safercpp pointer matchers. Matches the DeclRefExprs.
 */
class SFCPPPNTR02 : public MatchFinder::MatchCallback
{
  public:
    SFCPPPNTR02 (Rewriter &Rewrite) : Rewrite(Rewrite) {}

    virtual void run(const MatchFinder::MatchResult &MR)
    {
      if (MR.Nodes.getNodeAs<clang::DeclRefExpr>("sfcpppntr02") != nullptr)
      {
        const DeclRefExpr* DRE = MR.Nodes.getNodeAs<clang::DeclRefExpr>("sfcpppntr02");

        SourceLocation SL = DRE->getLocStart();
        CheckSLValidity(SL);
        SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

        if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
        {
          return void();
        }

        if (!Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
        {
          return void();
        }

        std::cout << "SaferCPP02" << ":" << "Native pointer used:" << SL.printToString(*MR.SourceManager) << ":" << "\n";

        XMLDocOut.XMLAddNode(MR.Context, SL, "SaferCPP02", "Native pointer used:");
        JSONDocOUT.JSONAddElement(MR.Context, SL, "SaferCPP02", "Native pointer used:");
      }
    }

  private:
    Rewriter &Rewrite;
};
/**********************************************************************************************************************/
/**********************************************************************************************************************/
/**********************************************************************************************************************/
/**********************************************************************************************************************/
/**********************************************************************************************************************/
/**********************************************************************************************************************/
/**********************************************************************************************************************/
/**********************************************************************************************************************/
/**********************************************************************************************************************/
/**********************************************************************************************************************/
/**********************************************************************************************************************/
/*the sourcelocation used in the overload of XMLAddNode that takes sourcemanager as input parameter uses
the spelling location so the client does not need to check the sourcelocation for macros expansions.*/
class PPInclusion : public PPCallbacks
{
public:
  explicit PPInclusion (SourceManager *SM) : SM(*SM) {}

  /*@DEVI-if we dont throw an exception for a bad header inclusion, we wil crash later on since we have InclusionDirective PPCallback.*/
  virtual bool FileNotFound(StringRef FileName, SmallVectorImpl<char> &RecoveryPath)
  {
    std::cerr << "\033[1;31mHeader Not Found: " << FileName.str() << "\033[0m" << "\n";
#if 0
    std::abort();
#endif
    throw MutExHeaderNotFound(FileName.str());
  }

  virtual void InclusionDirective (SourceLocation HashLoc, const Token &IncludeTok, StringRef FileName, \
                                   bool IsAngled, CharSourceRange FileNameRange, const FileEntry* File, \
                                   StringRef SearchPath, StringRef RelativePath, const clang::Module* Imported)
  {
 
    CheckSLValidity(HashLoc);

#if defined(__linux__)
    std::ifstream HeaderABS(SearchPath.str() + "/" + FileName.str());
    std::ifstream HeaderRel(RelativePath.str() + "/" + FileName.str());
#elif defined(__MACH__) && defined(__APPLE__)
    std::ifstream HeaderABS(SearchPath.str() + "/" + FileName.str());
    std::ifstream HeaderRel(RelativePath.str() + "/" + FileName.str());
#elif defined(__CYGWIN__) || defined(_WIN32) || defined(_WIN64)
    std::ifstream HeaderABS(SearchPath.str() + "\\" + FileName.str());
    std::ifstream HeaderRel(RelativePath.str() + "\\" + FileName.str());
#else
    std::ifstream HeaderABS(SearchPath.str() + "/" + FileName.str());
    std::ifstream HeaderRel(RelativePath.str() + "/" + FileName.str());
#endif

    if (File->isValid() && (HeaderABS.good() || HeaderRel.good()))
    {
#if 0
      assert(HashLoc.isValid() && "The SourceLocation for InclusionDirective is invalid.");
#endif

#if 1
      if (IsAngled)
      {
        size_t singleQPos = FileName.find("\'", 0);
        size_t doubleQPos = FileName.find("\"", 0);
        size_t whateverSlashPos = FileName.find("\\", 0);
        size_t commentPos = FileName.find("\\*", 0);

        if (singleQPos != std::string::npos || doubleQPos != std::string::npos || whateverSlashPos != std::string::npos || commentPos != std::string::npos)
        {
          if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, SM, HashLoc))
          {
            /*intentionally left blank*/
          }
          else
          {
            if (Devi::IsTheMatchInMainFile(MainFileOnly, SM, HashLoc))
            {
              std::cout << "19.2:" << "illegal characters in inclusion directive:";
              std::cout << HashLoc.printToString(SM) << ":" << "\n";

              XMLDocOut.XMLAddNode(SM, HashLoc, "19.2", "illegal characters in inclusion directive : ");
              JSONDocOUT.JSONAddElement(SM, HashLoc, "19.2", "illegal characters in inclusion directive : ");
            }
          }
        }

        if (FileName == "errno.h")
        {
          if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, SM, HashLoc))
          {
            /*intentionally left blank*/
          }
          else
          {
            if (Devi::IsTheMatchInMainFile(MainFileOnly, SM, HashLoc))
            {
              std::cout << "20.5:" << "errno shall not be used:";
              std::cout << HashLoc.printToString(SM) << ":" << "\n";

              XMLDocOut.XMLAddNode(SM, HashLoc, "20.5", "errno shall not be used : ");
              JSONDocOUT.JSONAddElement(SM, HashLoc, "20.5", "errno shall not be used : ");
            }
          }
        }

        if (FileName == "time.h")
        {
          if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, SM, HashLoc))
          {
            /*intentionally left blank*/
          }
          else
          {
            if (Devi::IsTheMatchInMainFile(MainFileOnly, SM, HashLoc))
            {
              std::cout << "20.12:" << "stdlib time.h is included in the project. use is forbidden:";
              std::cout << HashLoc.printToString(SM) << ":" << "\n";

              XMLDocOut.XMLAddNode(SM, HashLoc, "20.12", "stdlib time.h is included in the project. use is forbidden : ");
              JSONDocOUT.JSONAddElement(SM, HashLoc, "20.12", "stdlib time.h is included in the project. use is forbidden : ");
            }
          }
        }

        if (FileName == "stdio.h")
        {
          if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, SM, HashLoc))
          {
            /*intentionally left blank*/
          }
          else
          {
            if (Devi::IsTheMatchInMainFile(MainFileOnly, SM, HashLoc))
            {
              std::cout << "20.9:" << "stdlib stdio.h is included in the project. use is forbidden:";
              std::cout << HashLoc.printToString(SM) << ":" << "\n";

              XMLDocOut.XMLAddNode(SM, HashLoc, "20.9", "stdlib stdio.h is included in the project. use is forbidden : ");
              JSONDocOUT.JSONAddElement(SM, HashLoc, "20.9", "stdlib stdio.h is included in the project. use is forbidden : ");
            }
          }
        }

        if (FileName == "signal.h")
        {
          if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, SM, HashLoc))
          {
            /*intentionally left blank*/
          }
          else
          {
            if (Devi::IsTheMatchInMainFile(MainFileOnly, SM, HashLoc))
            {
              std::cout << "20.8:" << "stdlib signal.h is included in the project. use is forbidden:";
              std::cout << HashLoc.printToString(SM) << ":" << "\n";

              XMLDocOut.XMLAddNode(SM, HashLoc, "20.8", "stdlib signal.h is included in the project. use is forbidden : ");
              JSONDocOUT.JSONAddElement(SM, HashLoc, "20.8", "stdlib signal.h is included in the project. use is forbidden : ");
            }
          }
        }
      }
      else
      {
        size_t singleQPos = FileName.find("\'", 0);
        size_t whateverSlashPos = FileName.find("\\", 0);
        size_t commentPos = FileName.find("\\*", 0);

        if (singleQPos != std::string::npos || whateverSlashPos != std::string::npos || commentPos != std::string::npos)
        {
          if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, SM, HashLoc))
          {
            /*intentionally left blank*/
          }
          else
          {
            if (Devi::IsTheMatchInMainFile(MainFileOnly, SM, HashLoc))
            {
              std::cout << "19.2:" << "illegal characters in inclusion directive:";
              std::cout << HashLoc.printToString(SM) << "\n" << "\n";

              XMLDocOut.XMLAddNode(SM, HashLoc, "19.2", "illegal characters in inclusion directive : ");
              JSONDocOUT.JSONAddElement(SM, HashLoc, "19.2", "illegal characters in inclusion directive : ");
            }
          }
        }

        bool IsNewIncludeFile = true;

        for (unsigned x = 0; x < IncludeFileArr.size(); ++x)
        {
          if (SearchPath.str() + "/" + FileName.str() == IncludeFileArr[x])
          {
            IsNewIncludeFile = false;
            break;
          }
        }

        /*its supposed to supprt linux and cygwin,mingw and mac builds.*/
        if (IsNewIncludeFile)
        {
#if defined(__linux__)
          IncludeFileArr.push_back(SearchPath.str() + "/" + FileName.str());
#elif defined(__MACH__) && defined(__APPLE__)
          IncludeFileArr.push_back(SearchPath.str() + "/" + FileName.str());
#elif defined(__CYGWIN__) || defined(_WIN32) || defined(_WIN64)
          IncludeFileArr.push_back(SearchPath.str() + "\\" + FileName.str());
#else
          IncludeFileArr.push_back(SearchPath.str() + "/" + FileName.str());
#endif
        }
      }

      size_t whateverSlashPos = FileName.find("\\", 0);
      size_t theotherSlashPos = FileName.find(" / ", 0);

      if (whateverSlashPos != std::string::npos || theotherSlashPos != std::string::npos)
      {
        if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, SM, HashLoc))
        {
          /*intentionally left blank*/
        }
        else
        {
          if (Devi::IsTheMatchInMainFile(MainFileOnly, SM, HashLoc))
          {
            std::cout << "19.3:" << "Include directive contains file address, not just name:";
            std::cout << HashLoc.printToString(SM) << ":" << "\n";

            XMLDocOut.XMLAddNode(SM, HashLoc, "19.3", "Include directive contains file address, not just name : ");
            JSONDocOUT.JSONAddElement(SM, HashLoc, "19.3", "Include directive contains file address, not just name : ");
          }
        }
      }
#endif
    }
  }

  /*@DEVI-if the macro is not checked for being defined before almost any kind of use, the code will break in seemingly random ways.*/
  /*@DEVI-FIXME-the macro definition is the definition of the macro passed to defined. not sure what happens if there are more than two.
  basically i dont know how to just get the tokens after defined.*/
  virtual void Defined(const Token &MacroNameTok, const MacroDefinition &MD, SourceRange Range)
  {
#if 1
    SourceLocation SL [[maybe_unused]] = Range.getBegin(); 
    CheckSLValidity(SL);

#if 0
    assert(SL.isValid(), "the SourceLocation for macro Defined is not valid.");
#endif

    const MacroInfo* MI = MD.getMacroInfo();

    DefMacroDirective* DMD = MD.getLocalDirective();

    bool ShouldBeTagged194 = false;

#if 1
    if (DMD)
    {
      if (DMD->isDefined())
      {
        if (!MI->tokens_empty())
        {
          ArrayRef<Token> TokenArrayRef = MI->tokens();

          unsigned NumOfTokens = MI->getNumTokens();

          if (NumOfTokens == 1U)
          {
            if (!(TokenArrayRef[0].getKind() == tok::identifier))
            {
              ShouldBeTagged194 = true;
            }
          }
          else if (NumOfTokens == 3U)
          {
            if (!(TokenArrayRef[0].getKind() == tok::l_paren && TokenArrayRef[1].getKind() == tok::identifier && TokenArrayRef[2].getKind() == tok::r_paren))
            {
              ShouldBeTagged194 = true;
            }
          }
          else
          {
            ShouldBeTagged194 = true;
          }
        }
        else
        {
          ShouldBeTagged194 = true;
        }

        if (ShouldBeTagged194)
        {
#if 0
          std::cout << "19.14 : " << "Illegal \"defined\" form : " << "\n";
          std::cout << SL.printToString(SM) << "\n" << "\n";

          XMLDocOut.XMLAddNode(SM, SL, "19.14", "Illegal \"defined\" form : ");
#endif
        }
      }
    }
#endif
#endif
  }

  virtual void MacroUndefined(const Token &MacroNameTok, const MacroDefinition &MD)
  {
#if 1
    const MacroInfo* MI = MD.getMacroInfo();

    DefMacroDirective* DMD = MD.getLocalDirective();

    if (MI != nullptr && DMD != nullptr)
    {
      SourceLocation SL = MacroNameTok.getLocation(); 
      CheckSLValidity(SL);

#if 0
      assert(SL.isValid(), "the SourceLocation for MacroUndefined is not valid.");
#endif

      /*start of 20.1*/
      /*inline and restrict are C99*/
      if (MacroNameTok.isOneOf(tok::kw_auto, tok::kw_break, tok::kw_case, tok::kw_char, tok::kw_const, tok::kw_continue, \
                               tok::kw_default, tok::kw_do, tok::kw_double, tok::kw_else, tok::kw_enum, tok::kw_extern, \
                               tok::kw_float, tok::kw_for, tok::kw_goto, tok::kw_if, tok::kw_inline, tok::kw_int, tok::kw_long, \
                               tok::kw_register, tok::kw_restrict, tok::kw_return, tok::kw_short, tok::kw_signed, tok::kw_sizeof, \
                               tok::kw_static, tok::kw_struct, tok::kw_switch, \
                               tok::kw_typedef, tok::kw_union, tok::kw_unsigned, tok::kw_void, tok::kw_volatile, tok::kw_while))
      {
        if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, SM, SL))
        {
          /*intentionally left blank*/
        }
        else
        {
          if (Devi::IsTheMatchInMainFile(MainFileOnly, SM, SL))
          {
            std::cout << "20.1:" << "C keyword undefined:";
            std::cout << SL.printToString(SM) << ":" << "\n";

            XMLDocOut.XMLAddNode(SM, SL, "20.1", "C keyword undefined : ");
            JSONDocOUT.JSONAddElement(SM, SL, "20.1", "C keyword undefined : ");
          }
        }
      }

      if (DMD->getPrevious() != nullptr)
      {
        const MacroDirective* PMD = DMD->getPrevious();

        if (PMD != nullptr)
        {
          SourceLocation PSL = PMD->getLocation();
          CheckSLValidity(PSL);

          if (SM.isInSystemHeader(PSL) || MI->isBuiltinMacro())
          {
            if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, SM, SL))
            {
              /*intentionally left blank*/
            }
            else
            {
              if (Devi::IsTheMatchInMainFile(MainFileOnly, SM, SL))
              {
                std::cout << "20.1:" << "C standard library macro undefined:";
                std::cout << SL.printToString(SM) << ":" << "\n";

                XMLDocOut.XMLAddNode(SM, SL, "20.1", "C standard library macro undefined : ");
                JSONDocOUT.JSONAddElement(SM, SL, "20.1", "C standard library macro undefined : ");
              }
            }
          }
        }
      }
      /*end of 20.1*/

      /*start of 19.5*/
      if (!MI->isBuiltinMacro() && SM.isInMainFile(SL) && !SM.isInSystemHeader(SL))
      {
        MacroUndefSourceLocation.push_back(SL);
      }
      /*end of 19.5*/

      if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, SM, SL))
      {
        /*intentionally left blank*/
      }
      else
      {
        if (Devi::IsTheMatchInMainFile(MainFileOnly, SM, SL))
        {
          std::cout << "19.6:" << "Use of #undef is illegal:";
          std::cout << SL.printToString(SM) << ":" << "\n";

          XMLDocOut.XMLAddNode(SM, SL, "19.6", "Use of #undef is illegal : ");
          JSONDocOUT.JSONAddElement(SM, SL, "19.6", "Use of #undef is illegal : ");
        }
      }
    }
#endif
  }

  virtual void MacroDefined(const Token &MacroNameTok, const MacroDirective *MD)
  {
#if 1
    const MacroInfo* MI = MD->getMacroInfo();

    SourceLocation SL = MacroNameTok.getLocation(); 
    CheckSLValidity(SL);

#if 0
    assert(SL->isValid(), "the SourceLocation for MacroDefined is not valid.");
#endif

    unsigned MacroNumArgs = MI->getNumArgs();

    /*start of 19.5*/
    if (!MI->isBuiltinMacro() && SM.isInMainFile(SL) && !SM.isInSystemHeader(SL))
    {
      MacroDefSourceLocation.push_back(SM.getExpansionLoc(SL));
      MacroNameString.push_back(MacroNameTok.getIdentifierInfo()->getName().str());
    }
    /*end of 19.5*/

    /*start of 20.1*/
    /*inline and restrict are C99*/
    if (MacroNameTok.isOneOf(tok::kw_auto, tok::kw_break, tok::kw_case, tok::kw_char, tok::kw_const, tok::kw_continue, \
                             tok::kw_default, tok::kw_do, tok::kw_double, tok::kw_else, tok::kw_enum, tok::kw_extern, \
                             tok::kw_float, tok::kw_for, tok::kw_goto, tok::kw_if, tok::kw_inline, tok::kw_int, tok::kw_long, \
                             tok::kw_register, tok::kw_restrict, tok::kw_return, tok::kw_short, tok::kw_signed, tok::kw_sizeof, \
                             tok::kw_static, tok::kw_struct, tok::kw_switch, \
                             tok::kw_typedef, tok::kw_union, tok::kw_unsigned, tok::kw_void, tok::kw_volatile, tok::kw_while))
    {
      if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, SM, SL))
      {
        /*intentionally left blank*/
      }
      else
      {
        if (Devi::IsTheMatchInMainFile(MainFileOnly, SM, SL))
        {
          std::cout << "20.1:" << "C keyword defined:";
          std::cout << SL.printToString(SM) << ":" << "\n";

          XMLDocOut.XMLAddNode(SM, SL, "20.1", "C keyword defined : ");
          JSONDocOUT.JSONAddElement(SM, SL, "20.1", "C keyword defined : ");
        }
      }
    }

    if (MD->getPrevious() != nullptr)
    {
      const MacroDirective* PMD = MD->getPrevious();
      SourceLocation PSL = PMD->getLocation();
      /*@DEVI-A quick fix.Fixme.*/
      CheckSLValidity(PSL);

      if (SM.isInSystemHeader(PSL) || MI->isBuiltinMacro())
      {
        if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, SM, SL))
        {
          /*intentionally left blank*/
        }
        else
        {
          if (Devi::IsTheMatchInMainFile(MainFileOnly, SM, SL))
          {
            std::cout << "20.1:" << "C standard library macro redefined:";
            std::cout << SL.printToString(SM) << ":" << "\n";

            XMLDocOut.XMLAddNode(SM, SL, "20.1", "C standard library macro redefined : ");
            JSONDocOUT.JSONAddElement(SM, SL, "20.1", "C standard library macro redefined : ");
          }
        }
      }
    }
    /*end of 20.1*/

    ArrayRef<Token> TokenArrayRef = MI->tokens();
    ArrayRef<const IdentifierInfo*> MacroArgsArrRef = MI->args();

    unsigned NumOfTokens = MI->getNumTokens();

    bool hasSingleHash = false;
    bool hasDoubleHash = false;

    for (unsigned x = 0; x < NumOfTokens; ++x)
    {
#if 1
      if (TokenArrayRef[x].getKind() == tok::hash)
      {
        hasSingleHash = true;

        if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, SM, SL))
        {
          /*intentionally left blank*/
        }
        else
        {
          if (Devi::IsTheMatchInMainFile(MainFileOnly, SM, SL))
          {
            std::cout << "19.13:" << "Macro has # token:";
            std::cout << SL.printToString(SM) << ":" << "\n";

            XMLDocOut.XMLAddNode(SM, SL, "19.13", "Macro has # token : ");
            JSONDocOUT.JSONAddElement(SM, SL, "19.13", "Macro has # token : ");
          }
        }
      }

      if (TokenArrayRef[x].getKind() == tok::hashhash)
      {
        hasDoubleHash = true;

        if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, SM, SL))
        {
          /*intentionally left blank*/
        }
        else
        {
          if (Devi::IsTheMatchInMainFile(MainFileOnly, SM, SL))
          {
            std::cout << "19.13:" << "Macro has ## token:";
            std::cout << SL.printToString(SM) << ":" << "\n";

            XMLDocOut.XMLAddNode(SM, SL, "19.13", "Macro has ## token : ");
            JSONDocOUT.JSONAddElement(SM, SL, "19.13", "Macro has ## token : ");
          }
        }
      }
#endif
    }

    if (hasSingleHash && hasDoubleHash)
    {
      if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, SM, SL))
      {
        /*intentionally left blank*/
      }
      else
      {
        if (Devi::IsTheMatchInMainFile(MainFileOnly, SM, SL))
        {
          std::cout << "19.12:" << "Macro has # and ## tokens:";
          std::cout << SL.printToString(SM) << ":" << "\n";

          XMLDocOut.XMLAddNode(SM, SL, "19.12", "Macro has # and ## tokens : ");
          JSONDocOUT.JSONAddElement(SM, SL, "19.12", "Macro has # and ## tokens : ");
        }
      }
    }

    if (MI->isFunctionLike())
    {
      bool ShouldBeTagged = false;
      bool IsIdentifierMacroArg = false;
      bool HasHash = false;

      for (unsigned x = 0U; x < NumOfTokens; ++x)
      {
#if 1
        /*@DEVI-for macro defs that dont have more than two token NumOfTokens will wrap around since its
        unsigned if subtracted by two,hence the check. it does not hurt the logic since if there are less
        than two token in a macro definition, then it cannot possibly have a hash or a double hash.*/
        if (NumOfTokens >= 2U)
        {
          if (TokenArrayRef[x].getKind() == tok::identifier)
          {
            for (unsigned  xx = 0; xx < MacroNumArgs; ++xx)
            {
              if (TokenArrayRef[x].getIdentifierInfo()->getName().str() == MacroArgsArrRef[xx]->getName().str())
              {
                IsIdentifierMacroArg = true;
              }
            }

            if (IsIdentifierMacroArg)
            {
              if (x <= NumOfTokens - 2U)
              {
                if (TokenArrayRef[x + 1U].getKind() == tok::hashhash)
                {
                  HasHash = true;
                }
              }

              if (x >= 1U)
              {
                if (TokenArrayRef[x - 1U].getKind() == tok::hash || TokenArrayRef[x - 1U].getKind() == tok::hashhash)
                {
                  HasHash = true;
                }
              }

              if (x <= NumOfTokens - 2U)
              {
                if (!(TokenArrayRef[x + 1U].getKind() == tok::r_paren) && !HasHash)
                {
                  ShouldBeTagged = true;
                }
              }

              if (x >= 1U)
              {
                if (!(TokenArrayRef[x - 1U].getKind() == tok::l_paren) && !HasHash)
                {
                  ShouldBeTagged = true;
                }
              }
            }
          }

          IsIdentifierMacroArg = false;
          HasHash = false;
        }
#endif
      }

      if (ShouldBeTagged)
      {
        if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, SM, SL))
        {
          /*intentionally left blank*/
        }
        else
        {
          if (Devi::IsTheMatchInMainFile(MainFileOnly, SM, SL))
          {
            std::cout << "19.10:" << "Funciton-like macro's parameters are not enclosed in parantheses or dont have hash:";
            std::cout << SL.printToString(SM) << ":" << "\n";

            XMLDocOut.XMLAddNode(SM, SL, "19.10", "Funciton-like macro's parameters are not enclosed in parantheses or dont have hash : ");
            JSONDocOUT.JSONAddElement(SM, SL, "19.10", "Funciton-like macro's parameters are not enclosed in parantheses or dont have hash : ");
          }
        }
      }

      if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, SM, SL))
      {
        /*intentionally left blank*/
      }
      else
      {
        if (Devi::IsTheMatchInMainFile(MainFileOnly, SM, SL))
        {
          if (Devi::IsTheMatchInMainFile(MainFileOnly, SM, SL))
          {
            std::cout << "19.7:" << "Function-like macro used:";
            std::cout << SL.printToString(SM) << ":" << "\n";

            XMLDocOut.XMLAddNode(SM, SL, "19.7", "Function-like macro used : ");
            JSONDocOUT.JSONAddElement(SM, SL, "19.7", "Function-like macro used : ");
          }
        }
      }

      if (MacroNumArgs != 0)
      {
        for (unsigned x = 0; x < MacroNumArgs; ++x)
        {
          if (MacroArgsArrRef[0]->hasMacroDefinition())
          {
            if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, SM, SL))
            {
              /*intentionally left blank*/
            }
            else
            {
              if (Devi::IsTheMatchInMainFile(MainFileOnly, SM, SL))
              {
                std::cout << "19.9:" << "Function-like macro's argument contains macros:";
                std::cout << SL.printToString(SM) << ":" << "\n";

                XMLDocOut.XMLAddNode(SM, SL, "19.9", "Function-like macro's argument contains macros : ");
                JSONDocOUT.JSONAddElement(SM, SL, "19.9", "Function-like macro's argument contains macros : ");
              }
            }

            break;
          }
        }
      }
    }
#endif
  }

  virtual void MacroExpands(const Token &MacroNameTok, const MacroDefinition &MD, SourceRange Range, const MacroArgs *Args)
  {
#if 1
    SourceLocation SL = MacroNameTok.getLocation();
    CheckSLValidity(SL);

#if 0
    assert(SL.isValid(), "the SourceLocation for MacroExpands is not valid.");
#endif

    IdentifierInfo* II = MacroNameTok.getIdentifierInfo();

    std::string MacroNameString = II->getName().str();

    DefMacroDirective* DMD = MD.getLocalDirective();

    SourceLocation MDSL = DMD->getLocation();

    MacroInfo* MI = MD.getMacroInfo();

    /*start of 19.4*/
    ArrayRef<Token> TokenArrayRef = MI->tokens();

    /*@DEVI-guard against macro defs that just define the macro without a value.*/
    if (TokenArrayRef.size() != 0U)
    {
      bool MacroExpansionIsTypeQualifier = true;
      bool MacroExpansionIsStorageSpecifier = true;
      bool MacroExpansionStringLiteral = false;
      bool MacroExpansionConstant = false;
      bool MacroExpansionBracedInitializer = true;
      bool MacroExpansionParenExpr = true;
      bool MacroExpansionDoWhileZero = true;

      if (TokenArrayRef.front().getKind() == tok::l_paren && TokenArrayRef.back().getKind() == tok::r_paren)
      {
        /*currently we do not care what's inside the parens.*/
      }
      else
      {
        MacroExpansionParenExpr = false;
      }

      if (TokenArrayRef.front().getKind() == tok::l_brace && TokenArrayRef.back().getKind() == tok::r_brace)
      {
        /*currently we don't care what's inside the curly braces.*/
      }
      else
      {
        MacroExpansionBracedInitializer = false;
      }

      //std::vector<tok::TokenKind> TokenPattern = {tok::kw_while, tok::l_paren, tok::numeric_constant, tok::r_paren};
      std::vector<tok::TokenKind> TokenPattern;
      TokenPattern.push_back(tok::kw_while);
      TokenPattern.push_back(tok::l_paren);
      TokenPattern.push_back(tok::numeric_constant);
      TokenPattern.push_back(tok::r_paren);

      if (TokenArrayRef.front().getKind() == tok::kw_do)
      {
        unsigned marker = 0U;

        for (ArrayRef<Token>::iterator iter = TokenArrayRef.begin(), iterE = TokenArrayRef.end(); iter != iterE; ++iter)
        {
          if (iter->getKind() == tok::kw_while)
          {
            marker = 0U;
          }

          if (marker == 3U && iter->getKind() == TokenPattern[3])
          {
            marker = 4U;
          }
          else if (marker == 3U && iter->getKind() != TokenPattern[3])
          {
            marker = 0U;
          }
          else
          {
            /*empty*/
          }

          if (marker == 2U && iter->getKind() == TokenPattern[2])
          {
            marker = 3U;
          }
          else if (marker == 2U && iter->getKind() != TokenPattern[2])
          {
            marker = 0U;
          }
          else
          {
            /*empty*/
          }

          if (marker == 1U && iter->getKind() == TokenPattern[1])
          {
            marker = 2U;
          }
          else if (marker == 1U && iter->getKind() != TokenPattern[1])
          {

          }
          else
          {
            /*empty*/
          }

          if (marker == 0U && iter->getKind() == TokenPattern[0])
          {
            marker = 1U;
          }
        }

        if (marker != 4U)
        {
          MacroExpansionDoWhileZero = false;
        }
      }
      else
      {
        MacroExpansionDoWhileZero = false;
      }

      if (TokenArrayRef.size() == 1U)
      {
        if (TokenArrayRef[0].getKind() == tok::string_literal || TokenArrayRef[0].getKind() == tok::wide_string_literal \
            || TokenArrayRef[0].getKind() == tok::utf8_string_literal || TokenArrayRef[0].getKind() == tok::utf16_string_literal \
            || TokenArrayRef[0].getKind() == tok::utf32_string_literal)
        {
          MacroExpansionStringLiteral = true;
        }

        if (TokenArrayRef[0].getKind() == tok::numeric_constant || TokenArrayRef[0].getKind() == tok::char_constant \
            || TokenArrayRef[0].getKind() == tok::wide_char_constant || TokenArrayRef[0].getKind() == tok::utf8_char_constant \
            || TokenArrayRef[0].getKind() == tok::utf16_char_constant || TokenArrayRef[0].getKind() == tok::utf32_char_constant)
        {
          MacroExpansionConstant = true;
        }
      }

      for (auto &iter : TokenArrayRef)
      {
        if (iter.getKind() == tok::kw_const || iter.getKind() == tok::kw_restrict || iter.getKind() == tok::kw_volatile \
            || iter.getKind() == tok::l_paren || iter.getKind() == tok::r_paren)
        {
          /*has no significance*/
        }
        else
        {
          MacroExpansionIsTypeQualifier = false;
        }

        if (iter.getKind() == tok::kw_auto || iter.getKind() == tok::kw_extern || iter.getKind() == tok::kw_register \
            || iter.getKind() == tok::kw_static || iter.getKind() == tok::kw_typedef \
            || iter.getKind() == tok::l_paren || iter.getKind() == tok::r_paren)
        {
          /*has no significance*/
        }
        else
        {
          MacroExpansionIsStorageSpecifier = false;
        }
      }

      if (!MacroExpansionIsTypeQualifier && !MacroExpansionIsStorageSpecifier \
          && !MacroExpansionStringLiteral && !MacroExpansionConstant \
          && !MacroExpansionBracedInitializer && !MacroExpansionParenExpr \
          && !MacroExpansionDoWhileZero)
      {
        if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, SM, MDSL))
        {
          /*intentionally left blank*/
        }
        else
        {
          if (Devi::IsTheMatchInMainFile(MainFileOnly, SM, MDSL))
          {
            std::cout << "19.4:" << "Macro does not expand to braced initializer,panthesizes expression,string literal,constant,do-while-zero,storage class specifier or type qualifier:";
            std::cout << Range.getBegin().printToString(SM) << ":" << "\n";

            XMLDocOut.XMLAddNode(SM, SL, "19.4", "Macro does not expand to braced initializer,panthesizes expression,string literal,constant,do-while-zero,storage class specifier or type qualifier : ");
            JSONDocOUT.JSONAddElement(SM, SL, "19.4", "Macro does not expand to braced initializer,panthesizes expression,string literal,constant,do-while-zero,storage class specifier or type qualifier : ");
          }
        }
      }
    }
    /*end of 19.4*/

#if 1
    if (Args != nullptr)
    {
      /*@DEVI-Macro args are passed twice. first they are expanded and then the whole macro,
      including the args is checked again for expansion, so args are passed twice.*/
      if (MI->getNumArgs() != Args->getNumArguments() - MI->getNumArgs())
      {
        if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, SM, MDSL))
        {
          /*intentionally left blank*/
        }
        else
        {
          if (Devi::IsTheMatchInMainFile(MainFileOnly, SM, MDSL))
          {
            std::cout << "19.8:" << "Funciton-like macro invoked with wrong number of arguments:";
            std::cout << Range.getBegin().printToString(SM) << ":" << Args->getNumArguments() << " " << MI->getNumArgs() << ":" << "\n";

            XMLDocOut.XMLAddNode(SM, SL, "19.8", "Funciton-like macro invoked with wrong number of arguments:");
            JSONDocOUT.JSONAddElement(SM, SL, "19.8", "Funciton-like macro invoked with wrong number of arguments:");
          }
        }
      }
    }
#endif

    if (MacroNameString == "offsetof")
    {
      if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, SM, MDSL))
      {
        /*intentionally left blank*/
      }
      else
      {
        if (Devi::IsTheMatchInMainFile(MainFileOnly, SM, MDSL))
        {
          std::cout << "20.6:" << "use of offsetof is illegal:";
          std::cout << Range.getBegin().printToString(SM) << ":" << "\n";

          XMLDocOut.XMLAddNode(SM, SL, "20.6", "use of offsetof is illegal : ");
          JSONDocOUT.JSONAddElement(SM, SL, "20.6", "use of offsetof is illegal : ");
        }
      }
    }

    if (MacroNameString == "setjmp")
    {
      if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, SM, MDSL))
      {
        /*intentionally left blank*/
      }
      else
      {
        if (Devi::IsTheMatchInMainFile(MainFileOnly, SM, MDSL))
        {
          std::cout << "20.7:" << "use of setjmp is illegal:";
          std::cout << Range.getBegin().printToString(SM) << ":" << "\n";

          XMLDocOut.XMLAddNode(SM, SL, "20.7", "use of setjmp is illegal : ");
          JSONDocOUT.JSONAddElement(SM, SL, "20.7", "use of setjmp is illegal : ");
        }
      }
    }

    if (!DMD->isDefined())
    {
      if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, SM, MDSL))
      {
        /*intentionally left blank*/
      }
      else
      {
        /*@DEVI-by the time we get a callback on our callback, the macri is assigned a default vlaue even if it is undefined in the TU.*/
        if (Devi::IsTheMatchInMainFile(MainFileOnly, SM, MDSL))
        {
          std::cout << "19.11:" << "Use of undefined macro:";
          std::cout << Range.getBegin().printToString(SM) << ":" << "\n";

          XMLDocOut.XMLAddNode(SM, SL, "19.11", "Use of undefined macro : ");
          JSONDocOUT.JSONAddElement(SM, SL, "19.11", "Use of undefined macro : ");
        }
      }
    }
#endif
  }

  virtual void Elif(SourceLocation Loc, SourceRange ConditionRange, ConditionValueKind ConditionValue, SourceLocation IfLoc)
  {
#if 1
    SourceLocation SLoc = SM.getSpellingLoc(Loc); 
    CheckSLValidity(SLoc);
    SourceLocation SIfLoc = SM.getSpellingLoc(IfLoc); 
    CheckSLValidity(SIfLoc);

    if (SM.getFileID(SLoc) != SM.getFileID(SIfLoc))
    {
      if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, SM, Loc))
      {
        /*intentionally left blank*/
      }
      else
      {
        if (Devi::IsTheMatchInMainFile(MainFileOnly, SM, Loc))
        {
          std::cout << "19.17:" << "elif directive is not in the same file as its if directive:";
          std::cout << SLoc.printToString(SM) << ":" << "\n";

          XMLDocOut.XMLAddNode(SM, SLoc, "19.17", "elif directive is not in the same file as its if directive : ");
          JSONDocOUT.JSONAddElement(SM, SLoc, "19.17", "elif directive is not in the same file as its if directive : ");
        }
      }
    }
#endif
  }

  virtual void Else(SourceLocation Loc, SourceLocation IfLoc)
  {
#if 1
    SourceLocation SLoc = SM.getSpellingLoc(Loc);
    CheckSLValidity(SLoc);
    SourceLocation SIfLoc = SM.getSpellingLoc(IfLoc);
    CheckSLValidity(SIfLoc);

    if (SM.getFileID(SLoc) != SM.getFileID(SIfLoc))
    {
      if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, SM, Loc))
      {
        /*intentionally left blank*/
      }
      else
      {
        if (Devi::IsTheMatchInMainFile(MainFileOnly, SM, Loc))
        {
          std::cout << "19.17:" << "else directive is not in the same file as its if directive:";
          std::cout << SLoc.printToString(SM) << ":" << "\n";

          XMLDocOut.XMLAddNode(SM, SLoc, "19.17", "else directive is not in the same file as its if directive : ");
          JSONDocOUT.JSONAddElement(SM, SLoc, "19.17", "else directive is not in the same file as its if directive : ");
        }
      }
    }
#endif
  }

  virtual void Endif (SourceLocation Loc, SourceLocation IfLoc)
  {
#if 1
    SourceLocation SLoc = SM.getSpellingLoc(Loc); 
    CheckSLValidity(SLoc);
    SourceLocation SIfLoc = SM.getSpellingLoc(IfLoc); 
    CheckSLValidity(SIfLoc);

    if (SM.getFileID(SLoc) != SM.getFileID(SIfLoc))
    {
      if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, SM, Loc))
      {
        /*intentionally left blank*/
      }
      else
      {
        if (Devi::IsTheMatchInMainFile(MainFileOnly, SM, Loc))
        {
          std::cout << "19.17:" << "endif directive is not in the same file as its if directive:";
          std::cout << SLoc.printToString(SM) << ":" << "\n";

          XMLDocOut.XMLAddNode(SM, SLoc, "19.17", "endif directive is not in the same file as its if directive : ");
          JSONDocOUT.JSONAddElement(SM, SLoc, "19.17", "endif directive is not in the same file as its if directive : ");
        }
      }
    }
#endif
  }

private:
  const SourceManager &SM;
};
/**********************************************************************************************************************/
class IsThereJunkPreInclusion
{
public:
  IsThereJunkPreInclusion() {}

  void Check(std::vector<std::string> SourcePathList)
  {
    bool HaveWeMatchedInclusionDirYet = false;
    bool HaveWeMatchIllegal191Yet = false;

    for (auto &iter : SourcePathList)
    {
      //std::cout << iter << "\n";
      std::ifstream InputFile(iter);

      HaveWeMatchIllegal191Yet = false;
      HaveWeMatchedInclusionDirYet = false;

      for (std::string line; getline(InputFile, line);)
      {
        //std::cout << iter << ":" << line << ":" << HaveWeMatchedInclusionDirYet << " " << HaveWeMatchIllegal191Yet << "\n";

        if (line.empty())
        {
          continue;
        }

        if (line.front() == '#')
        {
          size_t st = line.find("#include", 0U);

          if (st == 0U)
          {
            /*we've found a header include*/
            HaveWeMatchedInclusionDirYet = true;

            if (HaveWeMatchIllegal191Yet)
            {
              /*print diag out*/
              std::cout << "19.1" << ":" << "Inclusion directives should only be preceeded by other inclusion directives, pp directives or comments" << ":" << iter << "\n";

              XMLDocOut.XMLAddNode(iter, "19.1", "Inclusion directives should only be preceeded by other inclusion directives, pp directives or comments : ");
              JSONDocOUT.JSONAddElement(iter, "19.1", "Inclusion directives should only be preceeded by other inclusion directives, pp directives or comments : ");
              break;
            }
            else
            {
              break;
            }
          }

          continue;
        }

        if (line.front() == '/')
        {
          /*has to be a comment*/
          continue;
        }

        if (line.front() == '\n' || line.front() == '\t' || line.front() == ' ' || line.front() == '\r')
        {
          HaveWeMatchIllegal191Yet = false;

          for (auto &iterchar : line)
          {
            if (iterchar == '\n' || iterchar == '\t' || iterchar == ' ' || line.front() == '\r')
            {
              continue;
            }

            if (iterchar == '/')
            {
              break;
            }

            if (iterchar == '#')
            {
              size_t st = line.find("#include", 0U);

              if (st == 0U)
              {
                /*we've found a header include*/
                HaveWeMatchedInclusionDirYet = true;

                if (HaveWeMatchIllegal191Yet)
                {
                  /*print diag out*/
                  std::cout << "19.1" << ":" << "Inclusion directives should only be preceeded by other inclusion directives, pp directives or comments" << ":" << iter << "\n";

                  XMLDocOut.XMLAddNode(iter, "19.1", "Inclusion directives should only be preceeded by other inclusion directives, pp directives or comments : ");
                  JSONDocOUT.JSONAddElement(iter, "19.1", "Inclusion directives should only be preceeded by other inclusion directives, pp directives or comments : ");
                  break;
                }
                else
                {
                  break;
                }


              }
            }

            HaveWeMatchIllegal191Yet = true;
          }

          continue;
        }

        HaveWeMatchIllegal191Yet = true;
      }

      InputFile.close();
    }
  }

private:
};
/**********************************************************************************************************************/
class CheckForNullStatements
{
public:
  CheckForNullStatements() {}

  void Check(void)
  {
    bool HaveWeMatchedASemi = false;
    bool ShouldBeTagged = false;
    bool HaveWeSeenAComment = false;
    bool WhiteSpacePostSemi = false;

    for (auto &iter : NullStmtProto)
    {
#if 0
      std::cout << iter.Line << ":" << iter.Column << ":" << iter.FileName << "\n";
#endif

      ShouldBeTagged = false;
      HaveWeMatchedASemi = false;
      HaveWeSeenAComment = false;
      WhiteSpacePostSemi = false;

      std::ifstream InputFile(iter.FileName);

      unsigned counter = 0U;

      for (std::string line; getline(InputFile, line);)
      {
        counter++;
        if (counter == iter.Line)
        {
          for (auto &iterchar : line)
          {
            if (iterchar == ';')
            {
              if (HaveWeMatchedASemi)
              {
                ShouldBeTagged = true;
                break;
              }

              HaveWeMatchedASemi = true;
              continue;
            }

            if (iterchar == ' ')
            {
              if (HaveWeMatchedASemi)
              {
                WhiteSpacePostSemi = true;
                continue;
              }

              if (WhiteSpacePostSemi)
              {
                ShouldBeTagged = true;
                break;
              }

              continue;
            }

            if (iterchar == '\t')
            {

              if (HaveWeMatchedASemi)
              {
                ShouldBeTagged = true;
                break;
              }
              else
              {
                continue;
              }
            }

            if (iterchar == '/')
            {
              HaveWeSeenAComment = true;

              if (HaveWeMatchedASemi)
              {
                if (WhiteSpacePostSemi)
                {
                  break;
                }
                else
                {
                  ShouldBeTagged = true;
                  break;
                }
              }
              else
              {
                ShouldBeTagged = true;
                break;
              }

              break;
            }

            ShouldBeTagged = true;
            break;
          }
        }

        if (ShouldBeTagged)
        {
          if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, iter.IsInSysHeader))
          {
            /*intentionally left blank*/
          }
          else
          {
            if (Devi::IsTheMatchInMainFile(MainFileOnly, iter.IsInMainFile))
            {
              std::cout << "14.3" << ":" << "Illegal NullStmt form:" << iter.FileName << ":" << iter.Line << ":" << iter.Column << ":" << "\n";

              XMLDocOut.XMLAddNode(iter.Line, iter.Column, iter.FileName, "14.3", "Illegal NullStmt form:");
              JSONDocOUT.JSONAddElement(iter.Line, iter.Column, iter.FileName, "14.3", "Illegal NullStmt form:");
            }
          }

          break;
        }
      }

      InputFile.close();
    }
  }

private:
};
/**********************************************************************************************************************/
class onEndOfAllTUs
{
public: onEndOfAllTUs() {}

  static void run(void)
  {
    /*@DEVI-start of 8.8*/
    for (auto &iter : ExternObjInfoProto)
    {
      if (iter.HasMoreThanOneDefinition)
      {
        std::cout << "8.8:" << "External function or object is defined in more than one file:";
        std::cout << iter.XObjSLStr << ":" << iter.XObjNameStr << "\n";

        XMLDocOut.XMLAddNode(iter.LineNumber, iter.ColumnNumber, iter.FileName, "8.8", "External function or object is defined in more than one file: ");
        JSONDocOUT.JSONAddElement(iter.LineNumber, iter.ColumnNumber, iter.FileName, "8.8", "External function or object is defined in more than one file: ");
      }
    }
    /*end of 8.8*/

    /*@DEVI-start of 5.x*/
    /*@DEVI-first we need to do some cleanup and mark some entries as invalid.*/
    for (auto &iter : IdentInfoProto)
    {
      if (iter.Name == "")
      {
        iter.IsValid = false;
      }

      if (iter.NK == Devi::NodeKind::RecordDecl)
      {
        for (auto &yaiter : IdentInfoProto)
        {
          if (iter.Name == yaiter.Name && iter.SLString != yaiter.SLString)
          {
            if ((iter.IsIncomplete != yaiter.IsIncomplete) && iter.IsValid && yaiter.IsValid)
            {
              iter.IsValid = false;
            }
          }

          if (iter.Name == yaiter.Name && iter.SLString == yaiter.SLString && yaiter.NK == Devi::NodeKind::VarDecl)
          {
            yaiter.IsValid = false;
          }
        }
      }

      if (iter.NK == Devi::NodeKind::FieldDecl)
      {
        for (auto &yaiter : IdentInfoProto)
        {
          if (iter.Name == yaiter.Name && iter.SLString == yaiter.SLString && yaiter.NK == Devi::NodeKind::VarDecl)
          {
            yaiter.IsValid = false;
          }
        }
      }

      if (iter.NK == Devi::NodeKind::ParmVarDecl)
      {
        for (auto &yaiter : IdentInfoProto)
        {
          if (iter.Name == yaiter.Name && iter.SLString == yaiter.SLString && yaiter.NK == Devi::NodeKind::VarDecl)
          {
            yaiter.IsValid = false;
          }
        }
      }

      if (iter.NK == Devi::NodeKind::FunctionDecl)
      {
        for (auto &yaiter : IdentInfoProto)
        {
          if (iter.Name == yaiter.Name && iter.SLString != yaiter.SLString)
          {
            if (iter.FDKind != yaiter.FDKind && iter.IsValid && yaiter.IsValid)
            {
              iter.IsValid = false;
            }
          }
        }
      }
    }

    /*@DEVI-now we can start looking for things to tag*/
    for (auto &iter : IdentInfoProto)
    {
      if (iter.IsValid == false)
      {
        continue;
      }

      for (auto &yaiter : IdentInfoProto)
      {
        if (yaiter.IsValid == false)
        {
          continue;
        }

        if (iter.Name == yaiter.Name && iter.SLString != yaiter.SLString)
        {
          /*tag 5.7*/
          std::cout << "5.7:" << "Identifier re-used:";
          std::cout << iter.SLString << ":" << iter.Name << ":" << yaiter.SLString << "\n";

          XMLDocOut.XMLAddNode(iter.Line, iter.Column, iter.FileName, "5.7", "Identifier re-used:");
          JSONDocOUT.JSONAddElement(iter.Line, iter.Column, iter.FileName, "5.7", "Identifier re-used:");

          if (iter.NK == Devi::NodeKind::TypedefDecl)
          {
            /*tag 5.3*/
            std::cout << "5.3:" << "Typedef identifier is not unique:";
            std::cout << iter.SLString << ":" << iter.Name << ":" << yaiter.SLString << "\n";

            XMLDocOut.XMLAddNode(iter.Line, iter.Column, iter.FileName, "5.3", "Typedef identifier is not unique:");
            JSONDocOUT.JSONAddElement(iter.Line, iter.Column, iter.FileName, "5.3", "Typedef identifier is not unique:");
          }

          if (iter.NK == Devi::NodeKind::RecordDecl)
          {
            /*tag 5.4*/
            std::cout << "5.4:" << "Tag identifier is not unique:";
            std::cout << iter.SLString << ":" << iter.Name << ":" << yaiter.SLString << "\n";

            XMLDocOut.XMLAddNode(iter.Line, iter.Column, iter.FileName, "5.4", "Tag identifier is not unique:");
            JSONDocOUT.JSONAddElement(iter.Line, iter.Column, iter.FileName, "5.4", "Tag identifier is not unique:");
          }

          if (iter.NK == Devi::NodeKind::RecordDecl && yaiter.NK != Devi::NodeKind::RecordDecl)
          {
            /*tag 5.6*/
            std::cout << "5.6:" << "The Identifier is re-used in another namespace:";
            std::cout << iter.SLString << ":" << iter.Name << ":" << yaiter.SLString << "\n";

            XMLDocOut.XMLAddNode(iter.Line, iter.Column, iter.FileName, "5.6", "The Identifier is re-used in another namespace:");
            JSONDocOUT.JSONAddElement(iter.Line, iter.Column, iter.FileName, "5.6", "The Identifier is re-used in another namespace:");
          }

          if (iter.NK == Devi::NodeKind::LabelDecl && yaiter.NK != Devi::NodeKind::LabelDecl)
          {
            /*tag 5.6*/
            std::cout << "5.6:" << "The Identifier is re-used in another namespace:";
            std::cout << iter.SLString << ":" << iter.Name << ":" << yaiter.SLString << "\n";

            XMLDocOut.XMLAddNode(iter.Line, iter.Column, iter.FileName, "5.6", "The Identifier is re-used in another namespace:");
            JSONDocOUT.JSONAddElement(iter.Line, iter.Column, iter.FileName, "5.6", "The Identifier is re-used in another namespace:");
          }

          if (((iter.NK != Devi::NodeKind::RecordDecl) && (iter.NK != Devi::NodeKind::LabelDecl)) && \
              ((yaiter.NK == Devi::NodeKind::RecordDecl) || (yaiter.NK == Devi::NodeKind::LabelDecl)))
          {
            /*tag 5.6*/
            std::cout << "5.6:" << "The Identifier is re-used in another namespace:";
            std::cout << iter.SLString << ":" << iter.Name << ":" << yaiter.SLString << "\n";

            XMLDocOut.XMLAddNode(iter.Line, iter.Column, iter.FileName, "5.6", "The Identifier is re-used in another namespace:");
            JSONDocOUT.JSONAddElement(iter.Line, iter.Column, iter.FileName, "5.6", "The Identifier is re-used in another namespace:");
          }

          if (iter.FileName == yaiter.FileName && iter.Scope == Devi::Scope::Block && yaiter.Scope == Devi::Scope::TU)
          {
            /*tag 5.2*/
            std::cout << "5.2:" << "This identifier is being hidden by an identifier of the same name in file scope:";
            std::cout << iter.SLString << ":" << iter.Name << ":" << yaiter.SLString << "\n";

            XMLDocOut.XMLAddNode(iter.Line, iter.Column, iter.FileName, "5.2", "This identifier is being hidden by an identifier of the same name in file scope:");
            JSONDocOUT.JSONAddElement(iter.Line, iter.Column, iter.FileName, "5.2", "This identifier is being hidden by an identifier of the same name in file scope:");
          }

          if (iter.IsStatic)
          {
            /*tag 5.5*/
            std::cout << "5.5:" << "Identifier with static storage duration is re-used:";
            std::cout << iter.SLString << ":" << iter.Name << ":" << yaiter.SLString << "\n";

            XMLDocOut.XMLAddNode(iter.Line, iter.Column, iter.FileName, "5.5", "Identifier with static storage duration is re-used:");
            JSONDocOUT.JSONAddElement(iter.Line, iter.Column, iter.FileName, "5.5", "Identifier with static storage duration is re-used:");
          }
        }
      }
    }
    /*end of 5.x*/
  }

private:
};
/**********************************************************************************************************************/
class MyASTConsumer : public ASTConsumer {

public:
  MyASTConsumer(Rewriter &R) : HandlerForCmpless(R), HandlerWhileCmpless(R), HandlerElseCmpless(R), HandlerIfCmpless(R), \
    HandlerForIfElse(R), HandlerForSwitchBrkLess(R), HandlerForSwitchDftLEss(R), HandlerForMCSwitch151(R), HandlerForMCSwitch155(R), \
    HandlerForMCFunction161(R), HandlerForFunction162(R), HandlerForFunction164(R), HandlerForFunction166(R), HandlerForFunction168(R), \
    HandlerForFunction169(R), HandlerForPA171(R), HandlerForSU184(R), HandlerForType6465(R), HandlerForDCDF81(R), HandlerForDCDF82(R), \
    HandlerForInit91(R), HandlerForInit92(R), HandlerForInit93(R), HandlerForExpr123(R), HandlerForExpr124(R), HandlerForExpr125(R), \
    HandlerForExpr126(R), HandlerForExpr127(R), HandlerForExpr128(R), HandlerForExpr129(R), HandlerForExpr1210(R), HandlerForExpr1213(R), \
    HandlerForCSE131(R), HandlerForCSE132(R), HandlerForCSE1332(R), HandlerForCSE134(R), HandlerForCSE136(R), HandlerForCF144(R), \
    HandlerForCF145(R), HandlerForCF146(R), HandlerForCF147(R), HandlerForCF148(R), HandlerForSwitch154(R), HandlerForPTC111(R), \
    HandlerForCSE137(R), HandlerForDCDF810(R), HandlerForFunction165(R), HandlerForFunction1652(R), HandlerForPointer171(R), \
    HandlerForPointer1723(R), HandlerForPointer174(R), HandlerForPointer175(R), HandlerForTypes61(R), HandlerForSU181(R), \
    HandlerForMCPTCCSTYLE(R), HandlerForATC101(R), HandlerForIdent51(R), HandlerForDCDF87(R), HandlerForDCDF88(R), HandlerForLangX23(R), \
    HandlerForFunction167(R), HandlerForCF143(R), HandlerForExpr1212(R), HandlerForExpr1211(R), HandlerForAtc105(R), HandlerForCSE135(R), \
    HandlerForTypes612(R), HandlerForConst71(R), HandlerForIdent5X(R), HandlerForSFCPPARR01(R), HandlerForSFCPPARR02(R), \
    HandlerForSFCPPPNTR01(R), HandlerForSFCPPPNTR02(R) 
  {

/*@DEVI-disables all matchers*/
#if defined(_MUT0_EN_MATCHERS)

    Matcher.addMatcher(forStmt(unless(hasDescendant(compoundStmt()))).bind("mcfor"), &HandlerForCmpless);

    Matcher.addMatcher(whileStmt(unless(hasDescendant(compoundStmt()))).bind("mcwhile"), &HandlerWhileCmpless);

    Matcher.addMatcher(ifStmt(allOf(hasElse(unless(ifStmt())), hasElse(unless(compoundStmt())))).bind("mcelse"), &HandlerElseCmpless);

    Matcher.addMatcher(ifStmt(unless(hasDescendant(compoundStmt()))).bind("mcif"), &HandlerIfCmpless);

    Matcher.addMatcher(ifStmt(allOf(hasElse(ifStmt()), unless(hasAncestor(ifStmt())), unless(hasDescendant(ifStmt(hasElse(unless(ifStmt()))))))).bind("mcifelse"), &HandlerForIfElse);

    Matcher.addMatcher(switchStmt(hasDescendant(compoundStmt(hasDescendant(switchCase(unless(hasDescendant(breakStmt()))))))).bind("mcswitchbrk"), &HandlerForSwitchBrkLess);

    Matcher.addMatcher(switchStmt(unless(hasDescendant(defaultStmt()))).bind("mcswitchdft"), &HandlerForSwitchDftLEss);

    if (umRuleList.at("15.1"))
    {
      Matcher.addMatcher(switchStmt(forEachDescendant(caseStmt(hasAncestor(compoundStmt().bind("mccmp151"))).bind("mccase151"))), &HandlerForMCSwitch151);
    }

    if (umRuleList.at("15.5"))
    {
      Matcher.addMatcher(switchStmt(unless(hasDescendant(caseStmt()))).bind("mcswitch155"), &HandlerForMCSwitch155);
    }

    if (umRuleList.at("16.1"))
    {
      Matcher.addMatcher(functionDecl().bind("mcfunction161"), &HandlerForMCFunction161);
    }

    if (umRuleList.at("16.2"))
    {
      Matcher.addMatcher(functionDecl(forEachDescendant(callExpr().bind("mc162callexpr"))).bind("mc162funcdec"), &HandlerForFunction162);
    }

    if (umRuleList.at("16.4"))
    {
      Matcher.addMatcher(functionDecl().bind("mcfunc164"), &HandlerForFunction164);
    }

    if (umRuleList.at("16.6"))
    {
      Matcher.addMatcher(callExpr().bind("mcfunc166"), &HandlerForFunction166);
    }

    if (umRuleList.at("16.8"))
    {
      Matcher.addMatcher(functionDecl(forEachDescendant(returnStmt().bind("mcfunc168"))), &HandlerForFunction168);
    }

    if (umRuleList.at("16.9"))
    {
      Matcher.addMatcher(implicitCastExpr(unless(hasAncestor(callExpr()))).bind("mcfunc169"), &HandlerForFunction169);
    }

    if (umRuleList.at("17.1"))
    {
      Matcher.addMatcher(varDecl().bind("mcpa171"), &HandlerForPA171);
    }

    if (umRuleList.at("18.4"))
    {
      Matcher.addMatcher(recordDecl(isUnion()).bind("mcsu184"), &HandlerForSU184);
    }

    if (umRuleList.at("6.4") || umRuleList.at("6.5"))
    {
      Matcher.addMatcher(fieldDecl(isBitField()).bind("mctype6465"), &HandlerForType6465);
    }

    if (umRuleList.at("8.1"))
    {
      Matcher.addMatcher(functionDecl().bind("mcdcdf81"), &HandlerForDCDF81);
    }

    if (umRuleList.at("8.2"))
    {
      Matcher.addMatcher(varDecl().bind("mcdcdf82"), &HandlerForDCDF82);
    }

    if (umRuleList.at("9.1"))
    {
      Matcher.addMatcher(varDecl().bind("mcinit91"), &HandlerForInit91);
    }

    if (umRuleList.at("9.2"))
    {
      Matcher.addMatcher(initListExpr(hasAncestor(varDecl().bind("mcinit92daddy"))).bind("mcinit92"), &HandlerForInit92);
    }

    if (umRuleList.at("9.3"))
    {
      Matcher.addMatcher(enumConstantDecl(anyOf(allOf(hasDescendant(integerLiteral().bind("mcinit93kiddy")), \
                                          hasAncestor(enumDecl().bind("mcinit93daddy"))), hasAncestor(enumDecl().bind("mcinit93daddy")))).bind("mcinit93"), &HandlerForInit93);
    }

    if (umRuleList.at("12.3"))
    {
      Matcher.addMatcher(unaryExprOrTypeTraitExpr(hasDescendant(expr().bind("mcexpr123kiddy"))).bind("mcexpr123"), &HandlerForExpr123);
    }

    if (umRuleList.at("12.4"))
    {
      Matcher.addMatcher(binaryOperator(allOf(eachOf(hasOperatorName("||"), hasOperatorName("&&")), hasRHS(expr().bind("mcexpr124")))), &HandlerForExpr124);
    }

    if (umRuleList.at("12.5"))
    {
      Matcher.addMatcher(binaryOperator(allOf(eachOf(hasOperatorName("||"), hasOperatorName("&&")), \
                                              eachOf(hasRHS(allOf(expr().bind("lrhs"), unless(anyOf(implicitCastExpr() , declRefExpr(), callExpr(), floatLiteral(), integerLiteral(), stringLiteral()))))\
                                                  , hasLHS(allOf(expr().bind("lrhs"), unless(anyOf(implicitCastExpr(), declRefExpr(), callExpr(), floatLiteral(), integerLiteral(), stringLiteral())))))))\
                         , &HandlerForExpr125);
    }

    if (umRuleList.at("12.6"))
    {
      Matcher.addMatcher(binaryOperator(allOf(eachOf(hasOperatorName("||"), hasOperatorName("&&")), \
                                              eachOf(hasLHS(expr().bind("mcexpr126rl")), hasRHS(expr().bind("mcexpr126rl"))))), &HandlerForExpr126);
    }

    if (umRuleList.at("12.7"))
    {
      Matcher.addMatcher(binaryOperator(allOf(eachOf(hasOperatorName("<<"), hasOperatorName(">>"), hasOperatorName("~"), hasOperatorName("<<="), \
                                              hasOperatorName(">>="), hasOperatorName("&"), hasOperatorName("&="), hasOperatorName("^"), hasOperatorName("^=")\
                                              , hasOperatorName("|"), hasOperatorName("|=")), eachOf(hasLHS(expr().bind("mcexpr127rl")), hasRHS(expr().bind("mcexpr127rl"))))), &HandlerForExpr127);
    }

    if (umRuleList.at("12.8"))
    {
      Matcher.addMatcher(binaryOperator(allOf(eachOf(hasOperatorName(">>"), hasOperatorName(">>="), hasOperatorName("<<="), hasOperatorName("<<")), \
                                              hasLHS(expr().bind("mcexpr128lhs")) , hasRHS(expr().bind("mcexpr128rhs")))), &HandlerForExpr128);
    }

    if (umRuleList.at("12.9"))
    {
      Matcher.addMatcher(unaryOperator(allOf(hasOperatorName("-"), hasUnaryOperand(expr().bind("mcexpr129")))), &HandlerForExpr129);
    }

    if (umRuleList.at("12.10"))
    {
      Matcher.addMatcher(binaryOperator(allOf(hasOperatorName(","), hasLHS(expr().bind("mcexpr1210")))), &HandlerForExpr1210);
    }

    if (umRuleList.at("12.13"))
    {
      Matcher.addMatcher(unaryOperator(allOf(eachOf(hasOperatorName("++"), hasOperatorName("--"))\
              , anyOf(hasAncestor(binaryOperator()), hasDescendant(binaryOperator())))).bind("mcexpr1213"), &HandlerForExpr1213);
    }

    if (umRuleList.at("13.1"))
    {
      Matcher.addMatcher(binaryOperator(allOf(hasOperatorName("="), eachOf(hasLHS(expr().bind("cse131rlhs")), hasRHS(expr().bind("cse131rlhs"))))), &HandlerForCSE131);
    }

    if (umRuleList.at("13.2"))
    {
      Matcher.addMatcher(ifStmt(hasCondition(expr(unless(hasDescendant(binaryOperator(anyOf(hasOperatorName("<")\
                                             , hasOperatorName(">"), hasOperatorName("=="), hasOperatorName("<="), hasOperatorName(">=")))))).bind("mccse132"))), &HandlerForCSE132);
    }

    if (umRuleList.at("13.3"))
    {
      Matcher.addMatcher(binaryOperator(allOf(anyOf(hasOperatorName("<"), hasOperatorName(">"), hasOperatorName("<="), hasOperatorName(">="), hasOperatorName("==")), \
                                              eachOf(hasLHS(expr().bind("mccse1332rl")), hasRHS(expr().bind("mccse1332rl"))))).bind("mccse1332daddy"), &HandlerForCSE1332);
    }

    if (umRuleList.at("13.4"))
    {
      Matcher.addMatcher(forStmt().bind("mccse134"), &HandlerForCSE134);
    }

    if (umRuleList.at("13.6"))
    {
      Matcher.addMatcher(forStmt(forEachDescendant(stmt(eachOf(unaryOperator(allOf(anyOf(hasOperatorName("++"), hasOperatorName("--")), hasUnaryOperand(declRefExpr().bind("mccse136kiddo")))), \
                                 binaryOperator(allOf(hasOperatorName("="), hasLHS(declRefExpr().bind("mccse136kiddo")))))))).bind("mccse136daddy"), &HandlerForCSE136);
    }

    if (umRuleList.at("14.4"))
    {
      Matcher.addMatcher(gotoStmt().bind("mccf144"), &HandlerForCF144);
    }

    if (umRuleList.at("14.5"))
    {
      Matcher.addMatcher(continueStmt().bind("mccf145"), &HandlerForCF145);
    }

    if (umRuleList.at("14.6"))
    {
      Matcher.addMatcher(breakStmt(hasAncestor(stmt(anyOf(forStmt().bind("mccffofo"), doStmt().bind("mccfdodo"), whileStmt().bind("mccfwuwu"))))), &HandlerForCF146);
    }

    if (umRuleList.at("14.7"))
    {
      Matcher.addMatcher(returnStmt(hasAncestor(functionDecl().bind("mccf147"))), &HandlerForCF147);
    }

    if (umRuleList.at("14.8"))
    {
      Matcher.addMatcher(forStmt(unless(has(compoundStmt()))).bind("mccf148for"), &HandlerForCF148);
    }

    if (umRuleList.at("14.8"))
    {
      Matcher.addMatcher(whileStmt(unless(has(compoundStmt()))).bind("mccf148while"), &HandlerForCF148);
    }

    if (umRuleList.at("14.8"))
    {
      Matcher.addMatcher(doStmt(unless(has(compoundStmt()))).bind("mccf148do"), &HandlerForCF148);
    }

    if (umRuleList.at("14.8"))
    {
      Matcher.addMatcher(switchStmt(unless(has(compoundStmt()))).bind("mccf148switch"), &HandlerForCF148);
    }

    if (umRuleList.at("15.4"))
    {
      Matcher.addMatcher(switchStmt(hasCondition(expr().bind("mcswitch154"))).bind("mcswitch154daddy"), &HandlerForSwitch154);
    }

    if (umRuleList.at("11.1"))
    {
      Matcher.addMatcher(implicitCastExpr().bind("mcptc111"), &HandlerForPTC111);
    }

    if (umRuleList.at("13.7"))
    {
      Matcher.addMatcher(expr().bind("mccse137"), &HandlerForCSE137);
    }

    if (umRuleList.at("8.10"))
    {
      Matcher.addMatcher(callExpr(hasAncestor(functionDecl().bind("mcdcdf810daddy"))).bind("mcdcdf810"), &HandlerForDCDF810);
    }

    if (umRuleList.at("16.5"))
    {
      Matcher.addMatcher(functionDecl(allOf(returns(anything()), unless(returns(asString("void"))), hasBody(compoundStmt()) \
                                            , unless(hasDescendant(returnStmt())))).bind("mcfunction165"), &HandlerForFunction165);
    }

    if (umRuleList.at("16.5"))
    {
      Matcher.addMatcher(functionDecl(allOf(parameterCountIs(0), hasBody(compoundStmt()))).bind("mcfunction1652"), &HandlerForFunction1652);
    }

    if (umRuleList.at("17.1"))
    {
      Matcher.addMatcher(declRefExpr(allOf(to(varDecl().bind("loco")), unless(hasParent(castExpr(hasCastKind(clang::CK_ArrayToPointerDecay)))), \
                                           hasAncestor(stmt(eachOf(binaryOperator(hasOperatorName("+")).bind("bino"), \
                                           binaryOperator(hasOperatorName("-")).bind("bino"), unaryOperator(hasOperatorName("++")).bind("uno"), \
                                           unaryOperator(hasOperatorName("--")).bind("uno"), binaryOperator(hasOperatorName("/")).bind("bino"), \
                                           binaryOperator(hasOperatorName("*")).bind("bino"), binaryOperator(hasOperatorName("<")).bind("bino"), \
                                           binaryOperator(hasOperatorName("<=")).bind("bino"), binaryOperator(hasOperatorName(">")).bind("bino"), \
                                           binaryOperator(hasOperatorName(">=")).bind("bino")))))).bind("mcpointer171"), &HandlerForPointer171);
    }

    /*start of 17.3 matchers*/
    if (umRuleList.at("17.2") || umRuleList.at("17.3"))
    {
      Matcher.addMatcher(binaryOperator(allOf(hasOperatorName("<="), hasRHS(expr(anyOf(hasDescendant(declRefExpr(hasType(pointerType())).bind("mcpointer1723rhs")), \
                                              has(declRefExpr(hasType(pointerType())).bind("mcpointer1723rhs"))))), \
                                              hasLHS(expr(anyOf(hasDescendant(declRefExpr(hasType(pointerType())).bind("mcpointer1723lhs")), \
                                                  has(declRefExpr(hasType(pointerType())).bind("mcpointer1723lhs"))))))).bind("mcpointer1723daddy"), &HandlerForPointer1723);
    }

    if (umRuleList.at("17.2") || umRuleList.at("17.3"))
    {
      Matcher.addMatcher(binaryOperator(allOf(hasOperatorName("<"), hasRHS(expr(anyOf(hasDescendant(declRefExpr(hasType(pointerType())).bind("mcpointer1723rhs")), \
                                              has(declRefExpr(hasType(pointerType())).bind("mcpointer1723rhs"))))), \
                                              hasLHS(expr(anyOf(hasDescendant(declRefExpr(hasType(pointerType())).bind("mcpointer1723lhs")), \
                                                  has(declRefExpr(hasType(pointerType())).bind("mcpointer1723lhs"))))))).bind("mcpointer1723daddy"), &HandlerForPointer1723);
    }

    if (umRuleList.at("17.2") || umRuleList.at("17.3"))
    {
      Matcher.addMatcher(binaryOperator(allOf(hasOperatorName(">="), hasRHS(expr(anyOf(hasDescendant(declRefExpr(hasType(pointerType())).bind("mcpointer1723rhs")), \
                                              has(declRefExpr(hasType(pointerType())).bind("mcpointer1723rhs"))))), \
                                              hasLHS(expr(anyOf(hasDescendant(declRefExpr(hasType(pointerType())).bind("mcpointer1723lhs")), \
                                                  has(declRefExpr(hasType(pointerType())).bind("mcpointer1723lhs"))))))).bind("mcpointer1723daddy"), &HandlerForPointer1723);
    }

    if (umRuleList.at("17.2") || umRuleList.at("17.3"))
    {
      Matcher.addMatcher(binaryOperator(allOf(hasOperatorName(">"), hasRHS(expr(anyOf(hasDescendant(declRefExpr(hasType(pointerType())).bind("mcpointer1723rhs")), \
                                              has(declRefExpr(hasType(pointerType())).bind("mcpointer1723rhs"))))), \
                                              hasLHS(expr(anyOf(hasDescendant(declRefExpr(hasType(pointerType())).bind("mcpointer1723lhs")), \
                                                  has(declRefExpr(hasType(pointerType())).bind("mcpointer1723lhs"))))))).bind("mcpointer1723daddy"), &HandlerForPointer1723);
    }

    if (umRuleList.at("17.2") || umRuleList.at("17.3"))
    {
      Matcher.addMatcher(binaryOperator(allOf(hasOperatorName("-"), hasRHS(expr(anyOf(hasDescendant(declRefExpr(hasType(pointerType())).bind("mcpointer1723rhs")), \
                                              has(declRefExpr(hasType(pointerType())).bind("mcpointer1723rhs"))))), \
                                              hasLHS(expr(anyOf(hasDescendant(declRefExpr(hasType(pointerType())).bind("mcpointer1723lhs")), \
                                                  has(declRefExpr(hasType(pointerType())).bind("mcpointer1723lhs"))))))).bind("mcpointer1723daddy"), &HandlerForPointer1723);
    }

    if (umRuleList.at("17.2") || umRuleList.at("17.3"))
    {
      Matcher.addMatcher(binaryOperator(allOf(hasOperatorName("-="), hasRHS(expr(anyOf(hasDescendant(declRefExpr(hasType(pointerType())).bind("mcpointer1723rhs")), \
                                              has(declRefExpr(hasType(pointerType())).bind("mcpointer1723rhs"))))), \
                                              hasLHS(expr(anyOf(hasDescendant(declRefExpr(hasType(pointerType())).bind("mcpointer1723lhs")), \
                                                  has(declRefExpr(hasType(pointerType())).bind("mcpointer1723lhs"))))))).bind("mcpointer1723daddy"), &HandlerForPointer1723);
    }
    /*end of 17.3 matchers*/

    /*start of 17.4 matchers*/
    if (umRuleList.at("17.4"))
    {
      Matcher.addMatcher(castExpr(allOf(hasCastKind(CK_ArrayToPointerDecay), unless(hasParent(arraySubscriptExpr())))).bind("mcpointer174"), &HandlerForPointer174);
    }

    if (umRuleList.at("17.4"))
    {
      Matcher.addMatcher(declRefExpr(allOf(hasAncestor(expr(anyOf(binaryOperator(hasOperatorName("-=")), \
                                           unaryOperator(hasOperatorName("++")), unaryOperator(hasOperatorName("--")), \
                                           binaryOperator(hasOperatorName("+")), binaryOperator(hasOperatorName("+=")), \
                                           binaryOperator(hasOperatorName("-"))))), to(varDecl(hasType(pointerType()))))).bind("mcpointer1742"), &HandlerForPointer174);
    }
    /*end of 17.4 matchers*/

    if (umRuleList.at("17.5"))
    {
      Matcher.addMatcher(varDecl(hasType(pointerType())).bind("mcpointer175"), &HandlerForPointer175);
    }

    if (umRuleList.at("17.5"))
    {
      Matcher.addMatcher(fieldDecl().bind("mcpointer175field"), &HandlerForPointer175);
    }

    if (umRuleList.at("6.1"))
    {
      Matcher.addMatcher(declRefExpr(allOf(to(varDecl().bind("mctypes6origin")), \
                                           hasAncestor(binaryOperator(allOf(hasRHS(expr().bind("mctypes6rhs")), \
                                               hasOperatorName("="))).bind("mctypes6dous")), hasType(isAnyCharacter()))), &HandlerForTypes61);
    }

    if (umRuleList.at("18.1"))
    {
      Matcher.addMatcher(varDecl(hasType(incompleteArrayType())).bind("mcsu181arr"), &HandlerForSU181);
    }

    if (umRuleList.at("18.1"))
    {
      Matcher.addMatcher(recordDecl(isStruct()).bind("mcsu181struct"), &HandlerForSU184);
    }

    Matcher.addMatcher(cStyleCastExpr().bind("mcptc11cstyle"), &HandlerForMCPTCCSTYLE);

    if (umRuleList.at("10.1"))
    {
      Matcher.addMatcher(implicitCastExpr(has(expr(anyOf(binaryOperator().bind("atcdous"), unaryOperator().bind("atcuno"), \
                                              parenExpr().bind("atcparens"), implicitCastExpr().bind("atckidice"), \
                                              cStyleCastExpr().bind("atccstyle"))))).bind("atcdaddy"), &HandlerForATC101);
    }

    if (umRuleList.at("5.1"))
    {
      Matcher.addMatcher(namedDecl().bind("ident5nameddecl"), &HandlerForIdent51);
    }

    if (umRuleList.at("8.7"))
    {
      Matcher.addMatcher(declRefExpr(allOf(hasAncestor(functionDecl().bind("mcdcdf87daddy")), \
                                           to(varDecl(unless(hasAncestor(functionDecl()))).bind("mcdcdf87origin")))).bind("mcdcdfobj"), &HandlerForDCDF87);
    }

/*@DEVI-these two matcheres are breaking our 3.9 backwards compatibility.*/
#if 1
    if (umRuleList.at("8.8"))
    {
      Matcher.addMatcher(functionDecl(hasExternalFormalLinkage()).bind("mcdcdf88function"), &HandlerForDCDF88);
    }

    if (umRuleList.at("8.8"))
    {
      Matcher.addMatcher(varDecl(hasExternalFormalLinkage()).bind("mcdcdf88var"), &HandlerForDCDF88);
    }
#endif

    if (umRuleList.at("2.3"))
    {
      Matcher.addMatcher(expr().bind("mclangx23"), &HandlerForLangX23);
    }

    if (umRuleList.at("16.7"))
    {
      Matcher.addMatcher(parmVarDecl(unless(allOf(hasAncestor(functionDecl(hasDescendant(binaryOperator(allOf(hasOperatorName("="), \
                                            hasLHS(hasDescendant(declRefExpr(allOf(hasAncestor(unaryOperator(hasOperatorName("*"))), \
                                                to(parmVarDecl(hasType(pointerType())).bind("zulu"))))))))))), equalsBoundNode("zulu")))).bind("mcfunction167"), &HandlerForFunction167);
    }

    if (umRuleList.at("14.3"))
    {
      Matcher.addMatcher(nullStmt().bind("mccf143nullstmt"), &HandlerForCF143);
    }

    if (umRuleList.at("12.12"))
    {
      Matcher.addMatcher(recordDecl(allOf(has(fieldDecl(hasType(realFloatingPointType()))), isUnion())).bind("mcexpr1212"), &HandlerForExpr1212);
    }

    if (umRuleList.at("12.11"))
    {
      Matcher.addMatcher(expr(hasDescendant(expr(anyOf(unaryOperator(hasOperatorName("--"), hasOperatorName("++")).bind("mcexpr1211uno"), \
                                            binaryOperator(anyOf(hasOperatorName("*"), hasOperatorName("/"), \
                                                hasOperatorName("-"), hasOperatorName("+"))).bind("mcexpr1211dous"))))).bind("mcexpr1211"), &HandlerForExpr1211);
    }

    if (umRuleList.at("10.5"))
    {
      Matcher.addMatcher(binaryOperator(allOf(hasLHS(expr(hasType(isInteger())).bind("mcatc105lhs")), hasOperatorName("<<"))).bind("mcatc105"), &HandlerForAtc105);
    }

    if (umRuleList.at("10.5"))
    {
      Matcher.addMatcher(unaryOperator(allOf(hasOperatorName("~") , hasUnaryOperand(expr(hasType(isInteger())).bind("mcatc105lhs")))).bind("mcatc105uno"), &HandlerForAtc105);
    }

    if (umRuleList.at("13.5"))
    {
      Matcher.addMatcher(forStmt().bind("mccse135"), &HandlerForCSE135);
    }

    if (umRuleList.at("6.1") || umRuleList.at("6.2"))
    {
      Matcher.addMatcher(binaryOperator(allOf(hasRHS(expr(has(expr(anyOf(integerLiteral().bind("mc612intlit"), \
                                              characterLiteral().bind("mc612charlit")))))), hasLHS(expr(hasType(isAnyCharacter())).bind("mc612exp")), \
                                              hasOperatorName("="))), &HandlerForTypes612);
    }

    /*@DEVI-start of 7.1 matchers.*/
    if (umRuleList.at("7.1"))
    {
      Matcher.addMatcher(stringLiteral().bind("mcconst71string"), &HandlerForConst71);
    }

    if (umRuleList.at("7.1"))
    {
      Matcher.addMatcher(characterLiteral().bind("mcconst71char"), &HandlerForConst71);
    }

    if (umRuleList.at("7.1"))
    {
      Matcher.addMatcher(integerLiteral().bind("mcconst71int"), &HandlerForConst71);
    }
    /*end of 7.1*/

    /*@DEVI-matchers for 5.x*/
    /*@DEVI-typedefs always have file scope.*/
    if (umRuleList.at("5.1") || umRuleList.at("5.2"), umRuleList.at("5.3") || umRuleList.at("5.4") || umRuleList.at("5.5") || umRuleList.at("5.6") || umRuleList.at("5.7"))
    {
      Matcher.addMatcher(typedefDecl().bind("ident5typedef"), &HandlerForIdent5X);
    }

#if 0
    Matcher.addMatcher(typedefDecl(unless(hasAncestor(functionDecl()))).bind("ident5typedef"), &HandlerForIdent5X);

    Matcher.addMatcher(typedefDecl(hasAncestor(functionDecl().bind("id5funcscope"))).bind("ident5typedef"), &HandlerForIdent5X);
#endif


    if (umRuleList.at("5.1") || umRuleList.at("5.2"), umRuleList.at("5.3") || umRuleList.at("5.4") || umRuleList.at("5.5") || umRuleList.at("5.6") || umRuleList.at("5.7"))
    {
      Matcher.addMatcher(recordDecl(unless(hasAncestor(functionDecl()))).bind("ident5record"), &HandlerForIdent5X);

      Matcher.addMatcher(recordDecl(hasAncestor(functionDecl().bind("id5funcscope"))).bind("ident5record"), &HandlerForIdent5X);

      Matcher.addMatcher(fieldDecl(unless(hasAncestor(functionDecl()))).bind("ident5field"), &HandlerForIdent5X);

      Matcher.addMatcher(fieldDecl(hasAncestor(functionDecl().bind("id5funcscope"))).bind("ident5field"), &HandlerForIdent5X);

      Matcher.addMatcher(parmVarDecl(hasAncestor(functionDecl().bind("id5funcscope"))).bind("ident5parmvar"), &HandlerForIdent5X);

      Matcher.addMatcher(functionDecl().bind("ident5func"), &HandlerForIdent5X);

      Matcher.addMatcher(varDecl(unless(hasAncestor(functionDecl()))).bind("ident5var"), &HandlerForIdent5X);

      Matcher.addMatcher(varDecl(hasAncestor(functionDecl().bind("id5funcscope"))).bind("ident5var"), &HandlerForIdent5X);

      Matcher.addMatcher(enumDecl(unless(hasAncestor(functionDecl()))).bind("ident5enum") , &HandlerForIdent5X);

      Matcher.addMatcher(enumDecl(hasAncestor(functionDecl().bind("id5funcscope"))).bind("ident5enum"), &HandlerForIdent5X);

      /*@DEVI-labels always have function scope.*/
      Matcher.addMatcher(labelDecl(hasAncestor(functionDecl().bind("id5funcscope"))).bind("ident5label"), &HandlerForIdent5X);

      Matcher.addMatcher(enumConstantDecl(unless(hasAncestor(functionDecl()))).bind("ident5enumconst"), &HandlerForIdent5X);

      Matcher.addMatcher(enumConstantDecl(hasAncestor(functionDecl().bind("id5funcscope"))).bind("ident5enumconst"), &HandlerForIdent5X);
    }
    /*end of matchers for 5.x*/

    /*start of SaferCPP matchers*/
    Matcher.addMatcher(varDecl(hasType(arrayType())).bind("sfcpparrdecl"), &HandlerForSFCPPARR01);

    Matcher.addMatcher(fieldDecl(hasType(arrayType())).bind("sfcpparrfield"), &HandlerForSFCPPARR01);

    Matcher.addMatcher(implicitCastExpr(hasCastKind(CK_ArrayToPointerDecay)).bind("sfcpparrcastexpr"), &HandlerForSFCPPARR01);

    Matcher.addMatcher(cStyleCastExpr(hasCastKind(CK_ArrayToPointerDecay)).bind("sfcpparrcastexpr"), &HandlerForSFCPPARR01);

    Matcher.addMatcher(declRefExpr(hasAncestor(binaryOperator(allOf(hasLHS(declRefExpr().bind("sfcpparrdeep")), hasRHS(hasDescendant(implicitCastExpr(hasCastKind(CK_ArrayToPointerDecay))))\
                , hasOperatorName("="))))), &HandlerForSFCPPARR02);

    Matcher.addMatcher(varDecl(hasType(pointerType())).bind("sfcpppntr01"), &HandlerForSFCPPPNTR01);

    Matcher.addMatcher(declRefExpr(hasType(pointerType())).bind("sfcpppntr02"), &HandlerForSFCPPPNTR02);
    /*end of SaferCPP matchers*/

#endif
  }

  void HandleTranslationUnit(ASTContext &Context) override {
    Matcher.matchAST(Context);
  }

private:
  MCForCmpless HandlerForCmpless;
  MCWhileCmpless HandlerWhileCmpless;
  MCElseCmpless HandlerElseCmpless;
  MCIfCmpless HandlerIfCmpless;
  IfElseMissingFixer HandlerForIfElse;
  MCSwitchBrkless HandlerForSwitchBrkLess;
  MCSwitchDftLess HandlerForSwitchDftLEss;
  MCSwitch151 HandlerForMCSwitch151;
  MCSwitch155 HandlerForMCSwitch155;
  MCFunction161 HandlerForMCFunction161;
  MCFunction162 HandlerForFunction162;
  MCFunction164 HandlerForFunction164;
  MCFunction166 HandlerForFunction166;
  MCFunction168 HandlerForFunction168;
  MCFunction169 HandlerForFunction169;
  MCPA171 HandlerForPA171;
  MCSU184 HandlerForSU184;
  MCTypes6465 HandlerForType6465;
  MCDCDF81 HandlerForDCDF81;
  MCDCDF82 HandlerForDCDF82;
  MCInit91 HandlerForInit91;
  MCInit92 HandlerForInit92;
  MCInit93 HandlerForInit93;
  MCExpr123 HandlerForExpr123;
  MCExpr124 HandlerForExpr124;
  MCExpr125 HandlerForExpr125;
  MCExpr126 HandlerForExpr126;
  MCExpr127 HandlerForExpr127;
  MCExpr128 HandlerForExpr128;
  MCExpr129 HandlerForExpr129;
  MCExpr1210 HandlerForExpr1210;
  MCExpr1213 HandlerForExpr1213;
  MCCSE131 HandlerForCSE131;
  MCCSE132 HandlerForCSE132;
  MCCSE1332 HandlerForCSE1332;
  MCCSE134 HandlerForCSE134;
  MCCSE136 HandlerForCSE136;
  MCCF144 HandlerForCF144;
  MCCF145 HandlerForCF145;
  MCCF146 HandlerForCF146;
  MCCF147 HandlerForCF147;
  MCCF148 HandlerForCF148;
  MCSwitch154 HandlerForSwitch154;
  MCPTC111 HandlerForPTC111;
  MCCSE137 HandlerForCSE137;
  MCDCDF810 HandlerForDCDF810;
  MCFunction165 HandlerForFunction165;
  MCFunction1652 HandlerForFunction1652;
  MCPointer171 HandlerForPointer171;
  MCPointer1723 HandlerForPointer1723;
  MCPointer174 HandlerForPointer174;
  MCPointer175 HandlerForPointer175;
  MCTypes61 HandlerForTypes61;
  MCSU181 HandlerForSU181;
  MCPTC11CSTYLE HandlerForMCPTCCSTYLE;
  MCATC101 HandlerForATC101;
  MCIdent51 HandlerForIdent51;
  MCDCDF87 HandlerForDCDF87;
  MCDCDF88 HandlerForDCDF88;
  MCLangX23 HandlerForLangX23;
  MCFunction167 HandlerForFunction167;
  MCCF143 HandlerForCF143;
  MCExpr1212 HandlerForExpr1212;
  MCExpr1211 HandlerForExpr1211;
  MCATC105 HandlerForAtc105;
  MCCSE135 HandlerForCSE135;
  MCTypes612 HandlerForTypes612;
  MCConst71 HandlerForConst71;
  MCIdent5x HandlerForIdent5X;
  SFCPPARR01 HandlerForSFCPPARR01;
  SFCPPARR02 HandlerForSFCPPARR02;
  SFCPPPNTR01 HandlerForSFCPPPNTR01;
  SFCPPPNTR02 HandlerForSFCPPPNTR02;
  MatchFinder Matcher;
};
/**********************************************************************************************************************/
class Mutator0DiagnosticConsumer : public clang::DiagnosticConsumer
{
public:

  virtual void HandleDiagnostic(DiagnosticsEngine::Level DiagLevel, const Diagnostic &Info) override
  {
    DiagnosticConsumer::HandleDiagnostic(DiagLevel, Info);

    SourceLocation SL = Info.getLocation(); 
    CheckSLValidity(SL);

    SourceManager &SM = Info.getSourceManager();

    if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, SM, SL))
    {
      return void();
    }

    if (!Devi::IsTheMatchInMainFile(MainFileOnly, SM, SL))
    {
      return void();
    }

    SL = SM.getSpellingLoc(SL);

    unsigned SpellingLine = SM.getSpellingLineNumber(SL);
    unsigned SpellingColumn = SM.getSpellingColumnNumber(SL);
    std::string FileName = SM.getFilename(SL).str();

    SmallString<100> DiagBuffer;

    Info.FormatDiagnostic(DiagBuffer);

    std::cout << "ClangDiag:" << DiagBuffer.str().str() << ":" << SL.printToString(SM) << ":" << Info.getID() << ":" << "\n";

    XMLDocOut.XMLAddNode(SpellingLine, SpellingColumn, FileName, "ClangDiag", DiagBuffer.str().str());
    JSONDocOUT.JSONAddElement(SpellingLine, SpellingColumn, FileName, "ClangDiag", DiagBuffer.str().str());

    if (Info.getID() == 872U)
    {
      std::cout << "2.2:" << "Illegal comment format(/*...*/) used:" << SL.printToString(SM) << ":" << "\n";

      XMLDocOut.XMLAddNode(SpellingLine, SpellingColumn, FileName, "2.2", "Illegal comment format(/*...*/) used:");
      JSONDocOUT.JSONAddElement(SpellingLine, SpellingColumn, FileName, "2.2", "Illegal comment format(/*...*/) used:");
    }

    if (Info.getID() == 974U)
    {
      std::cout << "2.3:" << "Use of the character sequence /* inside a comment is illegal:" << SL.printToString(SM) << ":" << "\n";

      XMLDocOut.XMLAddNode(SpellingLine, SpellingColumn, FileName, "2.3", "Use of the character sequence /* inside a comment is illegal:");
      JSONDocOUT.JSONAddElement(SpellingLine, SpellingColumn, FileName, "2.3", "Use of the character sequence /* inside a comment is illegal:");
    }

    if (Info.getID() == 938U)
    {
      std::cout << "4.2:" << "Use of trigraphs is illegal:" << SL.printToString(SM) << ":" << "\n";

      XMLDocOut.XMLAddNode(SpellingLine, SpellingColumn, FileName, "4.2", "Use of trigraphs is illegal:");
      JSONDocOUT.JSONAddElement(SpellingLine, SpellingColumn, FileName, "4.2", "Use of trigraphs is illegal:");
    }

    if (Info.getID() == 4578U)
    {
      std::cout << "9.2:" << "Brace initialization has either not been correctly used or not used at all:" << SL.printToString(SM) << ":" << "\n";

      XMLDocOut.XMLAddNode(SpellingLine, SpellingColumn, FileName, "9.2", "Brace initialization has either not been correctly used or not used at all:");
      JSONDocOUT.JSONAddElement(SpellingLine, SpellingColumn, FileName, "9.2", "Brace initialization has either not been correctly used or not used at all:");
    }

    if (Info.getID() == 4872U)
    {
      std::cout << "14.2:" << "Expression result is unused:" << SL.printToString(SM) << ":" << "\n";

      XMLDocOut.XMLAddNode(SpellingLine, SpellingColumn, FileName, "14.2", "Expression result is unused:");
      JSONDocOUT.JSONAddElement(SpellingLine, SpellingColumn, FileName, "14.2", "Expression result is unused:");
    }

    if (Info.getID() == 966U)
    {
      std::cout << "19.14:" << "\"defined\" has undefined behaviour:" << SL.printToString(SM) << ":" << "\n";

      XMLDocOut.XMLAddNode(SpellingLine, SpellingColumn, FileName, "19.14", "\"defined\" has undefined behaviour:");
      JSONDocOUT.JSONAddElement(SpellingLine, SpellingColumn, FileName, "19.14", "\"defined\" has undefined behaviour:");
    }

    if (Info.getID() == 895U)
    {
      std::cout << "20.1:" << "Redefining built-in macro:" << SL.printToString(SM) << ":" << "\n";

      XMLDocOut.XMLAddNode(SpellingLine, SpellingColumn, FileName, "20.1", "Redefining built-in macro:");
      JSONDocOUT.JSONAddElement(SpellingLine, SpellingColumn, FileName, "20.1", "Redefining built-in macro:");
    }

  }

private:

};
/**********************************************************************************************************************/
class MyFrontendAction : public ASTFrontendAction
{
public:
  MyFrontendAction() {}

  void EndSourceFileAction() override
  {

  }

  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, StringRef file) override
  {
#if 1
    CI.getPreprocessor().addPPCallbacks(llvm::make_unique<PPInclusion>(&CI.getSourceManager()));
#endif

    DiagnosticsEngine &DiagEngine = CI.getPreprocessor().getDiagnostics();

#if 1
    Mutator0DiagnosticConsumer* M0DiagConsumer = new Mutator0DiagnosticConsumer;

    DiagEngine.setClient(M0DiagConsumer, true);
#endif

    TheRewriter.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
    return llvm::make_unique<MyASTConsumer>(TheRewriter);
  }

private:
  Rewriter TheRewriter;
};
/**********************************************************************************************************************/
/*Main*/
int main(int argc, const char **argv)
{
  CommonOptionsParser op(argc, argv, MutatorLVL0Cat);

  CompilationDatabase &CDB [[maybe_unused]] = op.getCompilations();
  std::vector<CompileCommand> ComCom = CDB.getAllCompileCommands();
  std::vector<std::vector<std::string>> ExecCL;
  
#if defined(_MUT0_TEST)
  for (auto &iter : ComCom)
  {
    ExecCL.push_back(iter.CommandLine);
  }

  for (auto &iter : ExecCL)
  {
    for (auto &yaiter : iter)
    {
      std::cout << "comcom: " << yaiter << "\n";
    }

    std::cout << "\n";
  }
#endif

  const std::vector<std::string> &SourcePathList = op.getSourcePathList();

  ClangTool Tool(op.getCompilations(), op.getSourcePathList());

  StringOptionsParser SOPProto;

  SOPProto.MC2Parser();

#if defined(_MUT0_TEST)
  SOPProto.Dump(true);
#endif

#if defined(_MUT0_TEST)
  if (SOPProto.MC2Parser())
  {
    typedef std::multimap<std::string, std::string>::iterator Iter;
    for (Iter iter = MC1EquivalencyMap.begin(), iterE = MC1EquivalencyMap.end(); iter != iterE; ++iter)
    {
      std::cout << "Key: " << iter->first << "  " << "Value: " << iter->second << "\n";
    }
  }
#endif
  
  XMLDocOut.XMLCreateReport();

  JSONDocOUT.JSONCreateReport();

  IsThereJunkPreInclusion ITJPIInstance;

  ITJPIInstance.Check(SourcePathList);
  
  int RunResult = 0;

  try 
  {
    RunResult = Tool.run(newFrontendActionFactory<MyFrontendAction>().get());
  }
  catch (MutExHeaderNotFound &E1)
  {
    std::cerr << E1.what() << "\n";
  }
  catch (std::domain_error &E2)
  {
    std::cerr << E2.what() << "\n";
  }
  catch(...)
  {
    std::cerr << "Unexpected exception!\n";
  }

  CheckForNullStatements CheckForNull;

  CheckForNull.Check();

  onEndOfAllTUs::run();

  XMLDocOut.SaveReport();

  JSONDocOUT.CloseReport();
  
  ME.DumpAll();
  ME.XMLReportAncestry();

  return RunResult;
} //end of main
/*last line intentionally left blank.*/

