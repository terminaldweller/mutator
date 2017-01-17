
/*first line intentionally left blank.*/
/*code structure inspired by Eli Bendersky's tutorial on Rewriters.*/
/**********************************************************************************************************************/
/*included modules*/
/*project headers*/
#include "mutator_aux.h"
/*standard headers*/
#include <cassert>
#include <fstream>
#include <iostream>
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
#include "clang/Basic/OperatorKinds.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Basic/IdentifierTable.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Lex/Lexer.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Lex/PPCallbacks.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Core/QualTypeNames.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Rewrite/Core/Rewriter.h"
/*LLVM headers*/
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
/*global vars*/
Devi::XMLReport XMLDocOut;
Devi::JSONReport JSONDocOUT;

std::vector<SourceLocation> MacroDefSourceLocation;
std::vector<SourceLocation> MacroUndefSourceLocation;
std::vector<std::string> MacroNameString;
std::vector<std::string> IncludeFileArr;

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

/*mutator-lvl0 executable options*/
enum MisraC
{
  MisraC2004, MisraC2012, C2, C3
};

static llvm::cl::OptionCategory MutatorLVL0Cat("mutator-lvl0 options category");
/*@DEVI-the option has been added since gcc does it.its as simple as that.*/
cl::opt<bool> CheckSystemHeader("SysHeader", cl::desc("mutator-lvl0 will run through System Headers"), cl::init(false), cl::cat(MutatorLVL0Cat), cl::ZeroOrMore);
cl::opt<bool> MainFileOnly("MainOnly", cl::desc("mutator-lvl0 will only report the results that reside in the main file"), cl::init(false), cl::cat(MutatorLVL0Cat), cl::ZeroOrMore);
cl::opt<MisraC> MisraCVersion(cl::desc("choose the MisraC version to check against"), \
                              cl::values(clEnumVal(MisraC2004, "Misra-C:2004"), clEnumVal(MisraC2012, "Misra-C:2012"), \
                                  clEnumVal(C2, "Misra-C:2004"), clEnumVal(C3, "Misra-C:2012")));
/**********************************************************************************************************************/
class [[deprecated("replaced by a more efficient class"), maybe_unused]] MCForCmpless : public MatchFinder::MatchCallback {
public:
  MCForCmpless (Rewriter &Rewrite) : Rewrite (Rewrite) {}

  virtual void run(const MatchFinder::MatchResult &MR)
  {
    if (MR.Nodes.getNodeAs<clang::ForStmt>("mcfor") != nullptr)
    {
      const ForStmt *FS = MR.Nodes.getNodeAs<clang::ForStmt>("mcfor");

      SourceLocation SL = FS->getLocStart();
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

#if 0
      std::cout << "14.8 : " << "\"For\" statement has no braces {}: " << std::endl;
      std::cout << SL.printToString(*MR.SourceManager) << "\n" << std::endl;
#endif
    }
    else
    {
      std::cout << "matcher -mcfor- returned nullptr." << std::endl;
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
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

#if 0
      std::cout << "14.8 : " << "\"While\" statement has no braces {}: " << std::endl;
      std::cout << SL.printToString(*MR.SourceManager) << "\n" << std::endl;
#endif
    }
    else
    {
      std::cout << "matcher -mcwhile- returned nullptr." << std::endl;
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

      SourceLocation SL = IS->getLocStart();
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

      if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
      {
        return void();
      }

      if (Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
      {
        std::cout << "14.9:" << "\"Else\" statement has no braces {}:";
        std::cout << SL.printToString(*MR.SourceManager) << ":" << std::endl;

        XMLDocOut.XMLAddNode(MR.Context, SL, "14.9", "\"Else\" statement has no braces {}: ");
        JSONDocOUT.JSONAddElement(MR.Context, SL, "14.9", "\"Else\" statement has no braces {}: ");
      }
    }
    else
    {
      std::cout << "matcher -mcelse- returned nullptr." << std::endl;
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

      SourceLocation SL = IS->getLocStart();
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

      if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
      {
        return void();
      }

      if (Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
      {
        std::cout << "14.9:" << "\"If\" statement has no braces {}:";
        std::cout << SL.printToString(*MR.SourceManager) << ":" << std::endl;

        XMLDocOut.XMLAddNode(MR.Context, SL, "14.9", "\"If\" statement has no braces {}: ");
        JSONDocOUT.JSONAddElement(MR.Context, SL, "14.9", "\"If\" statement has no braces {}: ");
      }
    }
    else
    {
      std::cout << "matcher -mcif- returned nullptr." << std::endl;
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
      IFESL = Devi::SourceLocationHasMacro(IFESL, Rewrite, "start");

      if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, IFESL))
      {
        return void();
      }

      if (Devi::IsTheMatchInMainFile(MainFileOnly, MR, IFESL))
      {
        std::cout << "14.10:" << "\"If-Else If\" statement has no ending Else:";
        std::cout << IFESL.printToString(*MR.SourceManager) << ":" << std::endl;

        XMLDocOut.XMLAddNode(MR.Context, IFESL, "14.10", "\"If-Else If\" statement has no ending Else: ");
        JSONDocOUT.JSONAddElement(MR.Context, IFESL, "14.10", "\"If-Else If\" statement has no ending Else: ");
      }
    }
    else
    {
      std::cout << "matcher -mcifelse- returned nullptr." << std::endl;
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
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

      if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
      {
        return void();
      }

      if (Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
      {
        std::cout << "15.2:" << "\"SwitchStmt\" has a caseStmt that's missing a breakStmt:";
        std::cout << SL.printToString(*MR.SourceManager) << ":" << std::endl;

        XMLDocOut.XMLAddNode(MR.Context, SL, "15.2", "\"SwitchStmt\" has a caseStmt that's missing a breakStmt: ");
        JSONDocOUT.JSONAddElement(MR.Context, SL, "15.2", "\"SwitchStmt\" has a caseStmt that's missing a breakStmt: ");
      }
    }
    else
    {
      std::cout << "matcher -mcswitchbrk- returned nullptr." << std::endl;
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
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

      if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
      {
        return void();
      }

      if (Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
      {
        std::cout << "15.3:" << "\"SwitchStmt\" does not have a defaultStmt:";
        std::cout << SL.printToString(*MR.SourceManager) << ":" << std::endl;

        XMLDocOut.XMLAddNode(MR.Context, SL, "15.3", "\"SwitchStmt\" does not have a defaultStmt: ");
        JSONDocOUT.JSONAddElement(MR.Context, SL, "15.3", "\"SwitchStmt\" does not have a defaultStmt: ");
      }
    }
    else
    {
      std::cout << "matcher -mcswitchdft- returned nullptr." << std::endl;
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
          std::cout << SL.printToString(*MR.SourceManager) << ":" << std::endl;

          XMLDocOut.XMLAddNode(MR.Context, SL, "15.1", "\"CaseStmt\" has a CompoundStmt ancestor that is not the child of the SwitchStmt: ");
          JSONDocOUT.JSONAddElement(MR.Context, SL, "15.1", "\"CaseStmt\" has a CompoundStmt ancestor that is not the child of the SwitchStmt: ");
        }
      }
    }
    else
    {
      std::cout << "matcher -mccmp151- or -mccase151- returned nullptr." << std::endl;
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
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

      if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
      {
        return void();
      }

      if (Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
      {
        std::cout << "15.5:" << "\"SwitchStmt\" does not have a CaseStmt:";
        std::cout << SL.printToString(*MR.SourceManager) << ":" << std::endl;

        XMLDocOut.XMLAddNode(MR.Context, SL, "15.5", "\"SwitchStmt\" does not have a CaseStmt: ");
        JSONDocOUT.JSONAddElement(MR.Context, SL, "15.5", "\"SwitchStmt\" does not have a CaseStmt: ");
      }
    }
    else
    {
      std::cout << "matcher -mcswitch155- returned nullptr." << std::endl;
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
        SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

        if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
        {
          return void();
        }

        if (Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
        {
          std::cout << "16.1:" << "\"FunctionDecl\" is variadic:";
          std::cout << SL.printToString(*MR.SourceManager) << ":" << std::endl;

          XMLDocOut.XMLAddNode(MR.Context, SL, "16.1", "\"FunctionDecl\" is variadic: ");
          JSONDocOUT.JSONAddElement(MR.Context, SL, "16.1", "\"FunctionDecl\" is variadic: ");
        }
      }
    }
    else
    {
      std::cout << "matcher -mcfunction161- returned nullptr." << std::endl;
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
          std::cout << SL.printToString(*MR.SourceManager) << ":" << std::endl;

          XMLDocOut.XMLAddNode(MR.Context, SL, "16.2", "\"FunctionDecl\" is recursive: ");
          JSONDocOUT.JSONAddElement(MR.Context, SL, "16.2", "\"FunctionDecl\" is recursive: ");
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
      std::cout << "matcher -mc162funcdec- and/or -mc162callexpr- returned nullptr." << std::endl;
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
          std::cout << "numparam of functiondefinition and functionDecl dont match! : " << SL.printToString(*MR.SourceManager) << "\n" << std::endl;
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
                std::cout << SL.printToString(*MR.SourceManager) << ":" << std::endl;

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
      std::cout << "matcher -mcfunc164- returned nullptr." << std::endl;
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
            std::cout << SL.printToString(*MR.SourceManager) << ":" << std::endl;

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
            std::cout << SL.printToString(*MR.SourceManager) << ":" << std::endl;

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
            std::cout << SL.printToString(*MR.SourceManager) << ":" << std::endl;

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
            std::cout << SL.printToString(*MR.SourceManager) << ":" << std::endl;

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
            std::cout << SL.printToString(*MR.SourceManager) << ":" << std::endl;

            XMLDocOut.XMLAddNode(MR.Context, SL, "16.6", "CallExpr number of arguments does not equal the number of parameters in the declaration: ");
            JSONDocOUT.JSONAddElement(MR.Context, SL, "16.6", "CallExpr number of arguments does not equal the number of parameters in the declaration: ");
          }
        }
      }
    }
    else
    {
      std::cout << "matcher -mcfunc166- returned nullptr." << std::endl;
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
      std::cout << RetType << "\n" << std::endl;
#endif
    }
    else
    {
      std::cout << "matcher -mcfunc168- returned nullptr." << std::endl;
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
        std::cout << SL.printToString(*MR.SourceManager) << ":" << std::endl;

        XMLDocOut.XMLAddNode(MR.Context, SL, "16.9", "FunctionToPointerDecay: ");
        JSONDocOUT.JSONAddElement(MR.Context, SL, "16.9", "FunctionToPointerDecay: ");
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
      std::cout << QT.getAsString() << "\n" << std::endl;
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
          std::cout << SL.printToString(*MR.SourceManager) << ":" << std::endl;

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
            std::cout << StructInfoProto[x].StructSL << ":" << std::endl;

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
            std::cout << UnionInfoProto[x].UnionSL << ":" << std::endl;

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
            std::cout << SL.printToString(*MR.SourceManager) << ":" << std::endl;

            XMLDocOut.XMLAddNode(MR.Context, SL, "6.4", "BitField has a type other than int or unsigned int: ");
            JSONDocOUT.JSONAddElement(MR.Context, SL, "6.4", "BitField has a type other than int or unsigned int: ");
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
              std::cout << SL.printToString(*MR.SourceManager) << ":" << std::endl;

              XMLDocOut.XMLAddNode(MR.Context, SL, "6.5", "BitField of type signed integer has a length of less than 2 in bits : ");
              JSONDocOUT.JSONAddElement(MR.Context, SL, "6.5", "BitField of type signed integer has a length of less than 2 in bits : ");
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
            std::cout << SL.printToString(*MR.SourceManager) << ":" << std::endl;

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
                std::cout << MacroDefSourceLocation[x].printToString(*MR.SourceManager) << " " << MacroNameString[x] << "\n" << std::endl;
#endif
                std::cout << MacroDefSourceLocation[x].printToString(*MR.SourceManager) << ":" << std::endl;

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
                std::cout << MacroUndefSourceLocation[x].printToString(*MR.SourceManager) << ":" << std::endl;

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
            std::cout << FuncInfoProto[x].StrcSL << ":" << std::endl;

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
      std::cout << QualifiedName << "\n" << std::endl;
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
        std::cout << "XXXXXXXXXXXXXXXXXXXXXXXX" << " " << IncludeFileArr.size() << std::endl;
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
            std::cout << SL.printToString(*MR.SourceManager) << ":" << std::endl;

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
                std::cout << SL.printToString(*MR.SourceManager) << ":" << std::endl;

                XMLDocOut.XMLAddNode(MR.Context, SL, "8.12", "External array type is incomplete and has no initialization : ");
                JSONDocOUT.JSONAddElement(MR.Context, SL, "8.12", "External array type is incomplete and has no initialization : ");
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
          IESL = Devi::SourceLocationHasMacro(IESL, Rewrite, "start");
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
                std::cout << SL.printToString(*MR.SourceManager) << ":" << std::endl;

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
              std::cout << SL.printToString(*MR.SourceManager) << ":" << std::endl;

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
      std::cout << NumInits << "\n" << std::endl;
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
                std::cout << SL.printToString(*MR.SourceManager) << ":" << std::endl;

                XMLDocOut.XMLAddNode(MR.Context, SL, "9.3", "first enumeration has integerliteral initialization but not all enumerations do : ");
                JSONDocOUT.JSONAddElement(MR.Context, SL, "9.3", "first enumeration has integerliteral initialization but not all enumerations do : ");
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
                std::cout << SL.printToString(*MR.SourceManager) << ":" << std::endl;

                XMLDocOut.XMLAddNode(MR.Context, SL, "9.3", "first enumeration does not have integerliteral initialization but at least one other enumeration does : ");
                JSONDocOUT.JSONAddElement(MR.Context, SL, "9.3", "first enumeration does not have integerliteral initialization but at least one other enumeration does : ");
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
        std::cout << SL.printToString(*MR.SourceManager) << ":" << std::endl;

        XMLDocOut.XMLAddNode(MR.Context, SL, "12.3", "sizeof working on an expr with a side-effect : ");
        JSONDocOUT.JSONAddElement(MR.Context, SL, "12.3", "sizeof working on an expr with a side-effect : ");
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
        std::cout << SL.printToString(*MR.SourceManager) << ":" << std::endl;

        XMLDocOut.XMLAddNode(MR.Context, SL, "12.4", "Righ-hand expr has side-effect");
        JSONDocOUT.JSONAddElement(MR.Context, SL, "12.4", "Righ-hand expr has side-effect");
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
            std::cout << SL.printToString(*MR.SourceManager) << ":" << std::endl;

            XMLDocOut.XMLAddNode(MR.Context, SL, "12.5", "RHS and/or LHS operands are not primary expressions : ");
            JSONDocOUT.JSONAddElement(MR.Context, SL, "12.5", "RHS and/or LHS operands are not primary expressions : ");
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
        std::cout << SL.printToString(*MR.SourceManager) << ":" << std::endl;

        XMLDocOut.XMLAddNode(MR.Context, SL, "12.6", "RHS and/or LHS operands are not effectively-boolean values : ");
        JSONDocOUT.JSONAddElement(MR.Context, SL, "12.6", "RHS and/or LHS operands are not effectively-boolean values : ");
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
        std::cout << SL.printToString(*MR.SourceManager) << ":" << std::endl;

        XMLDocOut.XMLAddNode(MR.Context, SL, "12.7", "Bitwise operator has signed RHS and/or LHS operands: ");
        JSONDocOUT.JSONAddElement(MR.Context, SL, "12.7", "Bitwise operator has signed RHS and/or LHS operands: ");
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
          std::cout << SL.printToString(*MR.SourceManager) << ":" << std::endl;

          XMLDocOut.XMLAddNode(MR.Context, SL, "12.8", "shift size should be between zero and one less than the size of the LHS operand: ");
          JSONDocOUT.JSONAddElement(MR.Context, SL, "12.8", "shift size should be between zero and one less than the size of the LHS operand: ");
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
        std::cout << SL.printToString(*MR.SourceManager) << ":" << std::endl;

        XMLDocOut.XMLAddNode(MR.Context, SL, "12.9", "UnaryOperator - has an expr with an unsigned underlying type: ");
        JSONDocOUT.JSONAddElement(MR.Context, SL, "12.9", "UnaryOperator - has an expr with an unsigned underlying type: ");
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
      std::cout << SL.printToString(*MR.SourceManager) << ":" << std::endl;

      XMLDocOut.XMLAddNode(MR.Context, SL, "12.10", "Comma used: ");
      JSONDocOUT.JSONAddElement(MR.Context, SL, "12.10", "Comma used: ");
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
      std::cout << SL.printToString(*MR.SourceManager) << ":" << std::endl;

      XMLDocOut.XMLAddNode(MR.Context, SL, "12.13", "Unary ++ or -- have been used in an expr with other operators: ");
      JSONDocOUT.JSONAddElement(MR.Context, SL, "12.13", "Unary ++ or -- have been used in an expr with other operators: ");
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
        std::cout << SL.printToString(*MR.SourceManager) << ":" << std::endl;

        XMLDocOut.XMLAddNode(MR.Context, SL, "13.1", "assignment operator used in an expr that is known to return boolean: ");
        JSONDocOUT.JSONAddElement(MR.Context, SL, "13.1", "assignment operator used in an expr that is known to return boolean: ");
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
        std::cout << SL.printToString(*MR.SourceManager) << ":" << std::endl;

        XMLDocOut.XMLAddNode(MR.Context, SL, "13.2", "Implicit test of an expr against zero which is not known to return a boolean result: ");
        JSONDocOUT.JSONAddElement(MR.Context, SL, "13.2", "Implicit test of an expr against zero which is not known to return a boolean result: ");
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
      SLD = Devi::SourceLocationHasMacro(SLD, Rewrite, "start");
      NewSL = SLD;

      SourceLocation SL = EXP->getLocStart();
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
              std::cout << SL.printToString(*MR.SourceManager) << ":" << std::endl;

              XMLDocOut.XMLAddNode(MR.Context, SL, "13.3", "Float type expression checked for equality/inequality: ");
              JSONDocOUT.JSONAddElement(MR.Context, SL, "13.3", "Float type expression checked for equality/inequality: ");
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
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

      const Expr* FSCond = FS->getCond();
      const Expr* FSInc = FS->getInc();

#if 0
      if (FSCond != nullptr)
      {
        std::string multix = Rewrite.getRewrittenText(FSCond->getSourceRange());
        std::cout << "diagnostic" << ":" << multix << std::endl;
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
              std::cout << SL.printToString(*MR.SourceManager) << ":" << std::endl;
              AlreadyHaveAHit = true;

              XMLDocOut.XMLAddNode(MR.Context, SL, "13.4", "Float type used in the controlling expression of a forstmt: ");
              JSONDocOUT.JSONAddElement(MR.Context, SL, "13.4", "Float type used in the controlling expression of a forstmt: ");
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
              std::cout << SL.printToString(*MR.SourceManager) << ":" << std::endl;
              AlreadyHaveAHit = true;

              XMLDocOut.XMLAddNode(MR.Context, SL, "13.4", "Float type used in the controlling expression of a forstmt: ");
              JSONDocOUT.JSONAddElement(MR.Context, SL, "13.4", "Float type used in the controlling expression of a forstmt: ");
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
        SourceLocation CSLE = FSCond->getLocEnd();
        SourceRange CSR;
        CSR.setBegin(CSL);
        CSR.setEnd(CSLE);

        std::string outstring = Rewrite.getRewrittenText(CSR);

#if 0
        std::cout << "XXXXXXXXXXXXXXXXXXXXXX" << outstring << std::endl;
#endif
      }


      SourceLocation SLD = FS->getLocStart();
      SLD = Devi::SourceLocationHasMacro(SLD, Rewrite, "start");
      SourceLocation SL = DRE->getLocStart();
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
                std::cout << SL.printToString(*MR.SourceManager) << ":" << std::endl;

                XMLDocOut.XMLAddNode(MR.Context, SL, "13.6", "ForStmt controlling variable modified in the body of the loop: ");
                JSONDocOUT.JSONAddElement(MR.Context, SL, "13.6", "ForStmt controlling variable modified in the body of the loop: ");
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
      std::cout << SL.printToString(*MR.SourceManager) << ":" << std::endl;

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
      std::cout << SL.printToString(*MR.SourceManager) << ":" << std::endl;

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
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");
    }

    if (MR.Nodes.getNodeAs<clang::WhileStmt>("mccfwuwu") != nullptr)
    {
      const WhileStmt* WS =  MR.Nodes.getNodeAs<clang::WhileStmt>("mccfwuwu");

      SL = WS->getLocStart();
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");
    }

    if (MR.Nodes.getNodeAs<clang::DoStmt>("mccfdodo") != nullptr)
    {
      const DoStmt* DS =  MR.Nodes.getNodeAs<clang::DoStmt>("mccfdodo");

      SL = DS->getLocStart();
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
          std::cout << SL.printToString(*MR.SourceManager) << ":" << std::endl;
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
            std::cout << SL.printToString(*MR.SourceManager) << ":" << std::endl;
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
          std::cout << SL.printToString(*MR.SourceManager) << ":" << std::endl;

          XMLDocOut.XMLAddNode(MR.Context, SL, "14.8", "ForStmt does not have a child CompoundStmt: ");
          JSONDocOUT.JSONAddElement(MR.Context, SL, "14.8", "ForStmt does not have a child CompoundStmt: ");
        }
      }
    }

    if (MR.Nodes.getNodeAs<clang::WhileStmt>("mccf148while") != nullptr)
    {
      const WhileStmt* WS = MR.Nodes.getNodeAs<clang::WhileStmt>("mccf148while");

      SL = WS->getLocStart();
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
          std::cout << SL.printToString(*MR.SourceManager) << ":" << std::endl;

          XMLDocOut.XMLAddNode(MR.Context, SL, "14.8", "WhileStmt does not have a child CompoundStmt: ");
          JSONDocOUT.JSONAddElement(MR.Context, SL, "14.8", "WhileStmt does not have a child CompoundStmt: ");
        }
      }
    }

    if (MR.Nodes.getNodeAs<clang::DoStmt>("mccf148do") != nullptr)
    {
      const DoStmt* DS = MR.Nodes.getNodeAs<clang::DoStmt>("mccf148do");

      SL = DS->getLocStart();
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
          std::cout << SL.printToString(*MR.SourceManager) << ":" << std::endl;

          XMLDocOut.XMLAddNode(MR.Context, SL, "14.8", "DoStmt does not have a child CompoundStmt: ");
          JSONDocOUT.JSONAddElement(MR.Context, SL, "14.8", "DoStmt does not have a child CompoundStmt: ");
        }
      }
    }

    if (MR.Nodes.getNodeAs<clang::SwitchStmt>("mccf148switch") != nullptr)
    {
      const SwitchStmt* SS = MR.Nodes.getNodeAs<clang::SwitchStmt>("mccf148switch");

      SL = SS->getLocStart();
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
          std::cout << SL.printToString(*MR.SourceManager) << ":" << std::endl;

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
      std::cout << SL.printToString(*MR.SourceManager) << ":" << std::endl;

      XMLDocOut.XMLAddNode(MR.Context, SL, "15.4", "Switch expression is effectively boolean: ");
      JSONDocOUT.JSONAddElement(MR.Context, SL, "15.4", "Switch expression is effectively boolean: ");
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
              std::cout << SL.printToString(*MR.SourceManager) << ":" << std::endl;

              XMLDocOut.XMLAddNode(MR.Context, SL, "11.1", "ImplicitCastExpr - FunctionPointerType converted to or from a type other than IntegralType: ");
              JSONDocOUT.JSONAddElement(MR.Context, SL, "11.1", "ImplicitCastExpr - FunctionPointerType converted to or from a type other than IntegralType: ");
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
            std::cout << SL.printToString(*MR.SourceManager) << ":" << std::endl;

            XMLDocOut.XMLAddNode(MR.Context, SL, "10.1/2", "ImplicitCastExpr - Conversion of FloatingType to or from IntegralType is recommended against: ");
            JSONDocOUT.JSONAddElement(MR.Context, SL, "10.1/2", "ImplicitCastExpr - Conversion of FloatingType to or from IntegralType is recommended against: ");
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
            std::cout << SL.printToString(*MR.SourceManager) << ":" << std::endl;

            XMLDocOut.XMLAddNode(MR.Context, SL, "11.3", "ImplicitCastExpr - Conversion of PointerType to or from IntegralType is recommended against: ");
            JSONDocOUT.JSONAddElement(MR.Context, SL, "11.3", "ImplicitCastExpr - Conversion of PointerType to or from IntegralType is recommended against: ");
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
            std::cout << SL.printToString(*MR.SourceManager) << ":" << std::endl;

            XMLDocOut.XMLAddNode(MR.Context, SL, "11.x", "ImplicitCastExpr - PointerType has implicit BitCast. This could be caused by a cast removing const or volatile qualifier from the type addressed by a pointer or by a cast to a different function or object type: ");
            JSONDocOUT.JSONAddElement(MR.Context, SL, "11.x", "ImplicitCastExpr - PointerType has implicit BitCast. This could be caused by a cast removing const or volatile qualifier from the type addressed by a pointer or by a cast to a different function or object type: ");
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
              std::cout << SL.printToString(*MR.SourceManager) << ":" << std::endl;

              XMLDocOut.XMLAddNode(MR.Context, SL, "13.7", "EffectivelyBooleanExpr's result is known at compile-time: ");
              JSONDocOUT.JSONAddElement(MR.Context, SL, "13.7", "EffectivelyBooleanExpr's result is known at compile-time: ");
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
      CESL = Devi::SourceLocationHasMacro(CESL, Rewrite, "start");
      SourceLocation FDDadSL = FDDad->getLocStart();
      FDDadSL = Devi::SourceLocationHasMacro(FDDadSL, Rewrite, "start");
      SourceLocation FDSL = FDDef->getLocStart();
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
            std::cout << FuncScopeProto[x].DefinitionSL << ":" << std::endl;

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
      std::cout << SL.printToString(*MR.SourceManager) << ":" << std::endl;

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
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");
      SourceLocation SLE = FD->getBody()->getLocStart();
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
        std::cout << SL.printToString(*MR.SourceManager) << ":" << std::endl;

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
        std::cout << SL.printToString(*MR.SourceManager) << ":" << std::endl;

        XMLDocOut.XMLAddNode(MR.Context, SL, "17.1", "Pointer arithmatic for non-array pointers : ");
        JSONDocOUT.JSONAddElement(MR.Context, SL, "17.1", "Pointer arithmatic for non-array pointers : ");
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
        std::cout << SL.printToString(*MR.SourceManager) << ":" << std::endl;

        XMLDocOut.XMLAddNode(MR.Context, SL, "17.2 | 17.3", "Pointer-type operands to BinaryOperator dont point to the same array : ");
        JSONDocOUT.JSONAddElement(MR.Context, SL, "17.2 | 17.3", "Pointer-type operands to BinaryOperator dont point to the same array : ");
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
          std::cout << SL.printToString(*MR.SourceManager) << ":" << std::endl;

          XMLDocOut.XMLAddNode(MR.Context, SL, "17.4", "The only allowed form of pointer arithmetic is array indexing : ");
          JSONDocOUT.JSONAddElement(MR.Context, SL, "17.4", "The only allowed form of pointer arithmetic is array indexing : ");
        }
      }
    }

    if (MR.Nodes.getNodeAs<clang::DeclRefExpr>("mcpointer1742") != nullptr)
    {
      const DeclRefExpr* DRE = MR.Nodes.getNodeAs<clang::DeclRefExpr>("mcpointer1742");

      SourceLocation SL = DRE->getLocStart();
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
          std::cout << SL.printToString(*MR.SourceManager) << ":" << std::endl;

          XMLDocOut.XMLAddNode(MR.Context, SL, "17.4", "The only allowed form of pointer arithmetic is array indexing : ");
          JSONDocOUT.JSONAddElement(MR.Context, SL, "17.4", "The only allowed form of pointer arithmetic is array indexing : ");
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
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

      QT = VD->getType();
    }

    if (MR.Nodes.getNodeAs<clang::FieldDecl>("mcpointer175field") != nullptr)
    {
      FD = MR.Nodes.getNodeAs<clang::FieldDecl>("mcpointer175field");

      SL = FD->getSourceRange().getBegin();
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
            std::cout << SL.printToString(*MR.SourceManager) << ":" << std::endl;

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
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");
      SourceLocation SLE = EXP->getLocEnd();
      SLE = Devi::SourceLocationHasMacro(SLE, Rewrite, "start");

      SourceRange SR;
      SR.setBegin(SL);
      SR.setEnd(SLE);

#if 0
      std::string RHSString = Rewrite.getRewrittenText(SR);

      //std::cout << RHSString << "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << std::endl;

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
        //std::cout << RHSString << "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << SL.printToString(*MR.SourceManager) << std::endl;

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
              std::cout << SL.printToString(*MR.SourceManager) << ":" << std::endl;

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
        //std::cout << RHSString << "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << SL.printToString(*MR.SourceManager) << std::endl;

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
              std::cout << SL.printToString(*MR.SourceManager) << ":" << std::endl;

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
      std::cout << SL.printToString(*MR.SourceManager) << ":" << std::endl;

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
              std::cout << SL.printToString(*MR.SourceManager) << ":" << std::endl;

              XMLDocOut.XMLAddNode(MR.Context, SL, "11.1", "CStyleCastExpr - FunctionPointerType converted to or from a type other than IntegralType: ");
              JSONDocOUT.JSONAddElement(MR.Context, SL, "11.1", "CStyleCastExpr - FunctionPointerType converted to or from a type other than IntegralType: ");
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
            std::cout << SL.printToString(*MR.SourceManager) << ":" << std::endl;

            XMLDocOut.XMLAddNode(MR.Context, SL, "11.3", "CStyleCastExpr - Conversion of PointerType to or from IntegralType is recommended against: ");
            JSONDocOUT.JSONAddElement(MR.Context, SL, "11.3", "CStyleCastExpr - Conversion of PointerType to or from IntegralType is recommended against: ");
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
            std::cout << SL.printToString(*MR.SourceManager) << ":" << std::endl;

            XMLDocOut.XMLAddNode(MR.Context, SL, "11.x", "CStyleCastExpr - PointerType has implicit BitCast. This could be caused by a cast removing const or volatile qualifier from the type addressed by a pointer or by a cast to a different function or object type: ");
            JSONDocOUT.JSONAddElement(MR.Context, SL, "11.x", "CStyleCastExpr - PointerType has implicit BitCast. This could be caused by a cast removing const or volatile qualifier from the type addressed by a pointer or by a cast to a different function or object type: ");
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

        bool ICETypeIsSignedInt = CanonTypeDaddy->getAsPlaceholderType()->isSignedInteger();
        bool ChildTypeIsSignedInt = CanonTypeChild->getAsPlaceholderType()->isSignedInteger();

        bool ICETypeIsUSignedInt = CanonTypeDaddy->getAsPlaceholderType()->isUnsignedInteger();
        bool ChildTypeIsUSignedInt = CanonTypeChild->getAsPlaceholderType()->isUnsignedInteger();

        if (CanonTypeDaddy->getAsPlaceholderType()->isInteger() && CanonTypeChild->getAsPlaceholderType()->isInteger())
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
                std::cout << SL.printToString(*MR.SourceManager) << ":" << std::endl;

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
              std::cout << SL.printToString(*MR.SourceManager) << ":" << std::endl;

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
                std::cout << SL.printToString(*MR.SourceManager) << ":" << std::endl;

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

          bool IsSignedCPXDaddy = DaddyCPXElementType->getAsPlaceholderType()->isSignedInteger();
          bool IsSignedCPXChild = ChildCPXElementType->getAsPlaceholderType()->isSignedInteger();
          bool IsUnsignedCPXDaddy = DaddyCPXElementType->getAsPlaceholderType()->isUnsignedInteger();
          bool IsUnsignedCPXChild = ChildCPXElementType->getAsPlaceholderType()->isUnsignedInteger();

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
                std::cout << SL.printToString(*MR.SourceManager) << ":" << std::endl;

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
                std::cout << SL.printToString(*MR.SourceManager) << ":" << std::endl;

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
class MCIdent5 : public MatchFinder::MatchCallback
{
public:
  MCIdent5 (Rewriter &Rewrite) : Rewrite(Rewrite) {}

  virtual void run(const MatchFinder::MatchResult &MR)
  {
    IsTypedefIdent = false;
    IsNamedDecl = false;
    IsRecordDecl = false;
    HasHiddenVisibility = false;

    IdentifierInfo *II;

    SourceLocation SL;

    if (MR.Nodes.getNodeAs<clang::NamedDecl>("ident5var") != nullptr)
    {
      const NamedDecl* ND = MR.Nodes.getNodeAs<clang::NamedDecl>("ident5var");

      II = ND->getIdentifier();

      SL = ND->getLocStart();
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

      if (ND->isHidden())
      {
        HasHiddenVisibility = true;
      }

      IsNamedDecl = true;
    }

    if (MR.Nodes.getNodeAs<clang::TypedefDecl>("ident5typedef") != nullptr)
    {
      const TypedefDecl* TDD = MR.Nodes.getNodeAs<clang::TypedefDecl>("ident5typedef");

      II = TDD->getIdentifier();

      SL = TDD->getLocStart();
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

      IsTypedefIdent = true;
    }

    if (MR.Nodes.getNodeAs<clang::RecordDecl>("ident5record") != nullptr)
    {
      const RecordDecl* RD = MR.Nodes.getNodeAs<clang::RecordDecl>("ident5record");

      II = RD->getCanonicalDecl()->getIdentifier();

      SL = RD->getLocStart();
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

      IsRecordDecl = true;
    }

    ASTContext *const ASTC = MR.Context;
    const SourceManager &SM = ASTC->getSourceManager();

    if (HasHiddenVisibility)
    {
      if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
      {
        /*intentionally left blank*/
      }
      else
      {
        if (Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
        {
          std::cout << "5.2:" << "Object or function has hidden visibility:";
          std::cout << SL.printToString(*MR.SourceManager) << ":" << std::endl;

          XMLDocOut.XMLAddNode(MR.Context, SL, "5.2", "Object or function has hidden visibility: ");
          JSONDocOUT.JSONAddElement(MR.Context, SL, "5.2", "Object or function has hidden visibility: ");
        }
      }
    }

    const IdentifierTable &IT = ASTC->Idents;

    if (II != nullptr)
    {
      StringRef IdentStringRef = II->getName();

      IdentifierInfoLookup* IILU [[maybe_unused]] = IT.getExternalIdentifierLookup();

      for (auto &iter : IT)
      {
        /*@DEVI-only works for UTF-8. for larger sizes we need a multiple of 32. for UTF-16 we need to check against 64 and so on.*/
        if (IdentStringRef.str().size() >= 32U && IsNamedDecl)
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
                std::cout << SL.printToString(*MR.SourceManager) << ":" << std::endl;

                XMLDocOut.XMLAddNode(MR.Context, SL, "5.1", "Identifier relies on the significance of more than 31 charcaters: ");
                JSONDocOUT.JSONAddElement(MR.Context, SL, "5.1", "Identifier relies on the significance of more than 31 charcaters: ");
              }
            }
          }
        }
      }

      if (SM.isInMainFile(SL))
      {
        IdentInfo Temp = {II->getName().str(), SL, SL.printToString(*MR.SourceManager), ASTC->getFullLoc(SL), IsRecordDecl, IsTypedefIdent};
        IdentInfoProto.push_back(Temp);
      }
    }
  }

  void onEndOfTranslationUnit(void)
  {
    unsigned TagCounter = 0U;
    unsigned TypedefCounter = 0U;

    /*@DEVI-3 is the magical number because it will find itself twice, because typedef and tag
    identifiers are matched twice, once by nameddecl  and once by tag or typedef. the third time
    we find a match is where we have found something to tag.*/
    /*@DEVI-has false positives-incomplete record types that later become complete fit the criteria.*/
    for (auto &iter : IdentInfoProto)
    {
      if (iter.IsTagIdentifier)
      {
        for (auto &yaiter : IdentInfoProto)
        {
          if ((iter.Name == yaiter.Name))
          {
            TagCounter++;
          }
        }

        if (TagCounter >= 3U)
        {
          if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, iter.FSL.isInSystemHeader(), iter.SL))
          {
            /*intentionally elft blank*/
          }
          else
          {
            if (Devi::IsTheMatchInMainFile(MainFileOnly, iter.FSL.getManager().isInMainFile(iter.SL), iter.SL))
            {
              std::cout << "5.4:" << "tag identifier is not unique:";
              std::cout << iter.SLString << ":" << std::endl;

              XMLDocOut.XMLAddNode(iter.FSL, iter.SL, "5.4", "tag identifier is not unique: ");
              JSONDocOUT.JSONAddElement(iter.FSL, iter.SL, "5.4", "tag identifier is not unique: ");
            }
          }
        }

        TagCounter = 0U;
      }

      if (iter.IsTypeDefIdentifier)
      {
        for (auto &yaiter : IdentInfoProto)
        {
          if ((iter.Name == yaiter.Name))
          {
            TypedefCounter++;
          }
        }

        if (TypedefCounter >= 3U)
        {
          if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, iter.FSL.isInSystemHeader(), iter.SL))
          {
            /*intentionally left blank*/
          }
          else
          {
            if (Devi::IsTheMatchInMainFile(MainFileOnly, iter.FSL.getManager().isInMainFile(iter.SL), iter.SL))
            {
              std::cout << "5.3:" << "typedef identifier is not unique:";
              std::cout << iter.SLString << ":" << std::endl;

              XMLDocOut.XMLAddNode(iter.FSL, iter.SL, "5.3", "typedef identifier is not unique: ");
              JSONDocOUT.JSONAddElement(iter.FSL, iter.SL, "5.3", "typedef identifier is not unique: ");
            }
          }
        }

        TypedefCounter = 0U;
      }
    }
  }

private:
  bool IsTypedefIdent = false;
  bool IsNamedDecl = false;
  bool IsRecordDecl = false;
  bool HasHiddenVisibility = false;

  struct IdentInfo
  {
    IdentInfo(std::string iName, SourceLocation iSL, std::string iSLString, FullSourceLoc iFSL, bool iIsTagIdentifier = false, bool iIsTypeDefIdentifier = false)
    {
      Name = iName;
      SL = iSL;
      SLString = iSLString;
      FSL = iFSL;
      IsTagIdentifier = iIsTagIdentifier;
      IsTypeDefIdentifier = iIsTypeDefIdentifier;
    }

    std::string Name;
    SourceLocation SL;
    std::string SLString;
    FullSourceLoc FSL;
    bool IsTagIdentifier = false;
    bool IsTypeDefIdentifier = false;
  };

  std::vector<IdentInfo> IdentInfoProto;



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
            std::cout << iter.ObjSLStr << ":" << std::endl;

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
        std::cout << "diagnostic2:" << "Variable:" << NDNameString << ":" << iter.XObjNameStr << std::endl;
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

        //std::cout << "ZZZZZZZZZZZZZZZZZZZZZ" << RawText << std::endl;

        SourceLocation RCSL = iter->getLocStart();

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
                std::cout << RCSL.printToString(*MR.SourceManager) << ":" << std::endl;

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
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

      if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
      {
        return void();
      }

      QualType QT = PVD->getOriginalType();

      ASTContext *const ASTC = MR.Context;

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
              std::cout << SL.printToString(*MR.SourceManager) << ":" << std::endl;

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
          std::cout << SL.printToString(*MR.SourceManager) << ":" << std::endl;

          XMLDocOut.XMLAddNode(MR.Context, SL, "6.1", "Simple char type should only hold character values:");
          JSONDocOUT.JSONAddElement(MR.Context, SL, "6.1", "Simple char type should only hold character values:");
        }
      }

      if (CanonTP->isSpecificBuiltinType(BuiltinType::Kind::UChar) || CanonTP->isSpecificBuiltinType(BuiltinType::Kind::SChar))
      {
        if (RHSIsCharLit)
        {
          std::cout << "6.2:" << "Signed or unsigned char type should only hold numeric values:";
          std::cout << SL.printToString(*MR.SourceManager) << ":" << std::endl;

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
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

      if (Devi::IsTheMatchInSysHeader(CheckSystemHeader, MR, SL))
      {
        return void();
      }

      if (Devi::IsTheMatchInMainFile(MainFileOnly, MR, SL))
      {
        std::cout << "12.12:" << "Possible violation of 12.12-access to the underlying bit representation of a floating type:";
        std::cout << SL.printToString(*MR.SourceManager) << ":" << std::endl;

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

      bool TypeIsUSignedInt = CanonTP->getAsPlaceholderType()->isUnsignedInteger();

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
          std::cout << "12.11" << ":" << "Constant Unsinged Expr evaluation resuslts in an overflow:" << SL.printToString(*MR.SourceManager) << ":" << IntExprValue << " " << DousFinal << " " << ":" << targetExpr << std::endl;

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
        SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");
      }

      if (MR.Nodes.getNodeAs<clang::UnaryOperator>("mcatc105uno") != nullptr)
      {
        const UnaryOperator* UO = MR.Nodes.getNodeAs<clang::UnaryOperator>("mcatc105uno");
        DynOpNode = ast_type_traits::DynTypedNode::create<clang::UnaryOperator>(*UO);

        SL = UO->getLocStart();
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
            std::cout << "10.5" << ":" << "Result of operands << or ~ must be explicitly cast to the type of the expression:" << SL.printToString(*MR.SourceManager) << ":" << std::endl;

            XMLDocOut.XMLAddNode(MR.Context, SL, "10.5", "Result of operands << or ~ must be explicitly cast to the type of the expression:");
            JSONDocOUT.JSONAddElement(MR.Context, SL, "10.5", "Result of operands << or ~ must be explicitly cast to the type of the expression:");
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
        std::cout << "13.5" << ":" << "The three expressions of a ForStmt shall either all exist or not exist at all or only the initialization can be missing:" << SL.printToString(*MR.SourceManager) << ":" << std::endl;

        XMLDocOut.XMLAddNode(MR.Context, SL, "13.5", "The three expressions of a ForStmt shall either all exist or not exist at all or only the initialization can be missing:");
        JSONDocOUT.JSONAddElement(MR.Context, SL, "13.5", "The three expressions of a ForStmt shall either all exist or not exist at all or only the initialization can be missing:");
      }

      if (FSInc != nullptr)
      {
        if (!FSInc->HasSideEffects(*ASTC, true))
        {
          std::cout << "13.5" << ":" << "The increment expression in the ForStmt has no side-effects:" << SL.printToString(*MR.SourceManager) << ":" << std::endl;

          XMLDocOut.XMLAddNode(MR.Context, SL, "13.5", "The increment expression in the ForStmt has no side-effects:");
          JSONDocOUT.JSONAddElement(MR.Context, SL, "13.5", "The increment expression in the ForStmt has no side-effects:");
        }
      }

      if (FSCond != nullptr)
      {
        if (FSCond->HasSideEffects(*ASTC, true))
        {
          std::cout << "13.5" << ":" << "The condition expression in the ForStmt has side-effect:" << SL.printToString(*MR.SourceManager) << ":" << std::endl;

          XMLDocOut.XMLAddNode(MR.Context, SL, "13.5", "The condition expression in the ForStmt has side-effect:");
          JSONDocOUT.JSONAddElement(MR.Context, SL, "13.5", "The condition expression in the ForStmt has side-effect:");
        }

        if (!FSCond->isKnownToHaveBooleanValue())
        {
          std::cout << "13.5" << ":" << "The expression in the ForStmt condition does not return a boolean:" << SL.printToString(*MR.SourceManager) << ":" << std::endl;

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
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "strat");

      TagCandidateString = StringLit->getString().str();
    }

    if (MR.Nodes.getNodeAs<clang::CharacterLiteral>("mcconst71char") != nullptr)
    {
      const CharacterLiteral* CL = MR.Nodes.getNodeAs<clang::CharacterLiteral>("mcconst71char");

      SL = CL->getLocStart();
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
    std::cout << "diagnostic2:" << TagCandidateString << ":" << SL.printToString(*MR.SourceManager) << ":" << std::regex_search(TagCandidateString, result, octalconstant) << std::endl;
#endif

    if (std::regex_search(TagCandidateString, result, octalconstant) || std::regex_search(TagCandidateString, result, octalconstantint))
    {
      std::cout << "7.1" << ":" << "Octal escape sequence used:" << SL.printToString(*MR.SourceManager) << ":" << TagCandidateString << std::endl;

      XMLDocOut.XMLAddNode(MR.Context, SL, "7.1", "Octal escape sequence used:");
      JSONDocOUT.JSONAddElement(MR.Context, SL, "7.1", "Octal escape sequence used:");
    }

    if (std::regex_search(TagCandidateString, result, hexescapesequence))
    {
      std::cout << "4.1" << ":" << "Hexadecimal escape sequence used:" << SL.printToString(*MR.SourceManager) << ":" << TagCandidateString << std::endl;

      XMLDocOut.XMLAddNode(MR.Context, SL, "4.1", "Hexadecimal escape sequence used:");
      JSONDocOUT.JSONAddElement(MR.Context, SL, "4.1", "Hexadecimal escape sequence used:");
    }

    if (std::regex_search(TagCandidateString, result, otherescapesequence))
    {
      std::cout << "4.1" << ":" << "Non-standard escape sequence used:" << SL.printToString(*MR.SourceManager) << ":" << TagCandidateString << std::endl;

      XMLDocOut.XMLAddNode(MR.Context, SL, "4.1", "Non-standard escape sequence used:");
      JSONDocOUT.JSONAddElement(MR.Context, SL, "4.1", "Non-standard escape sequence used:");
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
/*the sourcelocation used in the overload of XMLAddNode that takes sourcemanager as input parameter uses
the spelling location so the client does not need to check the sourcelocation for macros expansions.*/
class PPInclusion : public PPCallbacks
{
public:
  explicit PPInclusion (SourceManager *SM) : SM(*SM) {}

  virtual void InclusionDirective (SourceLocation HashLoc, const Token &IncludeTok, StringRef FileName, \
                                   bool IsAngled, CharSourceRange FileNameRange, const FileEntry* File, \
                                   StringRef SearchPath, StringRef RelativePath, const clang::Module* Imported)
  {
    if (File->isValid())
    {
#if 0
      assert(HashLoc.isValid() && "The SourceLocation for InclusionDirective is invalid.");
#endif

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
              std::cout << HashLoc.printToString(SM) << ":" << std::endl;

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
              std::cout << HashLoc.printToString(SM) << ":" << std::endl;

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
              std::cout << HashLoc.printToString(SM) << ":" << std::endl;

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
              std::cout << HashLoc.printToString(SM) << ":" << std::endl;

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
              std::cout << HashLoc.printToString(SM) << ":" << std::endl;

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
              std::cout << HashLoc.printToString(SM) << "\n" << std::endl;

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
            std::cout << HashLoc.printToString(SM) << ":" << std::endl;

            XMLDocOut.XMLAddNode(SM, HashLoc, "19.3", "Include directive contains file address, not just name : ");
            JSONDocOUT.JSONAddElement(SM, HashLoc, "19.3", "Include directive contains file address, not just name : ");
          }
        }
      }
    }
  }

  /*@DEVI-if the macro is not checked for being defined before almost any kind of use, the code will break in seemingly random ways.*/
  /*@DEVI-FIXME-the macro definition is the definition of the macro passed to defined. not sure what happens if there are more than two.
  basically i dont know how to just get the tokens after defined.*/
  virtual void Defined(const Token &MacroNameTok, const MacroDefinition &MD, SourceRange Range)
  {
#if 1
    SourceLocation SL [[maybe_unused]] = Range.getBegin();

#if 0
    assert(SL.isValid(), "the SourceLocation for macro Defined is not valid.");
#endif

    const MacroInfo* MI = MD.getMacroInfo();

    DefMacroDirective* DMD = MD.getLocalDirective();

    bool ShouldBeTagged194 = false;

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
        std::cout << "19.14 : " << "Illegal \"defined\" form : " << std::endl;
        std::cout << SL.printToString(SM) << "\n" << std::endl;

        XMLDocOut.XMLAddNode(SM, SL, "19.14", "Illegal \"defined\" form : ");
#endif
      }
    }
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
            std::cout << SL.printToString(SM) << ":" << std::endl;

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
                std::cout << SL.printToString(SM) << ":" << std::endl;

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
          std::cout << SL.printToString(SM) << ":" << std::endl;

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
          std::cout << SL.printToString(SM) << ":" << std::endl;

          XMLDocOut.XMLAddNode(SM, SL, "20.1", "C keyword defined : ");
          JSONDocOUT.JSONAddElement(SM, SL, "20.1", "C keyword defined : ");
        }
      }
    }

    if (MD->getPrevious() != nullptr)
    {
      const MacroDirective* PMD = MD->getPrevious();
      SourceLocation PSL = PMD->getLocation();

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
            std::cout << SL.printToString(SM) << ":" << std::endl;

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
            std::cout << SL.printToString(SM) << ":" << std::endl;

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
            std::cout << SL.printToString(SM) << ":" << std::endl;

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
          std::cout << SL.printToString(SM) << ":" << std::endl;

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
            std::cout << SL.printToString(SM) << ":" << std::endl;

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
            std::cout << SL.printToString(SM) << ":" << std::endl;

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
                std::cout << SL.printToString(SM) << ":" << std::endl;

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

#if 0
    assert(SL.isValid(), "the SourceLocation for MacroExpands is not valid.");
#endif

    IdentifierInfo* II = MacroNameTok.getIdentifierInfo();

    std::string MacroNameString = II->getName().str();

    DefMacroDirective* DMD = MD.getLocalDirective();

    SourceLocation MDSL = DMD->getLocation();

    MacroInfo* MI = MD.getMacroInfo();

    /*underdev2*/
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

        for (ArrayRef<Token>::iterator iter = TokenArrayRef.begin(); iter != TokenArrayRef.end(); ++iter)
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
            std::cout << Range.getBegin().printToString(SM) << ":" << std::endl;

            XMLDocOut.XMLAddNode(SM, SL, "19.4", "Macro does not expand to braced initializer,panthesizes expression,string literal,constant,do-while-zero,storage class specifier or type qualifier : ");
            JSONDocOUT.JSONAddElement(SM, SL, "19.4", "Macro does not expand to braced initializer,panthesizes expression,string literal,constant,do-while-zero,storage class specifier or type qualifier : ");
          }
        }
      }
    }
    /*end of 19.4*/

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
          std::cout << Range.getBegin().printToString(SM) << ":" << std::endl;

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
          std::cout << Range.getBegin().printToString(SM) << ":" << std::endl;

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
          std::cout << Range.getBegin().printToString(SM) << ":" << std::endl;

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
    SourceLocation SIfLoc = SM.getSpellingLoc(IfLoc);

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
          std::cout << SLoc.printToString(SM) << ":" << std::endl;

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
    SourceLocation SIfLoc = SM.getSpellingLoc(IfLoc);

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
          std::cout << SLoc.printToString(SM) << ":" << std::endl;

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
    SourceLocation SIfLoc = SM.getSpellingLoc(IfLoc);

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
          std::cout << SLoc.printToString(SM) << ":" << std::endl;

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
      //std::cout << iter << std::endl;
      std::ifstream InputFile(iter);

      HaveWeMatchIllegal191Yet = false;
      HaveWeMatchedInclusionDirYet = false;

      for (std::string line; getline(InputFile, line);)
      {
        //std::cout << iter << ":" << line << ":" << HaveWeMatchedInclusionDirYet << " " << HaveWeMatchIllegal191Yet << std::endl;

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
              std::cout << "19.1" << ":" << "Inclusion directives should only be preceeded by other inclusion directives, pp directives or comments" << ":" << iter << std::endl;

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
                  std::cout << "19.1" << ":" << "Inclusion directives should only be preceeded by other inclusion directives, pp directives or comments" << ":" << iter << std::endl;

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
#if 1
      std::cout << iter.Line << ":" << iter.Column << ":" << iter.FileName << std::endl;
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
              std::cout << "14.3" << ":" << "Illegal NullStmt form:" << iter.FileName << ":" << iter.Line << ":" << iter.Column << ":" << std::endl;

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
        std::cout << "8.8:" << "External function or object (" + iter.XObjNameStr + ") is defined in more than one file:";
        std::cout << iter.XObjSLStr << ":" << std::endl;

        XMLDocOut.XMLAddNode(iter.LineNumber, iter.ColumnNumber, iter.FileName, "8.8", "External function or object (" + iter.XObjNameStr + ") is defined in more than one file: ");
        JSONDocOUT.JSONAddElement(iter.LineNumber, iter.ColumnNumber, iter.FileName, "8.8", "External function or object (" + iter.XObjNameStr + ") is defined in more than one file: ");
      }
    }
    /*end of 8.8*/
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
    HandlerForMCPTCCSTYLE(R), HandlerForATC101(R), HandlerForIdent5(R), HandlerForDCDF87(R), HandlerForLangX23(R), \
    HandlerForFunction167(R), HandlerForCF143(R), HandlerForExpr1212(R), HandlerForExpr1211(R), HandlerForAtc105(R), HandlerForCSE135(R), \
    HandlerForTypes612(R), HandlerForDCDF88(R), HandlerForConst71(R) {

#if 1
    /*forstmts whithout a compound statement.*/
    Matcher.addMatcher(forStmt(unless(hasDescendant(compoundStmt()))).bind("mcfor"), &HandlerForCmpless);

    /*whilestmts without a compound statement.*/
    Matcher.addMatcher(whileStmt(unless(hasDescendant(compoundStmt()))).bind("mcwhile"), &HandlerWhileCmpless);

    /*else blocks that dont have a compound statemnt.*/
    Matcher.addMatcher(ifStmt(allOf(hasElse(unless(ifStmt())), hasElse(unless(compoundStmt())))).bind("mcelse"), &HandlerElseCmpless);

    /*if blocks that dont have a compound statement.*/
    Matcher.addMatcher(ifStmt(unless(hasDescendant(compoundStmt()))).bind("mcif"), &HandlerIfCmpless);

    /*if-elseif statements that are missing the else block.*/
    Matcher.addMatcher(ifStmt(allOf(hasElse(ifStmt()), unless(hasAncestor(ifStmt())), unless(hasDescendant(ifStmt(hasElse(unless(ifStmt()))))))).bind("mcifelse"), &HandlerForIfElse);

    Matcher.addMatcher(switchStmt(hasDescendant(compoundStmt(hasDescendant(switchCase(unless(hasDescendant(breakStmt()))))))).bind("mcswitchbrk"), &HandlerForSwitchBrkLess);

    Matcher.addMatcher(switchStmt(unless(hasDescendant(defaultStmt()))).bind("mcswitchdft"), &HandlerForSwitchDftLEss);

    Matcher.addMatcher(switchStmt(forEachDescendant(caseStmt(hasAncestor(compoundStmt().bind("mccmp151"))).bind("mccase151"))), &HandlerForMCSwitch151);

    Matcher.addMatcher(switchStmt(unless(hasDescendant(caseStmt()))).bind("mcswitch155"), &HandlerForMCSwitch155);

    Matcher.addMatcher(functionDecl().bind("mcfunction161"), &HandlerForMCFunction161);

    Matcher.addMatcher(functionDecl(forEachDescendant(callExpr().bind("mc162callexpr"))).bind("mc162funcdec"), &HandlerForFunction162);

    Matcher.addMatcher(functionDecl().bind("mcfunc164"), &HandlerForFunction164);

    Matcher.addMatcher(callExpr().bind("mcfunc166"), &HandlerForFunction166);

    Matcher.addMatcher(functionDecl(forEachDescendant(returnStmt().bind("mcfunc168"))), &HandlerForFunction168);

    Matcher.addMatcher(implicitCastExpr(unless(hasAncestor(callExpr()))).bind("mcfunc169"), &HandlerForFunction169);

    Matcher.addMatcher(varDecl().bind("mcpa171"), &HandlerForPA171);

    Matcher.addMatcher(recordDecl(isUnion()).bind("mcsu184"), &HandlerForSU184);

    Matcher.addMatcher(fieldDecl(isBitField()).bind("mctype6465"), &HandlerForType6465);

    Matcher.addMatcher(functionDecl().bind("mcdcdf81"), &HandlerForDCDF81);

    Matcher.addMatcher(varDecl().bind("mcdcdf82"), &HandlerForDCDF82);

    Matcher.addMatcher(varDecl().bind("mcinit91"), &HandlerForInit91);

    Matcher.addMatcher(initListExpr(hasAncestor(varDecl().bind("mcinit92daddy"))).bind("mcinit92"), &HandlerForInit92);

    Matcher.addMatcher(enumConstantDecl(anyOf(allOf(hasDescendant(integerLiteral().bind("mcinit93kiddy")), \
                                        hasAncestor(enumDecl().bind("mcinit93daddy"))), hasAncestor(enumDecl().bind("mcinit93daddy")))).bind("mcinit93"), &HandlerForInit93);

    Matcher.addMatcher(unaryExprOrTypeTraitExpr(hasDescendant(expr().bind("mcexpr123kiddy"))).bind("mcexpr123"), &HandlerForExpr123);

    Matcher.addMatcher(binaryOperator(allOf(eachOf(hasOperatorName("||"), hasOperatorName("&&")), hasRHS(expr().bind("mcexpr124")))), &HandlerForExpr124);

    Matcher.addMatcher(binaryOperator(allOf(eachOf(hasOperatorName("||"), hasOperatorName("&&")), \
                                            eachOf(hasRHS(allOf(expr().bind("lrhs"), unless(anyOf(implicitCastExpr() , declRefExpr(), callExpr(), floatLiteral(), integerLiteral(), stringLiteral()))))\
                                                , hasLHS(allOf(expr().bind("lrhs"), unless(anyOf(implicitCastExpr(), declRefExpr(), callExpr(), floatLiteral(), integerLiteral(), stringLiteral())))))))\
                       , &HandlerForExpr125);
    Matcher.addMatcher(binaryOperator(allOf(eachOf(hasOperatorName("||"), hasOperatorName("&&")), \
                                            eachOf(hasLHS(expr().bind("mcexpr126rl")), hasRHS(expr().bind("mcexpr126rl"))))), &HandlerForExpr126);

    Matcher.addMatcher(binaryOperator(allOf(eachOf(hasOperatorName("<<"), hasOperatorName(">>"), hasOperatorName("~"), hasOperatorName("<<="), \
                                            hasOperatorName(">>="), hasOperatorName("&"), hasOperatorName("&="), hasOperatorName("^"), hasOperatorName("^=")\
                                            , hasOperatorName("|"), hasOperatorName("|=")), eachOf(hasLHS(expr().bind("mcexpr127rl")), hasRHS(expr().bind("mcexpr127rl"))))), &HandlerForExpr127);
    Matcher.addMatcher(binaryOperator(allOf(eachOf(hasOperatorName(">>"), hasOperatorName(">>="), hasOperatorName("<<="), hasOperatorName("<<")), \
                                            hasLHS(expr().bind("mcexpr128lhs")) , hasRHS(expr().bind("mcexpr128rhs")))), &HandlerForExpr128);
    Matcher.addMatcher(unaryOperator(allOf(hasOperatorName("-"), hasUnaryOperand(expr().bind("mcexpr129")))), &HandlerForExpr129);

    Matcher.addMatcher(binaryOperator(allOf(hasOperatorName(","), hasLHS(expr().bind("mcexpr1210")))), &HandlerForExpr1210);

    Matcher.addMatcher(unaryOperator(allOf(eachOf(hasOperatorName("++"), hasOperatorName("--")), anyOf(hasAncestor(binaryOperator()), hasDescendant(binaryOperator())))).bind("mcexpr1213"), &HandlerForExpr1213);

    Matcher.addMatcher(binaryOperator(allOf(hasOperatorName("="), eachOf(hasLHS(expr().bind("cse131rlhs")), hasRHS(expr().bind("cse131rlhs"))))), &HandlerForCSE131);

    Matcher.addMatcher(ifStmt(hasCondition(expr(unless(hasDescendant(binaryOperator(anyOf(hasOperatorName("<")\
                                           , hasOperatorName(">"), hasOperatorName("=="), hasOperatorName("<="), hasOperatorName(">=")))))).bind("mccse132"))), &HandlerForCSE132);
    Matcher.addMatcher(binaryOperator(allOf(anyOf(hasOperatorName("<"), hasOperatorName(">"), hasOperatorName("<="), hasOperatorName(">="), hasOperatorName("==")), \
                                            eachOf(hasLHS(expr().bind("mccse1332rl")), hasRHS(expr().bind("mccse1332rl"))))).bind("mccse1332daddy"), &HandlerForCSE1332);
    Matcher.addMatcher(forStmt().bind("mccse134"), &HandlerForCSE134);

    Matcher.addMatcher(forStmt(forEachDescendant(stmt(eachOf(unaryOperator(allOf(anyOf(hasOperatorName("++"), hasOperatorName("--")), hasUnaryOperand(declRefExpr().bind("mccse136kiddo")))), \
                               binaryOperator(allOf(hasOperatorName("="), hasLHS(declRefExpr().bind("mccse136kiddo")))))))).bind("mccse136daddy"), &HandlerForCSE136);

    Matcher.addMatcher(gotoStmt().bind("mccf144"), &HandlerForCF144);

    Matcher.addMatcher(continueStmt().bind("mccf145"), &HandlerForCF145);

    Matcher.addMatcher(breakStmt(hasAncestor(stmt(anyOf(forStmt().bind("mccffofo"), doStmt().bind("mccfdodo"), whileStmt().bind("mccfwuwu"))))), &HandlerForCF146);

    Matcher.addMatcher(returnStmt(hasAncestor(functionDecl().bind("mccf147"))), &HandlerForCF147);

    Matcher.addMatcher(forStmt(unless(has(compoundStmt()))).bind("mccf148for"), &HandlerForCF148);

    Matcher.addMatcher(whileStmt(unless(has(compoundStmt()))).bind("mccf148while"), &HandlerForCF148);

    Matcher.addMatcher(doStmt(unless(has(compoundStmt()))).bind("mccf148do"), &HandlerForCF148);

    Matcher.addMatcher(switchStmt(unless(has(compoundStmt()))).bind("mccf148switch"), &HandlerForCF148);

    Matcher.addMatcher(switchStmt(hasCondition(expr().bind("mcswitch154"))).bind("mcswitch154daddy"), &HandlerForSwitch154);

    Matcher.addMatcher(implicitCastExpr().bind("mcptc111"), &HandlerForPTC111);

    Matcher.addMatcher(expr().bind("mccse137"), &HandlerForCSE137);

    Matcher.addMatcher(callExpr(hasAncestor(functionDecl().bind("mcdcdf810daddy"))).bind("mcdcdf810"), &HandlerForDCDF810);

    Matcher.addMatcher(functionDecl(allOf(returns(anything()), unless(returns(asString("void"))), hasBody(compoundStmt()) \
                                          , unless(hasDescendant(returnStmt())))).bind("mcfunction165"), &HandlerForFunction165);

    Matcher.addMatcher(functionDecl(allOf(parameterCountIs(0), hasBody(compoundStmt()))).bind("mcfunction1652"), &HandlerForFunction1652);

    Matcher.addMatcher(declRefExpr(allOf(to(varDecl().bind("loco")), unless(hasParent(castExpr(hasCastKind(clang::CK_ArrayToPointerDecay)))), hasAncestor(stmt(eachOf(binaryOperator(hasOperatorName("+")).bind("bino"), \
                                         binaryOperator(hasOperatorName("-")).bind("bino"), unaryOperator(hasOperatorName("++")).bind("uno"), \
                                         unaryOperator(hasOperatorName("--")).bind("uno"), binaryOperator(hasOperatorName("/")).bind("bino"), \
                                         binaryOperator(hasOperatorName("*")).bind("bino"), binaryOperator(hasOperatorName("<")).bind("bino"), \
                                         binaryOperator(hasOperatorName("<=")).bind("bino"), binaryOperator(hasOperatorName(">")).bind("bino"), \
                                         binaryOperator(hasOperatorName(">=")).bind("bino")))))).bind("mcpointer171"), &HandlerForPointer171);

    /*start of 17.3 matchers*/
    Matcher.addMatcher(binaryOperator(allOf(hasOperatorName("<="), hasRHS(expr(anyOf(hasDescendant(declRefExpr(hasType(pointerType())).bind("mcpointer1723rhs")), \
                                            has(declRefExpr(hasType(pointerType())).bind("mcpointer1723rhs"))))), \
                                            hasLHS(expr(anyOf(hasDescendant(declRefExpr(hasType(pointerType())).bind("mcpointer1723lhs")), \
                                                has(declRefExpr(hasType(pointerType())).bind("mcpointer1723lhs"))))))).bind("mcpointer1723daddy"), &HandlerForPointer1723);

    Matcher.addMatcher(binaryOperator(allOf(hasOperatorName("<"), hasRHS(expr(anyOf(hasDescendant(declRefExpr(hasType(pointerType())).bind("mcpointer1723rhs")), \
                                            has(declRefExpr(hasType(pointerType())).bind("mcpointer1723rhs"))))), \
                                            hasLHS(expr(anyOf(hasDescendant(declRefExpr(hasType(pointerType())).bind("mcpointer1723lhs")), \
                                                has(declRefExpr(hasType(pointerType())).bind("mcpointer1723lhs"))))))).bind("mcpointer1723daddy"), &HandlerForPointer1723);

    Matcher.addMatcher(binaryOperator(allOf(hasOperatorName(">="), hasRHS(expr(anyOf(hasDescendant(declRefExpr(hasType(pointerType())).bind("mcpointer1723rhs")), \
                                            has(declRefExpr(hasType(pointerType())).bind("mcpointer1723rhs"))))), \
                                            hasLHS(expr(anyOf(hasDescendant(declRefExpr(hasType(pointerType())).bind("mcpointer1723lhs")), \
                                                has(declRefExpr(hasType(pointerType())).bind("mcpointer1723lhs"))))))).bind("mcpointer1723daddy"), &HandlerForPointer1723);

    Matcher.addMatcher(binaryOperator(allOf(hasOperatorName(">"), hasRHS(expr(anyOf(hasDescendant(declRefExpr(hasType(pointerType())).bind("mcpointer1723rhs")), \
                                            has(declRefExpr(hasType(pointerType())).bind("mcpointer1723rhs"))))), \
                                            hasLHS(expr(anyOf(hasDescendant(declRefExpr(hasType(pointerType())).bind("mcpointer1723lhs")), \
                                                has(declRefExpr(hasType(pointerType())).bind("mcpointer1723lhs"))))))).bind("mcpointer1723daddy"), &HandlerForPointer1723);

    Matcher.addMatcher(binaryOperator(allOf(hasOperatorName("-"), hasRHS(expr(anyOf(hasDescendant(declRefExpr(hasType(pointerType())).bind("mcpointer1723rhs")), \
                                            has(declRefExpr(hasType(pointerType())).bind("mcpointer1723rhs"))))), \
                                            hasLHS(expr(anyOf(hasDescendant(declRefExpr(hasType(pointerType())).bind("mcpointer1723lhs")), \
                                                has(declRefExpr(hasType(pointerType())).bind("mcpointer1723lhs"))))))).bind("mcpointer1723daddy"), &HandlerForPointer1723);

    Matcher.addMatcher(binaryOperator(allOf(hasOperatorName("-="), hasRHS(expr(anyOf(hasDescendant(declRefExpr(hasType(pointerType())).bind("mcpointer1723rhs")), \
                                            has(declRefExpr(hasType(pointerType())).bind("mcpointer1723rhs"))))), \
                                            hasLHS(expr(anyOf(hasDescendant(declRefExpr(hasType(pointerType())).bind("mcpointer1723lhs")), \
                                                has(declRefExpr(hasType(pointerType())).bind("mcpointer1723lhs"))))))).bind("mcpointer1723daddy"), &HandlerForPointer1723);
    /*end of 17.3 matchers*/

    /*start of 17.4 matchers*/
    Matcher.addMatcher(castExpr(allOf(hasCastKind(CK_ArrayToPointerDecay), unless(hasParent(arraySubscriptExpr())))).bind("mcpointer174"), &HandlerForPointer174);

    Matcher.addMatcher(declRefExpr(allOf(hasAncestor(expr(anyOf(binaryOperator(hasOperatorName("-=")), \
                                         unaryOperator(hasOperatorName("++")), unaryOperator(hasOperatorName("--")), \
                                         binaryOperator(hasOperatorName("+")), binaryOperator(hasOperatorName("+=")), \
                                         binaryOperator(hasOperatorName("-"))))), to(varDecl(hasType(pointerType()))))).bind("mcpointer1742"), &HandlerForPointer174);
    /*end of 17.4 matchers*/
    Matcher.addMatcher(varDecl(hasType(pointerType())).bind("mcpointer175"), &HandlerForPointer175);

    Matcher.addMatcher(fieldDecl().bind("mcpointer175field"), &HandlerForPointer175);

    Matcher.addMatcher(declRefExpr(allOf(to(varDecl().bind("mctypes6origin")), \
                                         hasAncestor(binaryOperator(allOf(hasRHS(expr().bind("mctypes6rhs")), \
                                             hasOperatorName("="))).bind("mctypes6dous")), hasType(isAnyCharacter()))), &HandlerForTypes61);

    Matcher.addMatcher(varDecl(hasType(incompleteArrayType())).bind("mcsu181arr"), &HandlerForSU181);

    Matcher.addMatcher(recordDecl(isStruct()).bind("mcsu181struct"), &HandlerForSU184);

    Matcher.addMatcher(cStyleCastExpr().bind("mcptc11cstyle"), &HandlerForMCPTCCSTYLE);

    Matcher.addMatcher(implicitCastExpr(has(expr(anyOf(binaryOperator().bind("atcdous"), unaryOperator().bind("atcuno"), \
                                            parenExpr().bind("atcparens"), implicitCastExpr().bind("atckidice"), \
                                            cStyleCastExpr().bind("atccstyle"))))).bind("atcdaddy"), &HandlerForATC101);

    Matcher.addMatcher(namedDecl().bind("ident5var"), &HandlerForIdent5);

    Matcher.addMatcher(typedefDecl().bind("ident5typedef"), &HandlerForIdent5);

    Matcher.addMatcher(recordDecl().bind("ident5record"), &HandlerForIdent5);

    Matcher.addMatcher(declRefExpr(allOf(hasAncestor(functionDecl().bind("mcdcdf87daddy")), \
                                         to(varDecl(unless(hasAncestor(functionDecl()))).bind("mcdcdf87origin")))).bind("mcdcdfobj"), &HandlerForDCDF87);

#if 0
    Matcher.addMatcher(functionDecl(hasExternalFormalLinkage()).bind("mcdcdf88function"), &HandlerForDCDF88);

    Matcher.addMatcher(varDecl(hasExternalFormalLinkage()).bind("mcdcdf88var"), &HandlerForDCDF88);
#endif

    Matcher.addMatcher(expr().bind("mclangx23"), &HandlerForLangX23);

    Matcher.addMatcher(parmVarDecl(unless(allOf(hasAncestor(functionDecl(hasDescendant(binaryOperator(allOf(hasOperatorName("="), \
                                          hasLHS(hasDescendant(declRefExpr(allOf(hasAncestor(unaryOperator(hasOperatorName("*"))), \
                                              to(parmVarDecl(hasType(pointerType())).bind("zulu"))))))))))), equalsBoundNode("zulu")))).bind("mcfunction167"), &HandlerForFunction167);

    Matcher.addMatcher(nullStmt().bind("mccf143nullstmt"), &HandlerForCF143);

    Matcher.addMatcher(recordDecl(allOf(has(fieldDecl(hasType(realFloatingPointType()))), isUnion())).bind("mcexpr1212"), &HandlerForExpr1212);

    Matcher.addMatcher(expr(hasDescendant(expr(anyOf(unaryOperator(hasOperatorName("--"), hasOperatorName("++")).bind("mcexpr1211uno"), \
                                          binaryOperator(anyOf(hasOperatorName("*"), hasOperatorName("/"), \
                                              hasOperatorName("-"), hasOperatorName("+"))).bind("mcexpr1211dous"))))).bind("mcexpr1211"), &HandlerForExpr1211);

    Matcher.addMatcher(binaryOperator(allOf(hasLHS(expr(hasType(isInteger())).bind("mcatc105lhs")), hasOperatorName("<<"))).bind("mcatc105"), &HandlerForAtc105);

    Matcher.addMatcher(unaryOperator(allOf(hasOperatorName("~") , hasUnaryOperand(expr(hasType(isInteger())).bind("mcatc105lhs")))).bind("mcatc105uno"), &HandlerForAtc105);

    Matcher.addMatcher(forStmt().bind("mccse135"), &HandlerForCSE135);

    Matcher.addMatcher(binaryOperator(allOf(hasRHS(expr(has(expr(anyOf(integerLiteral().bind("mc612intlit"), \
                                            characterLiteral().bind("mc612charlit")))))), hasLHS(expr(hasType(isAnyCharacter())).bind("mc612exp")), \
                                            hasOperatorName("="))), &HandlerForTypes612);

    /*@DEVI-start of 7.1 matchers.*/
    Matcher.addMatcher(stringLiteral().bind("mcconst71string"), &HandlerForConst71);

    Matcher.addMatcher(characterLiteral().bind("mcconst71char"), &HandlerForConst71);

    Matcher.addMatcher(integerLiteral().bind("mcconst71int"), &HandlerForConst71);
    /*end of 7.1*/
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
  MCIdent5 HandlerForIdent5;
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
  MatchFinder Matcher;
};
/**********************************************************************************************************************/
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
    CI.getPreprocessor().addPPCallbacks(llvm::make_unique<PPInclusion>(&CI.getSourceManager()));

    DiagnosticsEngine &DiagEngine = CI.getPreprocessor().getDiagnostics();

    const DiagnosticConsumer* DiagConsumer = DiagEngine.getClient();

#if 0
    const IdentifierTable &IT [[maybe_unused]] = CI.getPreprocessor().getIdentifierTable();
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

  const std::vector<std::string> &SourcePathList = op.getSourcePathList();

  ClangTool Tool(op.getCompilations(), op.getSourcePathList());

  XMLDocOut.XMLCreateReport();

  JSONDocOUT.JSONCreateReport();

  IsThereJunkPreInclusion ITJPIInstance;

  ITJPIInstance.Check(SourcePathList);

  int RunResult = Tool.run(newFrontendActionFactory<MyFrontendAction>().get());

  CheckForNullStatements CheckForNull;

  CheckForNull.Check();

  onEndOfAllTUs::run();

  XMLDocOut.SaveReport();

  JSONDocOUT.CloseReport();

  return RunResult;
}
/*last line intentionally left blank.*/
