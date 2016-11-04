
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
class IfElseFixer : public MatchFinder::MatchCallback
{
public:
  IfElseFixer (Rewriter &Rewrite) : Rewrite (Rewrite) {}

  virtual void run(const MatchFinder::MatchResult &MR)
  {
    /*underdev*/
    if (MR.Nodes.getNodeAs<clang::IfStmt>("mrifelse") != nullptr)
    {
      const IfStmt *ElseIf = MR.Nodes.getNodeAs<clang::IfStmt>("mrifelse");
      //const IfStmt *LastIf = MR.Nodes.getNodeAs<clang::IfStmt>("mrifelse");

      SourceLocation IFESL = ElseIf->getLocStart();
      IFESL = Devi::SourceLocationHasMacro(IFESL, Rewrite, "start");
      SourceLocation IFESLE = ElseIf->getLocEnd();
      IFESLE = Devi::SourceLocationHasMacro(IFESLE, Rewrite, "end");
      SourceRange SR;
      SR.setBegin(IFESL);
      SR.setEnd(IFESLE);

      clang::Rewriter::RewriteOptions opts;

      int RangeSize = Rewrite.getRangeSize(SR, opts);

      //std::cout << IFESLE.printToString(*MR.SourceManager) << "\n" << std::endl;

#if 1
      //Rewrite.InsertText(ElseIf->getThen()->getLocStart(), "{\n", "true", "true");
      Rewrite.InsertTextAfterToken(IFESL.getLocWithOffset(RangeSize + 1U), "else\n{/*intentionally left blank*/\n}\n");
#endif
    }
    else
    {
      std::cout << "matcher -mrifelse- returned nullptr." << std::endl;
    }
  }


private:
  Rewriter &Rewrite;
};
/**********************************************************************************************************************/
/**********************************************************************************************************************/
class MyASTConsumer : public ASTConsumer {

public:
  MyASTConsumer(Rewriter &R) : HandlerForIfElse(R) {
    Matcher.addMatcher(ifStmt(allOf(hasElse(ifStmt()), unless(hasAncestor(ifStmt())), unless(hasDescendant(ifStmt(hasElse(unless(ifStmt()))))))).bind("mrifelse"), &HandlerForIfElse);
  }

  void HandleTranslationUnit(ASTContext &Context) override {
    Matcher.matchAST(Context);
  }

private:
  IfElseFixer HandlerForIfElse;
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
/*Main*/
int main(int argc, const char **argv) {
  CommonOptionsParser op(argc, argv, MatcherSampleCategory);
  ClangTool Tool(op.getCompilations(), op.getSourcePathList());

  return Tool.run(newFrontendActionFactory<MyFrontendAction>().get());
}
/*last line intentionally left blank.*/
