
/*first line intentionally left blank.*/
/**********************************************************************************************************************/
/*included modules*/
/*standard library*/
#include <string>
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
/*adding the -- deafult option is not a good choice since we need extra flags to compile code or else the parsers not gonna parse all of the target file.*/
#if 0
#include "llvm/support/CommandLine.h"
#endif
/**********************************************************************************************************************/
/*used namespaces*/
using namespace clang;
using namespace clang::ast_matchers;
using namespace clang::driver;
using namespace clang::tooling;
/**********************************************************************************************************************/
/*global vars*/


static llvm::cl::OptionCategory MatcherSampleCategory("Matcher Sample");

/**********************************************************************************************************************/
/*matcher callback for 'if' and 'else if'.*/
class FunctionHandler : public MatchFinder::MatchCallback {
public:
  FunctionHandler (Rewriter &Rewrite) : Rewrite(Rewrite) {}

  virtual void run(const MatchFinder::MatchResult &MR)
  {
    /*dev*/
  }

private:
  Rewriter &Rewrite;
};
/**********************************************************************************************************************/

/**********************************************************************************************************************/
// Implementation of the ASTConsumer interface for reading an AST produced
// by the Clang parser. It registers a couple of matchers and runs them on
// the AST.
class MyASTConsumer : public ASTConsumer {
//friend class CalleeHandler;

public:
  MyASTConsumer(Rewriter &R) : HandlerForFunction(R) {
    Matcher.addMatcher (functionDecl(), &HandlerForFunction);
  }

  void HandleTranslationUnit(ASTContext &Context) override {
    // Run the matchers when we have the whole TU parsed.
    Matcher.matchAST(Context);
  }

private:
  FunctionHandler HandlerForFunction;
  MatchFinder Matcher;
};
/**********************************************************************************************************************/
// For each source file provided to the tool, a new FrontendAction is created.
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
