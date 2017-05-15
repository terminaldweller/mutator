
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
#include "CompletionHints.h"
#include "../mutator_aux.h"
/*standard headers*/
#include <string>
#include <cassert>
#include <iostream>
#include <regex>
/*LLVM headers*/
#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Lex/Lexer.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Function.h"
/*other*/
#include "linenoise/linenoise.h"
#include "lua-5.3.4/src/lua.hpp"
#include "lua-5.3.4/src/lualib.h"
#include "lua-5.3.4/src/lauxlib.h"
/**********************************************************************************************************************/
/*used namespaces*/
using namespace llvm;
using namespace clang;
using namespace clang::ast_matchers;
using namespace clang::driver;
using namespace clang::tooling;
/**********************************************************************************************************************/
#define __DBG_1
#if 1
#undef __DBG_1
#endif
/**********************************************************************************************************************/
/*global vars*/
static llvm::cl::OptionCategory BruiserCategory("Empty");

bruiser::M0_ERR m0_err;
bruiser::BruiserReport BruiseRep;
/**********************************************************************************************************************/
cl::opt<bool> Intrusive("intrusive", cl::desc("If set true. bruiser will mutate the source."), cl::init(true), cl::cat(BruiserCategory), cl::ZeroOrMore);
cl::opt<std::string> M0XMLPath("xmlpath", cl::desc("tells bruiser where to find the XML file containing the Mutator-LVL0 report."), cl::init(bruiser::M0REP), cl::cat(BruiserCategory), cl::ZeroOrMore);
/**********************************************************************************************************************/
class LuaEngine
{
  public:
    LuaEngine() 
    {
      LS = luaL_newstate();
    }

    void LoadBaseLib(void)
    {
      luaopen_base(LS);
    }

    void LoadAuxLibs(void)
    {
      luaopen_table(LS);
      luaopen_io(LS);
      luaopen_string(LS);
    }

    void LoadEverylib(void)
    {
      this->LoadBaseLib();
      this->LoadAuxLibs();
      luaopen_math(LS);
    }

    int RunScript(char* __lua_script)
    {
      return luaL_dofile(LS, __lua_script);
    }

    void Cleanup(void)
    {
      lua_close(LS);
    }

  private:
    lua_State* LS;
};
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

template <typename T>
/**
 * @brief Will print the argument in the log file. Expects to receive valid types usable for a stream.
 *
 * @param __arg
 *
 * @return Returns true if the write was successful, false otherwise.
 */
bool bruiser::BruiserReport::PrintToLog(T __arg)
{
  BruiserLog << __arg << "\n";
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
namespace bruiser
{
  void BruiserLinenoiseCompletionCallback(const char* __buf, linenoiseCompletions* __lc)
  {
    if (__buf[0] == 'h')
    {
      linenoiseAddCompletion(__lc, "help");
    }
  }

  char* BruiserLinenoiseHintsCallback(const char* __buf, int* __color, int* __bold)
  {
    if (!strcasecmp(__buf, "h"))
    {
      *__color = LN_MAGENTA;
      *__bold = NO_BOLD;
      return (char *)"elp";
    }

    return NULL;
  }
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

    virtual ~MatcherHandlerLVL0() {}

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

        const clang::Type* CTP [[maybe_unused]] = TIProto.getTypeInfo(MR.Context);

        NameFinder::runDeclRefExprMatcher DRENameMatcher(Rewrite);

        DRENameMatcher.runMatcher(StringRef(), *MR.Context);

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

        const clang::Type* CLTP [[maybe_unused]] = MR.Context->getCanonicalType(LTP);
        const clang::Type* CRTP [[maybe_unused]] = MR.Context->getCanonicalType(RTP);
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

      /*@DEVI-obviously the best way to do this is to use the main signature already used, instead of going with a general predefined one. the current form is a temp.*/
      Rewrite.InsertTextAfter(SLE.getLocWithOffset(1U), StringRef("\n\nint main(int argc, const char **argv)\n{\n\treturn sub_main(argc, argv);\n}\n"));

      BruiseRep.PrintToLog("hijacked main main.");
    }
  }

  private:
  Rewriter &Rewrite;
};
/**********************************************************************************************************************/
class LiveListFuncs : public MatchFinder::MatchCallback
{
  public:
    LiveListFuncs (Rewriter &R) : R(R) {}

    virtual void run(const MatchFinder::MatchResult &MR)
    {
      if (MR.Nodes.getNodeAs<clang::FunctionDecl>("livelistfuncs") != nullptr)
      {
        const clang::FunctionDecl* FD = MR.Nodes.getNodeAs<clang::FunctionDecl>("livelistfuncs");

        if (FD->hasBody())
        {
          Stmt* Body = FD->getBody();
          SourceLocation SLBody = Body->getLocStart();
          SourceLocation SLShebang = FD->getLocStart();
          //PRINT_WITH_COLOR_LB(GREEN, "begin");
          PRINT_WITH_COLOR_LB(CYAN, R.getRewrittenText(clang::SourceRange(SLShebang, SLBody.getLocWithOffset(-1))));
          //PRINT_WITH_COLOR_LB(GREEN, "end");
        }
        else
        {
          SourceLocation SL = FD->getLocStart();
          SourceLocation SLE = FD->getLocEnd();
          PRINT_WITH_COLOR_LB(CYAN, R.getRewrittenText(clang::SourceRange(SL, SLE)));
        }
      }
    }

  private:
    Rewriter R;
};
/**********************************************************************************************************************/
class LiveListVars : public MatchFinder::MatchCallback
{
  public:
    LiveListVars (Rewriter &R) : R(R) {}

    virtual void run(const MatchFinder::MatchResult &MR)
    {
      if (MR.Nodes.getNodeAs<clang::VarDecl>("livelistvars") != nullptr)
      {
        const clang::VarDecl* VD = MR.Nodes.getNodeAs<clang::VarDecl>("livelistvars");

        PRINT_WITH_COLOR_LB(CYAN, R.getRewrittenText(SourceRange(VD->getLocStart(), VD->getLocEnd())));
      }
    }

  private:
    Rewriter R;
};
/**********************************************************************************************************************/
class LiveListRecords : public MatchFinder::MatchCallback
{
  public:
    LiveListRecords (Rewriter &R) : R(R) {}

    virtual void run(const MatchFinder::MatchResult &MR)
    {
      if (MR.Nodes.getNodeAs<clang::RecordDecl>("livelistvars") != nullptr)
      {
        const clang::RecordDecl* RD = MR.Nodes.getNodeAs<clang::RecordDecl>("livelistvars");

        PRINT_WITH_COLOR_LB(CYAN, R.getRewrittenText(SourceRange(RD->getLocStart(), RD->getLocEnd())));
      }
    }

  private:
    Rewriter R;
};
/**********************************************************************************************************************/
/**********************************************************************************************************************/
/**********************************************************************************************************************/
/**********************************************************************************************************************/
/**********************************************************************************************************************/
/**********************************************************************************************************************/
/**********************************************************************************************************************/
class BruiserASTConsumer : public ASTConsumer {

public:
  BruiserASTConsumer(Rewriter &R) : HIfBreaker(R), HMainWrapper(R)
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
class LiveConsumerFactory
{
  public:
    LiveConsumerFactory() {}

    template<typename T>
      void operator()(T __consumer)
      {

      }

};
/**********************************************************************************************************************/
class LiveConsumer : public ASTConsumer
{
  public:
    LiveConsumer(Rewriter &R) : HLLVars(R), HLLFuncs(R), HLLRecords(R)
    {}

    void HandleTranslationUnit(ASTContext &ctx) override
    {
      Matcher.matchAST(ctx);
    }

  private:
    MatchFinder Matcher;
    LiveListVars HLLVars;
    LiveListFuncs HLLFuncs;
    LiveListRecords HLLRecords;
    Rewriter R;
};
/**********************************************************************************************************************/
class LiveListVarsConsumer : public ASTConsumer
{
  public:
    LiveListVarsConsumer(Rewriter &R) : HLLVars(R)
    {}

    void HandleTranslationUnit(ASTContext &ctx) override
    {
      Matcher.addMatcher(varDecl().bind("livelistvars"), &HLLVars);

      Matcher.matchAST(ctx);
    }

  private:
    LiveListVars HLLVars;
    Rewriter R;
    MatchFinder Matcher;
};
/**********************************************************************************************************************/
class LiveListFuncsConsumer : public ASTConsumer
{
  public:
    LiveListFuncsConsumer(Rewriter &R) : HLLFuncs(R)
    {}

    void HandleTranslationUnit(ASTContext &ctx) override
    {
      Matcher.addMatcher(functionDecl().bind("livelistfuncs"), &HLLFuncs);

      Matcher.matchAST(ctx);
    }

  private:
    MatchFinder Matcher;
    LiveListFuncs HLLFuncs;
    Rewriter R;
};
/**********************************************************************************************************************/
class LiveListClassConsumer : public ASTConsumer
{
  public:
    LiveListClassConsumer(Rewriter &R) : HLLRecords(R)
    {}

    void HandleTranslationUnit(ASTContext &ctx) override
    {
      Matcher.addMatcher(recordDecl(isClass()).bind("livelistclass"), &HLLRecords);

      Matcher.matchAST(ctx);
    }

  private:
    MatchFinder Matcher;
    LiveListRecords HLLRecords;
    Rewriter R;
};
/**********************************************************************************************************************/
class LiveListStructConsumer : public ASTConsumer
{
  public:
    LiveListStructConsumer(Rewriter &R) : HLLRecords(R)
    {}

    void HandleTranslationUnit(ASTContext &ctx) override
    {
      Matcher.addMatcher(recordDecl(isStruct()).bind("liveliststruct"), &HLLRecords);

      Matcher.matchAST(ctx);
    }

  private:
    MatchFinder Matcher;
    LiveListRecords HLLRecords;
    Rewriter R;
};
/**********************************************************************************************************************/
class LiveListUnionConsumer : public ASTConsumer
{
  public:
    LiveListUnionConsumer(Rewriter &R) : HLLRecords(R)
    {}

    void HandleTranslationUnit(ASTContext &ctx) override
    {
      Matcher.addMatcher(recordDecl(isUnion()).bind("livelistclass"), &HLLRecords);

      Matcher.matchAST(ctx);
    }

  private:
    MatchFinder Matcher;
    LiveListRecords HLLRecords;
    Rewriter R;
};
/**********************************************************************************************************************/
class LiveListArrayConsumer : public ASTConsumer
{
  public:
    LiveListArrayConsumer(Rewriter &R) : HLLVars(R)
    {}

    void HandleTranslationUnit(ASTContext &ctx) override
    {
      Matcher.addMatcher(varDecl(hasType(arrayType())).bind("livelistvars"), &HLLVars);

      Matcher.matchAST(ctx);
    }

  private:
    MatchFinder Matcher;
    LiveListVars HLLVars;
    Rewriter R;
};
/**********************************************************************************************************************/
/**********************************************************************************************************************/
/**********************************************************************************************************************/
/**********************************************************************************************************************/
class BlankDiagConsumer : public clang::DiagnosticConsumer
{
  virtual void HandleDiagnostic(DiagnosticsEngine::Level, const Diagnostic &Info) override {}
};
/**********************************************************************************************************************/
class BruiserFrontendAction : public ASTFrontendAction 
{
public:
  BruiserFrontendAction() {}
  void EndSourceFileAction() override 
  {
    TheRewriter.getEditBuffer(TheRewriter.getSourceMgr().getMainFileID()).write(llvm::outs());
  }

  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, StringRef file) override 
  {
    DiagnosticsEngine &DE = CI.getPreprocessor().getDiagnostics();
    BlankDiagConsumer* BDCProto = new BlankDiagConsumer;
    DE.setClient(BDCProto);
    TheRewriter.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
    return llvm::make_unique<BruiserASTConsumer>(TheRewriter);
  }

private:
  Rewriter TheRewriter;
};
/**********************************************************************************************************************/
class LiveActionListVars : public ASTFrontendAction
{
  public:
    LiveActionListVars() {}

    void EndSourceFileAction() override {}

    std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, StringRef file) override
    {
      DiagnosticsEngine &DE = CI.getPreprocessor().getDiagnostics();
      BlankDiagConsumer* BDCProto = new BlankDiagConsumer;
      DE.setClient(BDCProto);
      TheRewriter.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
      return llvm::make_unique<LiveListVarsConsumer>(TheRewriter);
    }

  private:
    Rewriter TheRewriter;
};
/**********************************************************************************************************************/
class LiveActionListFuncs : public ASTFrontendAction
{
  public:
    LiveActionListFuncs() {}

    void EndSourceFileAction() override {}

    std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, StringRef file) override
    {
      DiagnosticsEngine &DE = CI.getPreprocessor().getDiagnostics();
      BlankDiagConsumer* BDCProto = new BlankDiagConsumer;
      DE.setClient(BDCProto);
      TheRewriter.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
      return llvm::make_unique<LiveListFuncsConsumer>(TheRewriter);
    }

  private:
    Rewriter TheRewriter;
};
/**********************************************************************************************************************/
class LiveActionListStructs : public ASTFrontendAction
{
  public:
    LiveActionListStructs() {}

    void EndSourceFileAction() override {}

    std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, StringRef file) override
    {
      DiagnosticsEngine &DE = CI.getPreprocessor().getDiagnostics();
      BlankDiagConsumer* BDCProto = new BlankDiagConsumer;
      DE.setClient(BDCProto);
      TheRewriter.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
      return llvm::make_unique<LiveListStructConsumer>(TheRewriter);
    }

  private:
    Rewriter TheRewriter;
};
/**********************************************************************************************************************/
class LiveActionListClasses : public ASTFrontendAction
{
  public:
    LiveActionListClasses() {}

    void EndSourceFileAction() override {}

    std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, StringRef file) override
    {
      DiagnosticsEngine &DE = CI.getPreprocessor().getDiagnostics();
      BlankDiagConsumer* BDCProto = new BlankDiagConsumer;
      DE.setClient(BDCProto);
      TheRewriter.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
      return llvm::make_unique<LiveListClassConsumer>(TheRewriter);
    }

  private:
    Rewriter TheRewriter;
};
/**********************************************************************************************************************/
class LiveActionListUnions : public ASTFrontendAction
{
  public:
    LiveActionListUnions() {}

    void EndSourceFileAction() override {}

    std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, StringRef file) override
    {
      DiagnosticsEngine &DE = CI.getPreprocessor().getDiagnostics();
      BlankDiagConsumer* BDCProto = new BlankDiagConsumer;
      DE.setClient(BDCProto);
      TheRewriter.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
      return llvm::make_unique<LiveListUnionConsumer>(TheRewriter);
    }

  private:
    Rewriter TheRewriter;
};
/**********************************************************************************************************************/
class LiveActionListArrays : public ASTFrontendAction
{
  public:
    LiveActionListArrays() {}

    void EndSourceFileAction() override {}

    std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, StringRef file) override
    {
      DiagnosticsEngine &DE = CI.getPreprocessor().getDiagnostics();
      BlankDiagConsumer* BDCProto = new BlankDiagConsumer;
      DE.setClient(BDCProto);
      TheRewriter.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
      return llvm::make_unique<LiveListArrayConsumer>(TheRewriter);
    }

  private:
    Rewriter TheRewriter;
};
/**********************************************************************************************************************/
/**********************************************************************************************************************/
/*Main*/
int main(int argc, const char **argv) 
{
  int RunResult;
  bruiser::ShellHistory shHistory;

  std::regex listcommand("^list\\s");
  std::regex listfuncs("^list\\sfuncs$");
  std::regex listvars("^list\\svars$");
  std::regex listarrays("^list\\sarrays$");
  std::regex listrecords("^list\\srecords$");
  std::regex listclasses("^list\\sclasses$");
  std::regex liststructs("^list\\sstructs$$");
  std::regex listunions("^list\\sunions$$");
  std::regex dumplist("^list\\sdump\\s");
  std::smatch smresult;

  CommonOptionsParser op(argc, argv, BruiserCategory);
  ClangTool Tool(op.getCompilations(), op.getSourcePathList());

  /*linenoise init*/
  linenoiseSetCompletionCallback(bruiser::ShellCompletion);
  linenoiseSetHintsCallback(bruiser::ShellHints);
  /*setting up the initial history size to SHELL_HISTORY_SIZE*/
  linenoiseHistorySetMaxLen(SHELL_HISTORY_SIZE);
  linenoiseHistoryLoad(SHELL_HISTORY_FILE);
  linenoiseSetMultiLine(1);


  {
    char* command;
    while((command = linenoise("bruiser>>")) != NULL)
    {
      linenoiseHistoryAdd(command);
      linenoiseHistorySave(SHELL_HISTORY_FILE);

      //std::cin.getline(command, sizeof(command));
      std::string dummy_string(command);

      shHistory.History.push_back(command);

      if (std::regex_search(dummy_string, smresult, listcommand))
      {
        if (std::regex_search(dummy_string, smresult, listfuncs))
        {
          RunResult = Tool.run(newFrontendActionFactory<LiveActionListFuncs>().get());
          continue;
        }

        if (std::regex_search(dummy_string, smresult, listvars))
        {
          RunResult = Tool.run(newFrontendActionFactory<LiveActionListVars>().get());
          continue;
        }

        if (std::regex_search(dummy_string, smresult, listarrays))
        {
          RunResult = Tool.run(newFrontendActionFactory<LiveActionListArrays>().get());
          continue;
        }

        if (std::regex_search(dummy_string, smresult, listclasses))
        {
          RunResult = Tool.run(newFrontendActionFactory<LiveActionListClasses>().get());
          continue;
        }

        if (std::regex_search(dummy_string, smresult, liststructs))
        {
          RunResult = Tool.run(newFrontendActionFactory<LiveActionListStructs>().get());
          continue;
        }

        if (std::regex_search(dummy_string, smresult, listunions))
        {
          RunResult = Tool.run(newFrontendActionFactory<LiveActionListUnions>().get());
          continue;
        }

        if (std::regex_search(dummy_string, smresult, listrecords))
        {
          NOT_IMPLEMENTED;
          continue;
        }
      }

      if (std::strcmp(command, "exit") == 0 || std::strcmp(command, "quit") == 0)
      {
        return 0;
      }

      if (std::strcmp(command, "m0") == 0)
      {
        BruiseRep.PrintToLog("bruiser exited with:");
        BruiseRep.PrintToLog(RunResult);

        bruiser::ReadM0 M0Rep;
        tinyxml2::XMLError XMLErr;

        XMLErr = M0Rep.LoadXMLDoc();
        if (XMLErr != XML_SUCCESS)
        {
          std::cout << RED << "could not load m0 xml report.\n" << NORMAL;
          std::cout << RED << "tinyxml2 returned " << XMLErr << NORMAL;
          return XMLErr;
        }

        XMLErr = M0Rep.ReadFirstElement();
        if (XMLErr != XML_SUCCESS)
        {
          std::cerr << RED << "could not read first element of m0 xml report.\n" << NORMAL;
          return XMLErr;
        }

        bruiser::SearchM0(M0Rep.getRootPointer());
        continue;
      }

      if (std::strcmp(command, "hijack main") == 0)
      {

        RunResult = Tool.run(newFrontendActionFactory<BruiserFrontendAction>().get());
        std::cout << CYAN <<"hijacking main returned " << RunResult << "\n" << NORMAL;
        continue;
      }

      if (std::strcmp(command, "list") == 0)
      {

      }

      if (std::strcmp(command, "clear") == 0)
      {
        linenoiseClearScreen();
        //std::cout << CLEAR;
        continue;
      }

      if (std::strcmp(command, "shell") == 0)
      {
        system("bash -i");
        continue;
      }

      if (std::strcmp(command, "help") == 0)
      {
        std::cout << GREEN;

        for (auto &iter : bruiser::CMDHelp)
        {
          std::cout << iter.name << ":" << iter.proto << ":" << iter.descr << "\n";
        }

        std::cout << NORMAL;

        continue;
      }

      if (std::strcmp(command, "history") == 0)
      {
        unsigned int _cnt = 0;
        for (auto &it : shHistory.History)
        {
          std::cout << _cnt << "." << it << "\n";
          _cnt++;
        }

        continue;
      }

      if (std::strcmp(command, "version") == 0)
      {
        PRINT_WITH_COLOR_LB(GRAY, "bruiser experimental version something.");
        PRINT_WITH_COLOR_LB(GRAY, "project mutator");
        PRINT_WITH_COLOR_LB(GRAY, "GPL v2.0");
        PRINT_WITH_COLOR_LB(GRAY, "bloodstalker 2017");
      }

      if (std::strcmp(command, "runlua") == 0)
      {
        LuaEngine LE;
        LE.LoadEverylib();
        LE.RunScript((char*)"/home/bloodstalker/devi/abbatoir/hole6/proto.lua");
        LE.Cleanup();
      }

      if (command[0] == '!')
      {
        int command_number;
        std::string cut_string;
        //std::cout << CYAN << command << "\n" << NORMAL;
        //std::cout << GREEN << dummy_string << "\n" << NORMAL;

        //assert(dummy_st == 130);

        cut_string = dummy_string.substr(1, dummy_string.length());
        //std::cout << GREEN << cut_string << "\n" << NORMAL;
        command_number = std::stoi(cut_string, 0, 10);

        if (command_number > SHELL_HISTORY_SIZE)
        {
          std::cout << RED << "the command number provided is bigger than SHELL_HISTORY_SIZE." << NORMAL << "\n";
        }
        else
        {
          std::cout << CYAN << shHistory.History[command_number] << NORMAL;
        }

        continue;
      }

      std::cout << RED << "unknown command. run help.\n" << NORMAL;

      linenoiseFree(command);
    }

    return 0;
  }
  
}
/*last line intentionally left blank.*/

