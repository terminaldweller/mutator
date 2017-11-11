
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
#include "../mutator_aux.h"
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
class FuncDecl : public MatchFinder::MatchCallback
{
public:
  FuncDecl (Rewriter &Rewrite) : Rewrite (Rewrite) {}

  virtual void run(const MatchFinder::MatchResult &MR)
  {
    if (MR.Nodes.getNodeAs<clang::FunctionDecl>("funcdecl") != nullptr) {
      const FunctionDecl* FD = MR.Nodes.getNodeAs<clang::FunctionDecl>("funcdecl");
      std::string funcname = FD->getNameInfo().getAsString();
      std::size_t hash = std::hash<std::string>{}(funcname);
      std::string newname = "ID" + std::to_string(hash);
      std::cout << "Function name: "  << funcname << " Hash: " << hash << " New ID: " << newname << "\n";

      SourceRange SR = FD->getSourceRange();
    }
  }

private:
  Rewriter &Rewrite;
};
/**********************************************************************************************************************/
class VDecl : public MatchFinder::MatchCallback
{
public:
  VDecl (Rewriter &Rewrite) : Rewrite (Rewrite) {}

  virtual void run(const MatchFinder::MatchResult &MR)
  {
    if (MR.Nodes.getNodeAs<clang::VarDecl>("vardecl") != nullptr) {
      const VarDecl* VD = MR.Nodes.getNodeAs<clang::VarDecl>("vardecl");
      std::string varname = VD->getIdentifier()->getName().str();
      std::size_t hash = std::hash<std::string>{}(varname);
      std::string newname = "ID" + std::to_string(hash);
      std::cout << "Var name: "  << varname << " Hash: " << hash << " New ID: " << newname << "\n";

      SourceRange SR = VD->getSourceRange();
    }
  }

private:
  Rewriter &Rewrite;
};
/**********************************************************************************************************************/
class ClassDecl : public MatchFinder::MatchCallback {
  public:
    ClassDecl (Rewriter &Rewrite) : Rewrite(Rewrite) {}

    virtual void run(const MatchFinder::MatchResult &MR) {
      if (MR.Nodes.getNodeAs<clang::RecordDecl>("classdecl") != nullptr) {
        const RecordDecl* RD = MR.Nodes.getNodeAs<clang::RecordDecl>("classdecl");
      }
    }

  private:
    Rewriter &Rewrite;
};
/**********************************************************************************************************************/
class PPInclusion : public PPCallbacks
{
public:
  explicit PPInclusion (SourceManager *SM) : SM(*SM) {}

  virtual void MacroDefined(const Token &MacroNameTok, const MacroDirective *MD) {
    const MacroInfo* MI = MD->getMacroInfo();

    SourceLocation SL = MacroNameTok.getLocation(); 
    CheckSLValidity(SL);
    std::string macroname = MacroNameTok.getIdentifierInfo()->getName().str();
    std::size_t hash = std::hash<std::string>{}(macroname);
    std::string newname = "ID" + std::to_string(hash);
    std::cout << "Macro name: " << macroname << " Hash: " << hash << " New ID: " << newname << "\n";
  }

private:
  const SourceManager &SM;
};
/**********************************************************************************************************************/
class BlankDiagConsumer : public clang::DiagnosticConsumer
{
  public:
    BlankDiagConsumer() = default;
    virtual ~BlankDiagConsumer() {}
    virtual void HandleDiagnostic(DiagnosticsEngine::Level DiagLevel, const Diagnostic &Info) override {}
};
/**********************************************************************************************************************/
class MyASTConsumer : public ASTConsumer {
public:
  MyASTConsumer(Rewriter &R) : funcDeclHandler(R), HandlerForVar(R), HandlerForClass(R) {
    Matcher.addMatcher(functionDecl().bind("funcdecl"), &funcDeclHandler);
    Matcher.addMatcher(varDecl().bind("vardecl"), &HandlerForVar);
    Matcher.addMatcher(recordDecl(isClass()).bind("classdecl"), &HandlerForClass);
  }

  void HandleTranslationUnit(ASTContext &Context) override {
    Matcher.matchAST(Context);
  }

private:
  FuncDecl funcDeclHandler;
  VDecl HandlerForVar;
  ClassDecl HandlerForClass;
  MatchFinder Matcher;
};
/**********************************************************************************************************************/
class ObfFrontendAction : public ASTFrontendAction {
public:
  ObfFrontendAction() {}
  ~ObfFrontendAction() {}
  void EndSourceFileAction() override {
    //TheRewriter.getEditBuffer(TheRewriter.getSourceMgr().getMainFileID()).write(llvm::outs());
  }

  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, StringRef file) override {
    CI.getPreprocessor().addPPCallbacks(llvm::make_unique<PPInclusion>(&CI.getSourceManager()));
    DiagnosticsEngine &DE = CI.getPreprocessor().getDiagnostics();
    DE.setClient(BDCProto, false);
    TheRewriter.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
    return llvm::make_unique<MyASTConsumer>(TheRewriter);
  }

private:
  BlankDiagConsumer* BDCProto = new BlankDiagConsumer;
  Rewriter TheRewriter;
};
/**********************************************************************************************************************/
/*Main*/
int main(int argc, const char **argv) {
  CommonOptionsParser op(argc, argv, MatcherSampleCategory);
  ClangTool Tool(op.getCompilations(), op.getSourcePathList());
  return Tool.run(newFrontendActionFactory<ObfFrontendAction>().get());
}
/**********************************************************************************************************************/
/*last line intentionally left blank.*/

