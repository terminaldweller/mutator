
/***************************************************Project Mutator****************************************************/
//-*-c++-*-
/*first line intentionally left blank.*/
/*the main for bruiser.*/
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
#include "fstream"
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

#include "luadummy.h"
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
namespace
{
  static llvm::cl::OptionCategory BruiserCategory("Empty");
  std::vector<std::string> PushToLua;

  bruiser::M0_ERR m0_err;
  bruiser::BruiserReport BruiseRep;

  struct ShellGlobal
  {
    ShellGlobal() = default;

    std::vector<std::string> PATH;
    std::vector<std::string> SOURCE_FILES;
  };

  ShellGlobal ShellGlobalInstance;
}
/**********************************************************************************************************************/
cl::opt<bool> Intrusive("intrusive", cl::desc("If set true. bruiser will mutate the source."), cl::init(true), cl::cat(BruiserCategory), cl::ZeroOrMore);
cl::opt<std::string> M0XMLPath("xmlpath", cl::desc("tells bruiser where to find the XML file containing the Mutator-LVL0 report."), cl::init(bruiser::M0REP), cl::cat(BruiserCategory), cl::ZeroOrMore);
cl::opt<bool> LuaJIT("jit", cl::desc("should bruiser use luajit or not."), cl::init(true), cl::cat(BruiserCategory), cl::ZeroOrMore);
/**********************************************************************************************************************/
class LuaEngine
{
  public:
    LuaEngine() 
    {
      LS = luaL_newstate();
    }

    /*@DEVI-this will just create member functions that open a single lua libarary. 
     * For example to load the string library just call LuaEngine::LoadstringLib().*/
#define OPEN_LUA_LIBS(__x1) \
      void Load##__x1##Lib(void){\
      luaL_requiref(LS, #__x1, luaopen_##__x1, 1);}

    OPEN_LUA_LIBS(base)
    OPEN_LUA_LIBS(table)
    OPEN_LUA_LIBS(io)
    OPEN_LUA_LIBS(string)
    OPEN_LUA_LIBS(math)
    OPEN_LUA_LIBS(os)

#undef OPEN_LUA_LIBS

    void LoadAuxLibs(void)
    {
      luaL_requiref(LS, "table", luaopen_table, 1);
      luaL_requiref(LS, "io", luaopen_io, 1);
      luaL_requiref(LS, "string", luaopen_string, 1);
    }

    void LoadEverylib(void)
    {
      luaL_openlibs(LS);
    }

    void RunString(char* __lua_string)
    {

    }

    void RunChunk(char* __lua_chunk)
    {
      dostring(LS, __lua_chunk, "test");
    }

    int RunScript(char* __lua_script)
    {
      return luaL_dofile(LS, __lua_script);
    }

    void Test(void)
    {
      luaL_dofile(LS, "./lua-scripts/test.lua");
      luaL_dofile(LS, "./lua-scripts/test1.lua");
      luaL_dofile(LS, "./lua-scripts/test2.lua");
    }

    void Test2(void)
    {
      luaL_dofile(LS, "./lua-scripts/test1.lua");
    }

    void Test3(void)
    {
      luaL_dofile(LS, "./lua-scripts/test2.lua");
    }

    void Test4(void)
    {
      luaL_dofile(LS, "./lua-scripts/test3.lua");
    }

    lua_State* GetLuaState(void)
    {
      return this->LS;
    }

    void Cleanup(void)
    {
      lua_close(LS);
    }

  private:
    lua_State* LS;
};
/**********************************************************************************************************************/
class CompilationDatabaseProcessor
{
  public:
  CompilationDatabaseProcessor(CompilationDatabase &__cdb) : CDB(__cdb) {}

  void CalcMakePath(void)
  {
    std::vector<std::string> Paths;
    std::vector<CompileCommand> CCV = CDB.getAllCompileCommands();

    for(auto &iter : CCV)
    {
      SourceFiles.push_back(iter.Filename);
      //PRINT_WITH_COLOR_LB(RED, SourceFiles.back().c_str());
    }

    MakePath = CCV[0].Directory;
    //PRINT_WITH_COLOR_LB(RED, MakePath.c_str());
  }

  std::string GetMakePath(void)
  {
    return this->MakePath;
  }

  std::vector<std::string> GetSourceFiles(void)
  {
    return this->SourceFiles;
  }

  void PopulateGPATH(void)
  {
    ShellGlobalInstance.PATH.push_back(MakePath);
  }

  void PopulateGSOURCEFILES(void)
  {
    for (auto &iter : SourceFiles)
    {
      ShellGlobalInstance.SOURCE_FILES.push_back(iter);
    }
  }

  private:
  CompilationDatabase &CDB;
  std::string MakePath;
  std::vector<std::string> SourceFiles;
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

/**
 * @brief Will print the argument in the log file. Expects to receive valid types usable for a stream.
 *
 * @param __arg
 *
 * @return Returns true if the write was successful, false otherwise.
 */
template <typename T>
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
          //printf(CYAN"%s",  R.getRewrittenText(clang::SourceRange(SLShebang, SLBody.getLocWithOffset(-1))).c_str());
          //printf(NORMAL "\n");
          PRINT_WITH_COLOR_LB(CYAN, R.getRewrittenText(clang::SourceRange(SLShebang, SLBody.getLocWithOffset(-1))).c_str());
          PushToLua.push_back(R.getRewrittenText(clang::SourceRange(SLShebang, SLBody.getLocWithOffset(-1))));
          //PRINT_WITH_COLOR_LB(GREEN, "end");
        }
        else
        {
          SourceLocation SL = FD->getLocStart();
          SourceLocation SLE = FD->getLocEnd();
          PRINT_WITH_COLOR_LB(CYAN, R.getRewrittenText(clang::SourceRange(SL, SLE)).c_str());
          PushToLua.push_back(R.getRewrittenText(clang::SourceRange(SL, SLE)));
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

        PRINT_WITH_COLOR_LB(CYAN, R.getRewrittenText(SourceRange(VD->getLocStart(), VD->getLocEnd())).c_str());
        PushToLua.push_back(R.getRewrittenText(SourceRange(VD->getLocStart(), VD->getLocEnd())));
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

        PRINT_WITH_COLOR_LB(CYAN, R.getRewrittenText(SourceRange(RD->getLocStart(), RD->getLocEnd())).c_str());
        PushToLua.push_back(R.getRewrittenText(SourceRange(RD->getLocStart(), RD->getLocEnd())));
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
  public:
    BlankDiagConsumer() = default;
    virtual ~BlankDiagConsumer() {}
    virtual void HandleDiagnostic(DiagnosticsEngine::Level DiagLevel, const Diagnostic &Info) override {}
};
/**********************************************************************************************************************/
class BruiserFrontendAction : public ASTFrontendAction 
{
public:
  BruiserFrontendAction() {}
  virtual ~BruiserFrontendAction()
  {
    delete BDCProto;
  }

  void EndSourceFileAction() override 
  {
    TheRewriter.getEditBuffer(TheRewriter.getSourceMgr().getMainFileID()).write(llvm::outs());
  }

  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, StringRef file) override 
  {
    DiagnosticsEngine &DE = CI.getPreprocessor().getDiagnostics();
    DE.setClient(BDCProto, false);
    TheRewriter.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
    return llvm::make_unique<BruiserASTConsumer>(TheRewriter);
  }

private:
  Rewriter TheRewriter;
  BlankDiagConsumer* BDCProto = new BlankDiagConsumer;
};
/**********************************************************************************************************************/
class LiveActionListVars : public ASTFrontendAction
{
  public:
    LiveActionListVars() = default;
    virtual ~LiveActionListVars()
    {
      delete BDCProto;
    }

    void EndSourceFileAction() override {}

    std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, StringRef file) override
    {
      DiagnosticsEngine &DE = CI.getPreprocessor().getDiagnostics();
      DE.setClient(BDCProto, false);
      TheRewriter.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
      return llvm::make_unique<LiveListVarsConsumer>(TheRewriter);
    }

  private:
    Rewriter TheRewriter;
    BlankDiagConsumer* BDCProto = new BlankDiagConsumer();
};
/**********************************************************************************************************************/
class LiveActionListFuncs : public ASTFrontendAction
{
  public:
    LiveActionListFuncs() {}
    ~LiveActionListFuncs()
    {
      delete BDCProto;
    }

    void EndSourceFileAction() override {}

    std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, StringRef file) override
    {
      DiagnosticsEngine &DE = CI.getPreprocessor().getDiagnostics();
      DE.setClient(BDCProto, false);
      TheRewriter.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
      return llvm::make_unique<LiveListFuncsConsumer>(TheRewriter);
    }

  private:
    Rewriter TheRewriter;
    BlankDiagConsumer* BDCProto = new BlankDiagConsumer;
};
/**********************************************************************************************************************/
class LiveActionListStructs : public ASTFrontendAction
{
  public:
    LiveActionListStructs() {}
    ~LiveActionListStructs()
    {
      delete BDCProto;
    }

    void EndSourceFileAction() override {}

    std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, StringRef file) override
    {
      DiagnosticsEngine &DE = CI.getPreprocessor().getDiagnostics();
      DE.setClient(BDCProto, false);
      TheRewriter.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
      return llvm::make_unique<LiveListStructConsumer>(TheRewriter);
    }

  private:
    Rewriter TheRewriter;
    BlankDiagConsumer* BDCProto = new BlankDiagConsumer;
};
/**********************************************************************************************************************/
class LiveActionListClasses : public ASTFrontendAction
{
  public:
    LiveActionListClasses() {}
    ~LiveActionListClasses()
    {
      delete BDCProto;
    }

    void EndSourceFileAction() override {}

    std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, StringRef file) override
    {
      DiagnosticsEngine &DE = CI.getPreprocessor().getDiagnostics();
      DE.setClient(BDCProto, false);
      TheRewriter.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
      return llvm::make_unique<LiveListClassConsumer>(TheRewriter);
    }

  private:
    Rewriter TheRewriter;
    BlankDiagConsumer* BDCProto = new BlankDiagConsumer;
};
/**********************************************************************************************************************/
class LiveActionListUnions : public ASTFrontendAction
{
  public:
    LiveActionListUnions() {}
    ~LiveActionListUnions()
    {
      delete BDCProto;
    }

    void EndSourceFileAction() override {}

    std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, StringRef file) override
    {
      DiagnosticsEngine &DE = CI.getPreprocessor().getDiagnostics();
      DE.setClient(BDCProto, false);
      TheRewriter.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
      return llvm::make_unique<LiveListUnionConsumer>(TheRewriter);
    }

  private:
    Rewriter TheRewriter;
    BlankDiagConsumer* BDCProto = new BlankDiagConsumer;
};
/**********************************************************************************************************************/
class LiveActionListArrays : public ASTFrontendAction
{
  public:
    LiveActionListArrays() {}
    ~LiveActionListArrays()
    {
      delete BDCProto;
    }

    void EndSourceFileAction() override {}

    std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, StringRef file) override
    {
      DiagnosticsEngine &DE = CI.getPreprocessor().getDiagnostics();
      DE.setClient(BDCProto, false);
      TheRewriter.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
      return llvm::make_unique<LiveListArrayConsumer>(TheRewriter);
    }

  private:
    Rewriter TheRewriter;
    BlankDiagConsumer* BDCProto = new BlankDiagConsumer;
};
/**********************************************************************************************************************/
/**********************************************************************************************************************/
/*lua wrappers*/
class LuaWrapper
{
  public:
    LuaWrapper(ClangTool &__CT) : CT(__CT) {}

    /*print out the history*/
    int BruiserLuaHistory(lua_State* __ls)
    {
      std::ifstream historyfile;
      historyfile.open(SHELL_HISTORY_FILE);

      std::string tempstring;
      unsigned int tempint = 0;
      while(std::getline(historyfile, tempstring))
      {
        printf(GREEN"%d - %s", tempint, tempstring.c_str());
        printf(NORMAL"\n");

        tempint++;
      }

      return tempint;
    }

    /*print the help menu*/
    int BruiserLuaHelp(lua_State* __ls)
    {
      unsigned int argcount = 0U;

      for (auto &iter : bruiser::CMDHelp)
      {
        printf(GREEN"%s:%s:%s",iter.name.c_str(),iter.proto.c_str(),iter.descr.c_str());
        printf(NORMAL"\n");
        argcount++;
      }

      std::cout << NORMAL;
      return argcount;
    }

    /*hijakcs the main main*/
    int BruiserLuaHijackMain(lua_State* __ls)
    {
        int RunResult = this->GetClangTool().run(newFrontendActionFactory<BruiserFrontendAction>().get());
        //std::cout << CYAN <<"hijacking main returned " << RunResult << "\n" << NORMAL;
        printf(CYAN"hijacking main returned %d", RunResult);
        printf(NORMAL"\n");

        return 1;
    }

    /*print out bruiser version*/
    int BruiserLuaVersion(lua_State* __ls)
    {
        PRINT_WITH_COLOR_LB(GREEN, "bruiser experimental version something.");
        PRINT_WITH_COLOR_LB(GREEN, "project mutator");
        PRINT_WITH_COLOR_LB(GREEN, "GPL v2.0");
        PRINT_WITH_COLOR_LB(GREEN, "bloodstalker 2017");

        return 1;
    }

    /*clear the screen*/
    int BruiserLuaClear(lua_State* _ls)
    {
      linenoiseClearScreen();
      return 0;
    }

    /*read the m0 report*/
    int BruiserLuaM0(lua_State* __ls)
    {
        BruiseRep.PrintToLog("bruiser exited with:");

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

        return 1;
    }

    /*quit*/
    int BruiserLuaQuit(lua_State* __ls)
    {
      dostring(__ls, "os.exit()", "test");
      return 0;
    }

    /*quit*/
    int BruiserLuaExit(lua_State* __ls)
    {
      dostring(__ls, "os.exit()", "test");
      return 0;
    }

    int BruiserLuaRunMake(lua_State* __ls)
    {
      unsigned int result = 0U;
      unsigned int args = 0U;

      if ((args = lua_gettop(__ls)) != 1U)
      {
        PRINT_WITH_COLOR_LB(RED, "function was not called by one argument. Run help().");
        return 0;
      }

      const char *makepath;
      makepath = lua_tostring(__ls, 1);

      result = dostring(__ls, makepath, "make");

      lua_pushnumber(__ls, result);
      free((char*)makepath);
      return 1;
    }

    int BruiserLuaChangeHistorySize(lua_State* __ls)
    {
      unsigned int args = 0U;

      if ((args = lua_gettop(__ls)) != 1U)
      {
        PRINT_WITH_COLOR_LB(RED, "function was not called by one argument. Run help().");
        return 0;
      }
      
      unsigned int historysize = lua_tonumber(__ls, 1);

      linenoiseHistorySetMaxLen(historysize);

      return 0;
    }

    int BruiserLuaShowSourcecode(lua_State* __ls)
    {
      unsigned int args = 0U;

      if ((args = lua_gettop(__ls)) != 3U)
      {
        PRINT_WITH_COLOR_LB(RED, "function called with the wrong number of arguments. Run help().");
        return 0;
      }

      unsigned int linebegin = lua_tonumber(__ls, 1);
      unsigned int lineend = lua_tonumber(__ls, 2);
      std::string filename = lua_tostring(__ls, 3);
      std::fstream targetfile;

      for(auto &iter : ShellGlobalInstance.SOURCE_FILES)
      {
        if (iter.rfind(filename) == iter.size() - filename.size())
        {
          targetfile.open(iter);
        }
      }

      std::string line;
      while(getline(targetfile, line))
      {
        lua_pushstring(__ls, line.c_str());
      }

      targetfile.close();
      return lineend - linebegin + 1U;
    }

#define LIST_GENERATOR(__x1) \
    int List##__x1(lua_State* __ls)\
    {\
      assert(PushToLua.size() == 0);\
      unsigned int InArgCnt = 0U;\
      InArgCnt = lua_gettop(__ls);\
      unsigned int returncount=0U;\
      this->GetClangTool().run(newFrontendActionFactory<LiveActionList##__x1>().get());\
      for(auto &iter : PushToLua)\
      {lua_pushstring(__ls, iter.c_str());returncount++;}\
      PushToLua.clear();\
      return returncount;\
    }

#define LIST_LIST_GENERATORS \
    X(Funcs, "lists all functions") \
    X(Vars, "lists all variables") \
    X(Arrays, "lists all arrays") \
    X(Classes, "lists all classes") \
    X(Structs, "lists all structs") \
    X(Unions, "lists all unions") \

#define X(__x1, __x2) LIST_GENERATOR(__x1)

    LIST_LIST_GENERATORS

#undef X
#undef LIST_GENERATOR

    ClangTool GetClangTool(void)
    {
      return this->CT;
    }

  private:
    ClangTool CT;
};
/**********************************************************************************************************************/
/**********************************************************************************************************************/
typedef int (LuaWrapper::*mem_func)(lua_State* L);

/**
 * @brief A template function to wrap LuaWrapper members into somehting that lua accepts.
 *
 * @param __ls lua state
 *
 * @return returns a pointer to the member function wrapped the way lua accepts it.
 */
template<mem_func func>
int LuaDispatch(lua_State* __ls)
{
  LuaWrapper* LWPtr = *static_cast<LuaWrapper**>(lua_getextraspace(__ls));
  return ((*LWPtr).*func)(__ls);
}
/**********************************************************************************************************************/
/**********************************************************************************************************************/
/*Main*/
int main(int argc, const char **argv) 
{
  /*gets the compilation database and options for the clang instances that we would later run*/
  CommonOptionsParser op(argc, argv, BruiserCategory);
  ClangTool Tool(op.getCompilations(), op.getSourcePathList());

  CompilationDatabase &CDB = op.getCompilations();
  std::vector<CompileCommand> CCV = CDB.getAllCompileCommands();

  /*populating the shellglobalinstance*/
  CompilationDatabaseProcessor CDBP(CDB);
  CDBP.CalcMakePath();
  CDBP.PopulateGPATH();
  CDBP.PopulateGSOURCEFILES();

  /*initialize the LuaWrapper class so we can register and run them from lua.*/
  LuaWrapper LW(Tool);

  /*linenoise init*/
  linenoiseSetCompletionCallback(bruiser::ShellCompletion);
  linenoiseSetHintsCallback(bruiser::ShellHints);
  /*setting up the initial history size to SHELL_HISTORY_SIZE*/
  linenoiseHistorySetMaxLen(SHELL_HISTORY_SIZE);
  linenoiseHistoryLoad(SHELL_HISTORY_FILE);
  linenoiseSetMultiLine(1);

  /*start running the cli*/
  {
    char* command;

    LuaEngine LE;
    LE.LoadEverylib();
    *static_cast<LuaWrapper**>(lua_getextraspace(LE.GetLuaState())) = &LW;

    /*@DEVI-this part is just registering our LuaWrapper member functions with lua so we can call them from lua.*/
    lua_register(LE.GetLuaState(), "history", &LuaDispatch<&LuaWrapper::BruiserLuaHistory>);
    lua_register(LE.GetLuaState(), "help", &LuaDispatch<&LuaWrapper::BruiserLuaHelp>);
    lua_register(LE.GetLuaState(), "hijackmain", &LuaDispatch<&LuaWrapper::BruiserLuaHijackMain>);
    lua_register(LE.GetLuaState(), "version", &LuaDispatch<&LuaWrapper::BruiserLuaVersion>);
    lua_register(LE.GetLuaState(), "clear", &LuaDispatch<&LuaWrapper::BruiserLuaClear>);
    lua_register(LE.GetLuaState(), "m0", &LuaDispatch<&LuaWrapper::BruiserLuaM0>);
    lua_register(LE.GetLuaState(), "quit", &LuaDispatch<&LuaWrapper::BruiserLuaQuit>);
    lua_register(LE.GetLuaState(), "exit", &LuaDispatch<&LuaWrapper::BruiserLuaExit>);
    lua_register(LE.GetLuaState(), "make", &LuaDispatch<&LuaWrapper::BruiserLuaRunMake>);
    lua_register(LE.GetLuaState(), "historysize", &LuaDispatch<&LuaWrapper::BruiserLuaChangeHistorySize>);
    lua_register(LE.GetLuaState(), "showsource", &LuaDispatch<&LuaWrapper::BruiserLuaShowSourcecode>);
    /*its just regisering the List function from LuaWrapper with X-macros.*/
#define X(__x1, __x2) lua_register(LE.GetLuaState(), #__x1, &LuaDispatch<&LuaWrapper::List##__x1>);

    LIST_LIST_GENERATORS

#undef X
#undef LIST_LIST_GENERATORS

    while((command = linenoise("bruiser>>")) != NULL)
    {
      linenoiseHistoryAdd(command);
      linenoiseHistorySave(SHELL_HISTORY_FILE);
      LE.RunChunk(command);
      linenoiseFree(command);
    }

    /*end of bruiser main*/
    return 0;
  } //end of cli block
  
} //end of main
/*last line intentionally left blank.*/

