
/***************************************************Project Mutator****************************************************/
//-*-c++-*-
/*first line intentionally left blank.*/
/*the source code for SaferCPP's automatic refactoring of C/C++ arrays.*/
/*Copyright (C) 2017 Noah L.,Farzad Sadeghi

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
/**********************************************************************************************************************/
/*included modules*/
/*Project Headers*/
#include "safercpp-arr.h"
#include "../mutator_aux.h"
/*Standard headers*/
#include <string>
#include <iostream>
/*Clang Headers*/
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
/*LLVM Headers*/
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
static llvm::cl::OptionCategory MatcherSampleCategory("TBD");
/**********************************************************************************************************************/
/**********************************************************************************************************************/
class MyASTConsumer : public ASTConsumer {

public:
  MyASTConsumer(Rewriter &R) 
  {
  }

  void HandleTranslationUnit(ASTContext &Context) override 
  {
    Matcher.matchAST(Context);
  }

private:
  MatchFinder Matcher;
};
/**********************************************************************************************************************/
class MyFrontendAction : public ASTFrontendAction 
{
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
int main(int argc, const char **argv) 
{
  CommonOptionsParser op(argc, argv, MatcherSampleCategory);
  ClangTool Tool(op.getCompilations(), op.getSourcePathList());

  return Tool.run(newFrontendActionFactory<MyFrontendAction>().get());
}
/*last line intentionally left blank*/

