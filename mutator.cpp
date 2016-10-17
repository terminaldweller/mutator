
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
      /*underdev*/
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
      Rewrite.ReplaceText(BinOpSL, 2U , "XXX");
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
/**********************************************************************************************************************/
class MyASTConsumer : public ASTConsumer {

public:
  MyASTConsumer(Rewriter &R) : HandlerForFunction(R) {
    Matcher.addMatcher(binaryOperator(hasOperatorName("==")).bind("binopeq"), &HandlerForFunction);
  }

  void HandleTranslationUnit(ASTContext &Context) override {
    Matcher.matchAST(Context);
  }

private:
  FunctionHandler HandlerForFunction;
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
