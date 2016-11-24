
/*first line intentionally left blank.*/
/**********************************************************************************************************************/
/*included modules*/
/*project headers*/
#include "mutator_aux.h"
/*standard headers*/
#include <string>
#include <iostream>
#include <cassert>
#include <vector>
/*LLVM headers*/
#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Lex/Lexer.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Rewrite/Core/Rewriter.h"
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

static llvm::cl::OptionCategory MatcherSampleCategory("Matcher Sample");
/**********************************************************************************************************************/
class MCForCmpless : public MatchFinder::MatchCallback {
public:
  MCForCmpless (Rewriter &Rewrite) : Rewrite (Rewrite) {}

  virtual void run(const MatchFinder::MatchResult &MR)
  {
    if (MR.Nodes.getNodeAs<clang::ForStmt>("mcfor") != nullptr)
    {
      const ForStmt *FS = MR.Nodes.getNodeAs<clang::ForStmt>("mcfor");

      SourceLocation SL = FS->getLocStart();
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

      std::cout << "14.8 : " << "\"For\" statement has no braces {}: " << std::endl;
      std::cout << SL.printToString(*MR.SourceManager) << "\n" << std::endl;
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
class MCWhileCmpless : public MatchFinder::MatchCallback {
public:
  MCWhileCmpless (Rewriter &Rewrite) : Rewrite (Rewrite) {}

  virtual void run(const MatchFinder::MatchResult &MR)
  {
    if (MR.Nodes.getNodeAs<clang::WhileStmt>("mcwhile") != nullptr)
    {
      const WhileStmt *WS = MR.Nodes.getNodeAs<clang::WhileStmt>("mcwhile");

      SourceLocation SL = WS->getLocStart();
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

      std::cout << "14.8 : " << "\"While\" statement has no braces {}: " << std::endl;
      std::cout << SL.printToString(*MR.SourceManager) << "\n" << std::endl;
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

      std::cout << "14.9 : " << "\"Else\" statement has no braces {}: " << std::endl;
      std::cout << SL.printToString(*MR.SourceManager) << "\n" << std::endl;
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

      std::cout << "14.9 : " << "\"If\" statement has no braces {}: " << std::endl;
      std::cout << SL.printToString(*MR.SourceManager) << "\n" << std::endl;
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

      std::cout << "14.10 : " << "\"If-Else If\" statement has no ending Else: " << std::endl;
      std::cout << IFESL.printToString(*MR.SourceManager) << "\n" << std::endl;
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

      std::cout << "15.2 : " << "\"SwitchStmt\" has a caseStmt that's missing a breakStmt: " << std::endl;
      std::cout << SL.printToString(*MR.SourceManager) << "\n" << std::endl;
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

      std::cout << "15.3 : " << "\"SwitchStmt\" does not have a defaultStmt: " << std::endl;
      std::cout << SL.printToString(*MR.SourceManager) << "\n" << std::endl;
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

      ASTContext *const ASTC = MR.Context;

      ASTContext::DynTypedNodeList NodeList = ASTC->getParents(*CS);

      /*assumptions:nothing has more than one parent in C.*/
      ast_type_traits::DynTypedNode ParentNode = NodeList[0];

      ast_type_traits::ASTNodeKind ParentNodeKind = ParentNode.getNodeKind();

      std::string StringKind = ParentNodeKind.asStringRef().str();

      if (StringKind != "SwitchStmt")
      {
        std::cout << "15.1 : " << "\"CaseStmt\" has a CompoundStmt ancestor that is not the child of the SwitchStmt: " << std::endl;
        std::cout << SL.printToString(*MR.SourceManager) << "\n" << std::endl;
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

      std::cout << "15.5 : " << "\"SwitchStmt\" does not have a CaseStmt: " << std::endl;
      std::cout << SL.printToString(*MR.SourceManager) << "\n" << std::endl;
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

        std::cout << "16.1 : " << "\"FunctionDecl\" is variadic: " << std::endl;
        std::cout << SL.printToString(*MR.SourceManager) << "\n" << std::endl;
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

      std::string FuncNameStr = FD->getNameInfo().getAsString();

      if (CE->getDirectCallee())
      {
        const FunctionDecl *FDCalled = CE->getDirectCallee();
        std::string CalledFuncNameStr = FDCalled->getNameInfo().getAsString();

        if (FuncNameStr == CalledFuncNameStr)
        {
          std::cout << "16.2 : " << "\"FunctionDecl\" is recursive: " << std::endl;
          std::cout << SL.printToString(*MR.SourceManager) << "\n" << std::endl;
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
                std::cout << "16.4 : " << "FunctionDecl parameter names are not the same as function definition parameter names: " << std::endl;
                std::cout << SL.printToString(*MR.SourceManager) << "  &  " << SLDcl.printToString(*MR.SourceManager) << "\n" << std::endl;

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

      if (CE->getNumArgs() != FD->getNumParams())
      {
        std::cout << "16.6 : " << "CallExpr number of arguments does not equal the number of parameters in the declaration: " << std::endl;
        std::cout << SL.printToString(*MR.SourceManager) << "\n" << std::endl;
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
class MCFunction168 : public MatchFinder::MatchCallback
{
public:
  MCFunction168 (Rewriter &Rewrite) : Rewrite (Rewrite) {}

  virtual void run(const MatchFinder::MatchResult &MR)
  {
    if (MR.Nodes.getNodeAs<clang::ReturnStmt>("mcfunc168") != nullptr)
    {
      const ReturnStmt *RT = MR.Nodes.getNodeAs<clang::ReturnStmt>("mcfunc168");

      const Expr *RE = RT->getRetValue();

      SourceLocation SL = RT->getLocStart();
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

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

      CastKind CK = ICE->getCastKind();

      if (CK == CK_FunctionToPointerDecay)
      {
        std::cout << "16.9 : " << "FunctionToPointerDecay: " << std::endl;
        std::cout << SL.printToString(*MR.SourceManager) << "\n" << std::endl;
      }
    }
  }

private:
  Rewriter &Rewrite;
};
/**********************************************************************************************************************/
/*@DEVI-what is correct: match a pointer then run matcher for implicitcastexpressions of type arraytopointerdecay
that have unary(--,++) and binary(-,+) operators as parents*/
class MCPA171 : public MatchFinder::MatchCallback
{
public:
  MCPA171 (Rewriter &Rewrite) : Rewrite(Rewrite) {}

  virtual void run(const MatchFinder::MatchResult &MR)
  {
    if (MR.Nodes.getNodeAs<clang::VarDecl>("mcpa171") != nullptr)
    {
      const VarDecl *VD = MR.Nodes.getNodeAs<clang::VarDecl>("mcpa171");

      QualType QT = VD->getType();

#if 0
      std::cout << QT.getAsString() << "\n" << std::endl;
#endif
    }
  }

private:
  Rewriter &Rewrite;
};
/**********************************************************************************************************************/
class MCSU184 : public MatchFinder::MatchCallback
{
public:
  MCSU184 (Rewriter &Rewrite) : Rewrite(Rewrite) {}

  virtual void run(const MatchFinder::MatchResult &MR)
  {
    if (MR.Nodes.getNodeAs<clang::RecordDecl>("mcsu184") != nullptr)
    {
      const RecordDecl *RD = MR.Nodes.getNodeAs<clang::RecordDecl>("mcsu184");

      SourceLocation SL = RD->getLocStart();
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

      std::cout << "18.4 : " << "Union declared: " << std::endl;
      std::cout << SL.printToString(*MR.SourceManager) << "\n" << std::endl;
    }
  }

private:
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
        /*this part is ueless since the clang parser wont let such a bitfield through.*/
        std::cout << "6.4 : " << "BitField has a type other than int or unsigned int: " << std::endl;
        std::cout << SL.printToString(*MR.SourceManager) << "\n" << std::endl;
      }

      ASTContext *const ASTC = MR.Context;
      unsigned int BitWidth = FD->getBitWidthValue(*ASTC);

      if (TP->hasSignedIntegerRepresentation())
      {
        if (BitWidth < 2U)
        {
          std::cout << "6.5 : " << "BitField of type signed integer has a length of less than 2 in bits : " << std::endl;
          std::cout << SL.printToString(*MR.SourceManager) << "\n" << std::endl;
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
    FuncInfoProto.push_back(FuncInfo());

    VecC = 0U;
  };

  /*underdev*/
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
        std::cout << "8.1 : " << "Function does not have a FunctionDecl that is not a definition : " << std::endl;
        std::cout << FuncInfoProto[x].StrcSL << "\n" << std::endl;
      }
    }

  }

private:
  struct FuncInfo {
    std::string FuncNameString;
    std::string StrcSL;
    bool HasDecThatisNotDef = false;
  };

  std::vector<FuncInfo> FuncInfoProto;

  unsigned int VecC;

  bool alreadymatched;

  Rewriter &Rewrite;
};
/**********************************************************************************************************************/
/*Notes:clang does not let 8.2 and 8.3 through.*/
/*clang gives the implicitly-typed vardecl and functiondecl a default type in the AST so we cant use that.
we should just get the rewritten text and do string searches inside. thats the only way i can think of.*/
class MCDCDF82 : public MatchFinder::MatchCallback
{
public:
  MCDCDF82 (Rewriter &Rewrite) : Rewrite (Rewrite) {}

  virtual void run(const MatchFinder::MatchResult &MR)
  {
    if (MR.Nodes.getNodeAs<clang::VarDecl>("mcdcdf82") != nullptr)
    {
      const VarDecl* VD = MR.Nodes.getNodeAs<clang::VarDecl>("mcdcdf82");

      std::string QualifiedName = VD->getQualifiedNameAsString();

      QualType QT = VD->getType();

#if 0
      std::cout << QualifiedName << "\n" << std::endl;
#endif
    }
  }

private:
  Rewriter &Rewrite;
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
      SL = Devi::SourceLocationHasMacro(SL, Rewrite, "start");

      /*we only check for local static since global static is meaningless.*/
      if (!VD->isStaticLocal() && VD->isLocalVarDecl())
      {
        if (!VD->hasInit())
        {
          std::cout << "9.1 : " << "staic local variable does not have initialization : " << std::endl;
          std::cout << SL.printToString(*MR.SourceManager) << "\n" << std::endl;
        }
      }
    }

  }

private:
  Rewriter &Rewrite;
};
/**********************************************************************************************************************/
class MCInit92 : public MatchFinder::MatchCallback
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

      unsigned int NumInits = ILE->getNumInits();

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
      const EnumConstantDecl * ECD = MR.Nodes.getNodeAs<clang::EnumConstantDecl>("mcinit93");
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
            /*in breach of misrac*/
            std::cout << "9.3 : " << "first enumeration has integerliteral initialization but not all enumerations do : " << std::endl;
            std::cout << SL.printToString(*MR.SourceManager) << "\n" << std::endl;
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
            /*in breach of misrac*/
            std::cout << "9.3 : " << "first enumeration does not have integerliteral initialization but at least one other enumeration does : " << std::endl;
            std::cout << SL.printToString(*MR.SourceManager) << "\n" << std::endl;
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

  bool isFirstElement;
  bool doesFirstElementHaveInit;
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

      ASTContext *const ASTC = MR.Context;

      if (EXP->HasSideEffects(*ASTC, true))
      {
        std::cout << "12.3 : " << "sizeof working on an expr with a side-effect : " << std::endl;
        std::cout << SL.printToString(*MR.SourceManager) << "\n" << std::endl;
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

      ASTContext *const ASTC = MR.Context;

      if (EXP->HasSideEffects(*ASTC, true))
      {
        std::cout << "12.4 : " << "Righ-hand expr has side-effect : " << std::endl;
        std::cout << SL.printToString(*MR.SourceManager) << "\n" << std::endl;
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

      ASTContext *const ASTC = MR.Context;

      QualType QT = EXP->getType();

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
        std::cout << "12.5 : " << "RHS and/or LHS operands are not primary expressions : " << std::endl;
        std::cout << SL.printToString(*MR.SourceManager) << "\n" << std::endl;
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

      if (!EXP->isKnownToHaveBooleanValue())
      {
        std::cout << "12.6 : " << "RHS and/or LHS operands are not effectively-boolean values : " << std::endl;
        std::cout << SL.printToString(*MR.SourceManager) << "\n" << std::endl;
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

      QualType QT = EXP->getType();

      const clang::Type* TP = QT.getTypePtr();

      if (TP->hasSignedIntegerRepresentation() && TP->isIntegerType())
      {
        std::cout << "12.7 : " << "Bitwise operator has signed RHS and/or LHS operands: " << std::endl;
        std::cout << SL.printToString(*MR.SourceManager) << "\n" << std::endl;
      }
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
class MyASTConsumer : public ASTConsumer {

public:
  MyASTConsumer(Rewriter &R) : HandlerForCmpless(R), HandlerWhileCmpless(R), HandlerElseCmpless(R), HandlerIfCmpless(R), \
    HandlerForIfElse(R), HandlerForSwitchBrkLess(R), HandlerForSwitchDftLEss(R), HandlerForMCSwitch151(R), HandlerForMCSwitch155(R), \
    HandlerForMCFunction161(R), HandlerForFunction162(R), HandlerForFunction164(R), HandlerForFunction166(R), HandlerForFunction168(R), \
    HandlerForFunction169(R), HandlerForPA171(R), HandlerForSU184(R), HandlerForType6465(R), HandlerForDCDF81(R), HandlerForDCDF82(R), \
    HandlerForInit91(R), HandlerForInit92(R), HandlerForInit93(R), HandlerForExpr123(R), HandlerForExpr124(R), HandlerForExpr125(R), \
    HandlerForExpr126(R), HandlerForExpr127(R) {

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
  MatchFinder Matcher;
};
/**********************************************************************************************************************/
class MyFrontendAction : public ASTFrontendAction {
public:
  MyFrontendAction() {}
  void EndSourceFileAction() override {
#if 0
    TheRewriter.getEditBuffer(TheRewriter.getSourceMgr().getMainFileID()).write(llvm::outs());
#endif
  }

  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, StringRef file) override {
    TheRewriter.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
    return llvm::make_unique<MyASTConsumer>(TheRewriter);
  }

private:
  Rewriter TheRewriter;
};
/**********************************************************************************************************************/
/*Main*/
int main(int argc, const char **argv) {
  CommonOptionsParser op(argc, argv, MatcherSampleCategory);
  ClangTool Tool(op.getCompilations(), op.getSourcePathList());

  return Tool.run(newFrontendActionFactory<MyFrontendAction>().get());
}
/*last line intentionally left blank.*/
