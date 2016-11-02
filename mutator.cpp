
/*first line intentionally left blank.*/
/**********************************************************************************************************************/
/*included modules*/
/*standard library*/
#include <string>
#include <iostream>
#include <cassert>
/*LLVM-libs*/
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
/*the variable that holds the previously-matched SOC for class StmtTrap.*/
std::string g_linenoold;

static llvm::cl::OptionCategory MatcherSampleCategory("Matcher Sample");
/**********************************************************************************************************************/
/*matcher callback for something.*/
class FunctionHandler : public MatchFinder::MatchCallback {
public:
  FunctionHandler (Rewriter &Rewrite) : Rewrite(Rewrite) {}

  virtual void run(const MatchFinder::MatchResult &MR)
  {
    if (MR.Nodes.getNodeAs<clang::BinaryOperator>("binopeq") != nullptr)
    {
      /*get the matched node.*/
      const BinaryOperator *BinOp = MR.Nodes.getNodeAs<clang::BinaryOperator>("binopeq");

      /*get the sourceloation.*/
      SourceLocation BinOpSL = BinOp->getLocStart();

      /*does the sourcelocation include a macro expansion?*/
      if ( BinOpSL.isMacroID() )
      {
        /*get the expansion range which is startloc and endloc*/
        std::pair <SourceLocation, SourceLocation> expansionRange = Rewrite.getSourceMgr().getImmediateExpansionRange(BinOpSL);

        /*get the startloc.*/
        BinOpSL = expansionRange.first;
      }

      /*replace it.*/
#if 0
      Rewrite.ReplaceText(BinOpSL, 2U , "XXX");
#endif
    }
    else
    {
      std::cout << "the macther -binopeq- returned nullptr!" << std::endl;
    }
  }

private:
  Rewriter &Rewrite;
};
/**********************************************************************************************************************/
class StmtTrapIf : public MatchFinder::MatchCallback {
public:
  StmtTrapIf (Rewriter &Rewrite) : Rewrite(Rewrite) {}

  virtual void run (const MatchFinder::MatchResult &MR)
  {
    /*just in case*/
    if (MR.Nodes.getNodeAs<clang::IfStmt>("iftrap") != nullptr)
    {
      /*getting the matched ifStmt.*/
      const IfStmt *TrapIf = MR.Nodes.getNodeAs<clang::IfStmt>("iftrap");

      /*getting the condition of the matched ifStmt.*/
      const Expr *IfCond = TrapIf->getCond();

      /*getting the sourcerange of the condition of the trapped ifstmt.*/
      SourceLocation TrapIfCondSL = IfCond->getLocStart();
      SourceLocation TrapIfCondSLEnd = IfCond->getLocEnd();
      SourceRange TrapIfCondSR;
      TrapIfCondSR.setBegin(TrapIfCondSL);
      TrapIfCondSR.setEnd(TrapIfCondSLEnd);

      /*replacing the condition with the utility trap function.*/
      Rewrite.ReplaceText(TrapIfCondSR, "C_Trap_P()");
    }
    else
    {
      std::cout << "the matcher -iftrap- returned nullptr!" << std::endl;
    }
  }

private:
  Rewriter &Rewrite;
};
/**********************************************************************************************************************/
class StmtTrap : public MatchFinder::MatchCallback {
public:
  StmtTrap (Rewriter &Rewrite) : Rewrite(Rewrite) {}

  /*if there are more than 2 statements in the traditional sense in one line, the behavior is undefined.*/
  /*for now this class only finds an SOC. later to be used for something useful.*/
  virtual void run (const MatchFinder::MatchResult &MR)
  {
    /*out of paranoia*/
    if (MR.Nodes.getNodeAs<clang::Stmt>("stmttrap") != nullptr)
    {
      const Stmt *StmtTrapMR = MR.Nodes.getNodeAs<clang::Stmt>("stmttrap");

      SourceLocation STSL = StmtTrapMR->getLocStart();
      SourceLocation STSLE = StmtTrapMR->getLocEnd();

      /*just getting the SOC of the matched statement out of its sourcelocation.*/
      /*this only works since we are guaranteed the same and known matching pattern by MatchFinder.*/
      size_t startloc = 0U;
      size_t endloc = 0U;
      std::string lineno;
      startloc = STSL.printToString(*MR.SourceManager).find(":", 0U);
      endloc = STSLE.printToString(*MR.SourceManager).find(":", startloc + 1U);
      lineno = STSL.printToString(*MR.SourceManager).substr(startloc, endloc - startloc);

      /*just prints out the sourcelocations for diagnostics.*/
#if 0
      std::cout << STSL.printToString(*MR.SourceManager) << std::endl;
      std::cout << STSLE.printToString(*MR.SourceManager) << std::endl;
      std::cout << startloc << "---" << endloc << "---" << lineno << std::endl;
#endif

      /*have we matched a new SOC? if yes:*/
      if (lineno != g_linenoold)
      {
        SourceRange SR;
        SR.setBegin(StmtTrapMR->getLocStart());
        SR.setEnd(StmtTrapMR->getLocEnd());

        //Rewrite.InsertText(StmtTrapMR->getLocStart(), "XXX", "true", "true");
      }
      else
      {
        /*intentionally left blank.*/
      }

      /*set the string representing the old SOC line number to the one matched now.*/
      g_linenoold = lineno;
    }
    else
    {
      std::cout << "the matcher -stmttrap- returned nullptr!" << std::endl;
    }
  }

private:
  Rewriter &Rewrite;
};
/**********************************************************************************************************************/
/**********************************************************************************************************************/
class MyASTConsumer : public ASTConsumer {

public:
  MyASTConsumer(Rewriter &R) : HandlerForFunction(R), HandlerForIfTrap(R), HandlerForStmtTrap(R) {
    Matcher.addMatcher(binaryOperator(hasOperatorName("==")).bind("binopeq"), &HandlerForFunction);

    Matcher.addMatcher(ifStmt(hasCondition(anything())).bind("iftrap"), &HandlerForIfTrap);

    Matcher.addMatcher(stmt().bind("stmttrap") , &HandlerForStmtTrap);
  }

  void HandleTranslationUnit(ASTContext &Context) override {
    Matcher.matchAST(Context);
  }

private:
  FunctionHandler HandlerForFunction;
  StmtTrapIf HandlerForIfTrap;
  StmtTrap HandlerForStmtTrap;
  MatchFinder Matcher;
};
/**********************************************************************************************************************/
class MyFrontendAction : public ASTFrontendAction {
public:
  MyFrontendAction() {}
  void EndSourceFileAction() override {
    TheRewriter.getEditBuffer(TheRewriter.getSourceMgr().getMainFileID()).write(llvm::outs());
  }

  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, StringRef file) override {
    TheRewriter.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
    return llvm::make_unique<MyASTConsumer>(TheRewriter);
  }

private:
  Rewriter TheRewriter;
};
/**********************************************************************************************************************/
/*MAIN*/
int main(int argc, const char **argv) {
  CommonOptionsParser op(argc, argv, MatcherSampleCategory);
  ClangTool Tool(op.getCompilations(), op.getSourcePathList());

  return Tool.run(newFrontendActionFactory<MyFrontendAction>().get());
}
/*last line intentionally left blank.*/
