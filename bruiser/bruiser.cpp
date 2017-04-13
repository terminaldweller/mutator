
/***************************************************Project Mutator****************************************************/
//-*-c++-*-
/*first line intentionally left blank.*/
/*the source code for the mutator code breaker.*/
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
/**********************************************************************************************************************/
/*included modules*/
/*project headers*/
#include "bruiser.h"
#include "../mutator_aux.h"
/*standard headers*/
#include <string>
#include <iostream>
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
static llvm::cl::OptionCategory BruiserCategory("Empty");
/**********************************************************************************************************************/
/*the implementation of the bruiser logger.*/
bruiser::BruiserReport::BruiserReport () 
{
  BruiserLog.open("bruiser.log");
}

bruiser::BruiserReport::~BruiserReport() 
{
  BruiserLog.close();
}

bool bruiser::BruiserReport::PrintToLog(std::string __in_arg)
{
  BruiserLog << __in_arg << "\n";
  return !BruiserLog.bad();
}
/**********************************************************************************************************************/
bruiser::TypeInfo::TypeInfo(const clang::ast_type_traits::DynTypedNode* __dtn) : DTN(__dtn) {};

bruiser::TypeInfo::~TypeInfo() {};

const clang::Type* bruiser::TypeInfo::getTypeInfo(clang::ASTContext* __astc)
{
  const clang::Expr* EXP = DTN->get<clang::Expr>();

  const clang::Type* TP = EXP->getType().getTypePtr();

  return __astc->getCanonicalType(TP);
}
/**********************************************************************************************************************/
class AbstractMatcherHandler : public virtual MatchFinder::MatchCallback
{
  public:
    AbstractMatcherHandler (Rewriter &Rewrite) : R(Rewrite) {}

  public:
    virtual void run(const MatchFinder::MatchResult &MR)
    {

    }

  private:
    Rewriter &R;
};
/**********************************************************************************************************************/
class MatcherHandlerLVL0 : public AbstractMatcherHandler
{
  public:
    explicit MatcherHandlerLVL0 (Rewriter &Rewrite) : AbstractMatcherHandler(Rewrite) {}

    virtual void run(const MatchFinder::MatchResult &MR) override
    {

    }

  private:
};
/**********************************************************************************************************************/
class NameFinder
{
  public:
    NameFinder () {}

    class runDeclRefExprMatcher
    {
      public:
        runDeclRefExprMatcher (Rewriter &__rwrt) : LVL0Proto(__rwrt), __rwrt(__rwrt) {}

        virtual void runMatcher(const StringRef __sr, ASTContext &__ctx)
        {
          Matcher.addMatcher(declRefExpr(to(namedDecl(hasName(__sr.str())))).bind("declrefexpbyname"), &LVL0Proto);
          Matcher.matchAST(__ctx);
        }

      private:
        MatchFinder Matcher;
        MatcherHandlerLVL0 LVL0Proto;
        Rewriter __rwrt;
        StringRef __sr;
    };

    class runNamedDeclMatcher
    {
      public:
        runNamedDeclMatcher (Rewriter &__rwrt) : LVL0Proto(__rwrt), __rwrt(__rwrt) {}

        virtual void runMatcher(const StringRef __sr, ASTContext &__ctx)
        {
          Matcher.addMatcher(declRefExpr(to(namedDecl(hasName(__sr.str())))).bind("nameddeclbyname"), &LVL0Proto);
          Matcher.matchAST(__ctx);
        }

      private:
        MatchFinder Matcher;
        MatcherHandlerLVL0 LVL0Proto;
        Rewriter __rwrt;
        StringRef __sr;
    };

  private:
};
/**********************************************************************************************************************/
/**********************************************************************************************************************/
/**********************************************************************************************************************/
class IfBreaker : public MatchFinder::MatchCallback 
{
  public:
    IfBreaker (Rewriter &Rewrite) : Rewrite(Rewrite) {}

    virtual void run(const MatchFinder::MatchResult &MR)
    {
      if (MR.Nodes.getNodeAs<clang::UnaryOperator>("uno") != nullptr)
      {
        const clang::UnaryOperator* UO = MR.Nodes.getNodeAs<clang::UnaryOperator>("uno");

        SourceLocation SL = UO->getLocStart();
        CheckSLValidity(SL);
        SL = Devi::SourceLocationHasMacro(SL, Rewrite);

        const Expr* EXP = UO->getSubExpr();
        const ast_type_traits::DynTypedNode DynNode = ast_type_traits::DynTypedNode::create<clang::Expr>(*EXP);
        bruiser::TypeInfo TIProto(&DynNode);

        const clang::Type* CTP = TIProto.getTypeInfo(MR.Context);

      }

      if (MR.Nodes.getNodeAs<clang::BinaryOperator>("dous") != nullptr)
      {
        const clang::BinaryOperator* BO = MR.Nodes.getNodeAs<clang::BinaryOperator>("dous");

        SourceLocation SL = BO->getLocStart();
        CheckSLValidity(SL);
        SL = Devi::SourceLocationHasMacro(SL, Rewrite);

        const Expr* LHS = BO->getLHS();
        const Expr* RHS = BO->getRHS();

        QualType LQT = LHS->getType();
        QualType RQT = RHS->getType();

        const clang::Type* LTP = LQT.getTypePtr();
        const clang::Type* RTP = RQT.getTypePtr();

        const clang::Type* CLTP = MR.Context->getCanonicalType(LTP);
        const clang::Type* CRTP = MR.Context->getCanonicalType(RTP);
      }
    }

  private:
    Rewriter &Rewrite;
    MatchFinder Matcher;
};
/**********************************************************************************************************************/
/**
 * @brief Hijacks the main main and replaces it with bruiser's main.
 */
class MainWrapper : public MatchFinder::MatchCallback
{
public:
  MainWrapper (Rewriter &Rewrite) : Rewrite(Rewrite) {}

  virtual void run(const MatchFinder::MatchResult &MR)
  {
    if (MR.Nodes.getNodeAs<clang::FunctionDecl>("mainwrapper") != nullptr)
    {
      const FunctionDecl* FD = MR.Nodes.getNodeAs<clang::FunctionDecl>("mainwrapper");

      SourceLocation SL = FD->getLocStart();
      CheckSLValidity(SL);
      SL = Devi::SourceLocationHasMacro(SL, Rewrite);

      SourceLocation SLE = FD->getLocEnd();
      CheckSLValidity(SLE);
      SLE = Devi::SourceLocationHasMacro(SLE, Rewrite);

      SourceRange SR(SL, SLE);

      std::string MainSig = Rewrite.getRewrittenText(SR); 

      size_t mainbegin = MainSig.find("main");

      StringRef __sr("sub_main");

      Rewrite.ReplaceText(SL.getLocWithOffset(mainbegin), 4U, __sr);

      Rewrite.InsertTextAfter(SLE.getLocWithOffset(1U), StringRef("\n\nint main(int argc, const char **argv)\n{\n\tsub_main(argc, argv);\n}"));

      //ruiseRep << "changed main main's name.\n"
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
  MyASTConsumer(Rewriter &R) : HIfBreaker(R), HMainWrapper(R)
  {}

  void HandleTranslationUnit(ASTContext &Context) override 
  {
    Matcher.addMatcher(ifStmt(hasDescendant(expr(anyOf(unaryOperator().bind("uno"), binaryOperator().bind("dous"))))), &HIfBreaker);

    Matcher.addMatcher(functionDecl(hasName("main")).bind("mainwrapper"), &HMainWrapper);

    Matcher.matchAST(Context);
  }

private:
  IfBreaker HIfBreaker;
  MainWrapper HMainWrapper;
  MatchFinder Matcher;
  Rewriter R;
};
/**********************************************************************************************************************/
class MyFrontendAction : public ASTFrontendAction 
{
public:
  MyFrontendAction() {}
  void EndSourceFileAction() override 
  {
    TheRewriter.getEditBuffer(TheRewriter.getSourceMgr().getMainFileID()).write(llvm::outs());
  }

  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, StringRef file) override 
  {
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
  CommonOptionsParser op(argc, argv, BruiserCategory);
  ClangTool Tool(op.getCompilations(), op.getSourcePathList());

  bruiser::BruiserReport BruiseRep;

  return Tool.run(newFrontendActionFactory<MyFrontendAction>().get());
}
/*last line interntionally left blank.*/

