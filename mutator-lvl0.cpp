
/*first line intentionally left blank.*/
/**********************************************************************************************************************/
/*included modules*/
/*project headers*/
#include "mutator_aux.h"
/*standard headers*/
#include <string>
#include <iostream>
#include <cassert>
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
/**********************************************************************************************************************/
/**********************************************************************************************************************/
class MyASTConsumer : public ASTConsumer {

public:
  MyASTConsumer(Rewriter &R) : HandlerForCmpless(R), HandlerWhileCmpless(R), HandlerElseCmpless(R), HandlerIfCmpless(R), \
    HandlerForIfElse(R), HandlerForSwitchBrkLess(R), HandlerForSwitchDftLEss(R), HandlerForMCSwitch151(R), HandlerForMCSwitch155(R), \
    HandlerForMCFunction161(R), HandlerForFunction162(R), HandlerForFunction164(R), HandlerForFunction166(R), HandlerForFunction168(R) {

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
