
/***************************************************Project Mutator****************************************************/
//-*-c++-*-
/*first line intentionally left blank.*/
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
/*FIXME-all classes should use replacements.*/
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
#include "clang/Basic/LLVM.h"
#include "clang/CodeGen/CodeGenAction.h"
#include "clang/CodeGen/BackendUtil.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Lex/Lexer.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Linker/Linker.h"
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
    //TheRewriter.getEditBuffer(TheRewriter.getSourceMgr().getMainFileID()).write(llvm::outs());
  }

  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, StringRef file) override {
    TheRewriter.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
    return llvm::make_unique<MyASTConsumer>(TheRewriter);
  }

private:
  Rewriter TheRewriter;
};
/**********************************************************************************************************************/
#if 0
class mutatorBEConsumer : public ASTConsumer {
  public:
    using LinkModule = CodeGenAction::LinkModule;
    mutatorBEConsumer(clang::BackendAction Backend_EmitObj, DiagnosticsEngine &diags, const HeaderSearchOptions &HSO, 
        const PreprocessorOptions &PPO, const CodeGenOptions &CGO, const clang::TargetOptions &TO, 
        const LangOptions &LO, bool TimePasses, const std::string &InFile, llvm::SmallVector<LinkModule, 4> LinkModules, 
        std::unique_ptr<raw_pwrite_stream> OS, LLVMContext &C) {}

    virtual void HandleTranslationUnit(ASTContext &astc) {}
};
#endif
/**********************************************************************************************************************/
class mutatorEmitObjAction : public EmitObjAction {
  public:
    mutatorEmitObjAction() {}
};
/**********************************************************************************************************************/
/*Main*/
int main(int argc, const char **argv) {
  CommonOptionsParser op(argc, argv, MatcherSampleCategory);
  ClangTool Tool(op.getCompilations(), op.getSourcePathList());
  std::vector<std::unique_ptr<ASTUnit>> ASTs;
  auto buildASTRes [[maybe_unused]] = Tool.buildASTs(ASTs);

  for (auto &iter : ASTs)
  {
    if (iter->hasSema())
    {
      std::cout << "sema acquired\n";
      iter->Save("./TU.save");
      clang::Sema &selfSema [[maybe_unused]] = iter->getSema();
    }
  }


  return Tool.run(newFrontendActionFactory<MyFrontendAction>().get());
}
/*last line intentionally left blank.*/

