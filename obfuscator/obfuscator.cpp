
/*first line intentionally left blank.*/
/***************************************************Project Mutator****************************************************/
//-*-c++-*-
// a C/C++ source code obfuscator
/*Copyright (C) 2017 Farzad Sadeghi

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 3
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
/*included modules*/
/*project headers*/
#include "../mutator_aux.h"
#include "./keccak-tiny/keccak-tiny.hpp"
/*standard headers*/
#include <cassert>
#include <cstdlib>
#include <dirent.h>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
/*LLVM headers*/
#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Basic/LLVM.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Lex/Lexer.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "llvm/Support/raw_ostream.h"
/**********************************************************************************************************************/
/*used namespaces*/
using namespace llvm;
using namespace clang;
using namespace clang::ast_matchers;
using namespace clang::driver;
using namespace clang::tooling;
/**********************************************************************************************************************/
namespace {
  static llvm::cl::OptionCategory ObfuscatorCat("Obfuscator custom options");
  std::string TMP_FILE = "";
}
/**********************************************************************************************************************/
//#define DBG
// @DEVI-FIXME
#if defined(__linux__)
#define TEMP_FILE "/tmp/obfuscator-tee"
#elif defined(__MACH__) && defined(__APPLE__)
#define TEMP_FILE "/tmp/obfuscator-tee"
#elif defined(__CYGWIN__) || defined(_WIN32) || defined(_WIN64)
#define TEMP_FILE "C:\Temp\obfuscator-tee"
#else
#define TEMP_FILE "/tmp/obfuscator-tee"
#endif
/**********************************************************************************************************************/
/**
 * @brief Gets the list of all directories and sub-directories starting from a base directory.
 * @param _path where the the base directory is. 
 * @return Returns the list of all found dirs.
 * @warning WIP
 */
std::vector<std::string> listDirs(std::string _path) {
  std::vector<std::string> dummy_;
  DIR* dir_;
  struct dirent* ent_;
  if ((dir_ = opendir(_path.c_str())) != nullptr) {
    while((ent_ = readdir(dir_)) != nullptr) {
      std::cout << "name: "  << ent_->d_name << "\ttype:" << int(ent_->d_type) << "\n";
      if (ent_->d_type == DT_DIR) {}
      dummy_.push_back(ent_->d_name);
    }
  }
  else {
    perror("could not open directory.");
  }
  return dummy_;
}

/**
 * @brief Dumps a list of directories, meant as a debug options.
 * @param _dirs The input to dump.
 */
void dumpDirList(std::vector<std::string> _dirs) {
  for (auto &iter : _dirs) {std::cout << iter << "\t";}
  std::cout << "\n";
}

/**
 * @brief Extract the filename, the extension and the path to the file.
 * @param _path The path to the file. Should include the file name also.
 * @return Returns a tuple including the filename, the extension and the path in that order.
 */
std::tuple<std::string, std::string, std::string> getNameFromPath(std::string _path) {
  size_t pos = _path.rfind("/");
  size_t pos2 = _path.rfind(".");
  auto extension_ = _path.substr(pos2 + 1, std::string::npos);
  auto name_ = _path.substr(pos + 1, pos2 - pos - 1);
  auto path_ = _path.substr(0, pos);
  return std::make_tuple(name_, extension_, path_);
}

/**
 * @brief Remakes the name of a file.
 * @param _name The name.
 * @param _extension The extension.
 * @param _extra optional extra string to add to the name. Default value is "obfusc".
 * @return Returns the newly made name.
 */
std::string nameMaker(std::string _name, std::string _extension, std::string _extra = "obfusc") {
  if (_extra == "") {
    return _name + _extra + "." + _extension;
  }
  else {
    return _name + "-" + _extra + "." + _extension;
  }
}

/**
 * @brief Return the hash digest of an identifier.
 * @param _name the identifier name to hash.
 * @return Returns the new name.
 */
std::string getHashedName(std::string _name) {
  std::size_t hash = std::hash<std::string>{}(_name);
  return "FILE" + std::to_string(hash);
}

/**
 * @brief Hashes all the filenames.
 * @param _filenames The input.
 * @return Returns the map of the hashed identifiers.
 */
std::unordered_map<std::string, std::string> hashFilenames(const std::vector<std::string>& _filenames) {
  std::unordered_map<std::string, std::string> ret_map_;
  for (auto &iter : _filenames) {
    auto filename_ = getNameFromPath(iter);
    std::size_t hash = std::hash<std::string>{}(std::get<0>(filename_));
    auto new_name_ = "FILE" + std::to_string(hash);
    ret_map_.insert(std::make_pair(iter, new_name_));
  }
  return ret_map_;
}

/**
 * @brief Dumps all the hashed filenames. Meant for debugging.
 * @param _map The map containing the identifier and hash digest pair.
 */
void dumpHashFilenames(std::unordered_map<std::string, std::string> _map) {
  for (auto &iter : _map) {
    std::cout << "Key: " << iter.first << "\t" << "Value: " << iter.second << "\n";
  }
}

/**
 * @brief Get the tmp directory address at compile-time.
 * @return Return the address of the tmp directory.
 */
std::string getTempDir1() {
  std::string tmpdir_;
#if defined(__linux__)
  tmpdir_ = "/tmp";
#elif defined(__MACH__) && defined(__APPLE__)
  // $TMPDIR
  tmpdir_ = std::getenv("TMPDIR");
#elif defined(__CYGWIN__) || defined(_WIN32) || defined(_WIN64)
  // %TEMP%
  tmpdir_ = std::getenv("TEMP");
#else
  // unix-posix
  tmpdir_ = "/tmp";
#endif
  return tmpdir_;
}

/**
 * @brief Get the path to the tmp directory at run-time.
 * @return Returns the path to the tmp directory.
 */
std::string getTempDir2() {
  std::string tmpdir_ = std::getenv("TMPDIR");
  if (tmpdir_ == "") {
    tmpdir_ = std::getenv("TEMP");
    if (tmpdir_ == "") {
      tmpdir_ = std::getenv("TEMP");
      if (tmpdir_ == "") {tmpdir_ = "";}
    }
  }

  return tmpdir_;
}
/**********************************************************************************************************************/
/**
 * @brief MatchCallback for CallExpr.
 */
class CalledFunc : public MatchFinder::MatchCallback {
  public:
    CalledFunc(Rewriter &Rewrite) : Rewrite(Rewrite) {}

    virtual void run(const MatchFinder::MatchResult &MR) {
      if (MR.Nodes.getNodeAs<clang::CallExpr>("calledfunc") != nullptr) {
        const CallExpr *CE = MR.Nodes.getNodeAs<clang::CallExpr>("calledfunc");
        std::string name = CE->getDirectCallee()->getNameInfo().getAsString();
        std::size_t hash = std::hash<std::string>{}(name);
        std::string newname = "ID" + std::to_string(hash);
#ifdef DBG
        std::cout << "CallExpr name: "  << name << " Hash: " << hash << " New ID: " << newname << "\n";
#endif
        auto dummy = Rewrite.getRewrittenText(SourceRange(CE->getLocStart(), CE->getRParenLoc()));
        auto LParenOffset = dummy.find("(");
        SourceLocation SL = Devi::getSLSpellingLoc(CE->getLocStart(), Rewrite);
        SourceLocation SLE = Devi::getSLSpellingLoc(CE->getLocStart(), Rewrite).getLocWithOffset(LParenOffset - 0U);
        dummy = Rewrite.getRewrittenText(SourceRange(SL, SLE));
        Rewrite.ReplaceText(SourceRange(CE->getLocStart(), CE->getLocStart().getLocWithOffset(LParenOffset - 1U)), StringRef(newname));
      }
    }

  private:
    Rewriter &Rewrite;
};
/**********************************************************************************************************************/
/**
 * @brief MatchCallback for DeclRefExpr. changes the identifier with its hash digest.
 */
class CalledVar : public MatchFinder::MatchCallback {
  public:
    CalledVar (Rewriter &Rewrite) : Rewrite(Rewrite) {}

    virtual void run(const MatchFinder::MatchResult &MR) {
      if (MR.Nodes.getNodeAs<clang::DeclRefExpr>("calledvar") != nullptr) {
        const DeclRefExpr* DRE = MR.Nodes.getNodeAs<clang::DeclRefExpr>("calledvar");
        auto name = DRE->getNameInfo().getAsString();
        std::size_t hash = std::hash<std::string>{}(name);
        std::string newname = "ID" + std::to_string(hash);
#ifdef DBG
        std::cout << "DeclRefExpr name: "  << name << " Hash: " << hash << " New ID: " << newname << "\n";
#endif
      SourceLocation SL = Devi::getSLSpellingLoc(DRE->getNameInfo().getBeginLoc(), Rewrite);
      SourceLocation SLE = Devi::getSLSpellingLoc(DRE->getNameInfo().getEndLoc(), Rewrite);

      Rewrite.ReplaceText(SourceRange(SL, SLE), StringRef(newname));
      }
    }

  private:
    Rewriter &Rewrite;
};
/**********************************************************************************************************************/
/**
 * @brief changes the identifier for function declarations with the hash digest of the identifier being defined.
 */
class FuncDecl : public MatchFinder::MatchCallback
{
public:
  FuncDecl (Rewriter &Rewrite) : Rewrite (Rewrite) {}

  virtual void run(const MatchFinder::MatchResult &MR)
  {
    if (MR.Nodes.getNodeAs<clang::FunctionDecl>("funcdecl") != nullptr) {
      const FunctionDecl* FD = MR.Nodes.getNodeAs<clang::FunctionDecl>("funcdecl");
      std::string funcname = FD->getNameInfo().getAsString();
      if (funcname == "main") return void();
      std::size_t hash = std::hash<std::string>{}(funcname);
      std::string newname = "ID" + std::to_string(hash);
#ifdef DBG
      std::cout << "Function name: "  << funcname << " Hash: " << hash << " New ID: " << newname << "\n";
#endif

      SourceLocation SL = Devi::getSLSpellingLoc(FD->getNameInfo().getBeginLoc(), Rewrite);
      SourceLocation SLE = Devi::getSLSpellingLoc(FD->getNameInfo().getEndLoc(), Rewrite);

      Rewrite.ReplaceText(SourceRange(SL, SLE), StringRef(newname));
    }
  }

private:
  Rewriter &Rewrite;
};
/**********************************************************************************************************************/
/**
 * @brief changes the identifier for var declarations with the hash digest of the identifier being defined.
 */
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
#ifdef DBG
      std::cout << "Var name: "  << varname << " Hash: " << hash << " New ID: " << newname << "\n";
#endif
      SourceLocation SL = Devi::getSLSpellingLoc(VD->getLocation(), Rewrite);
      SourceLocation SLE;
      const clang::Expr* EXP = nullptr;

      // if the VarDecl also contains an initial value assignment.
      if (MR.Nodes.getNodeAs<clang::Expr>("expr") !=nullptr) {
        EXP = MR.Nodes.getNodeAs<clang::Expr>("expr");
        SLE = Devi::getSLSpellingLoc(EXP->getExprLoc(), Rewrite);
      } else {
        SLE = Devi::getSLSpellingLoc(VD->getLocEnd(), Rewrite);
      }

      //@devi-FIXME-cluncky
      Rewrite.ReplaceText(SourceRange(SL, SL.getLocWithOffset(VD->getIdentifier()->getLength() - 1)), StringRef(newname));
    }
  }

private:
  Rewriter &Rewrite;
};
/**********************************************************************************************************************/
/**
 * @brief Replaces the class name with its hash digest.
 */
class ClassDecl : public MatchFinder::MatchCallback {
  public:
    ClassDecl (Rewriter &Rewrite) : Rewrite(Rewrite) {}

    virtual void run(const MatchFinder::MatchResult &MR) {
      if (MR.Nodes.getNodeAs<clang::RecordDecl>("classdecl") != nullptr) {
        const RecordDecl* RD = MR.Nodes.getNodeAs<clang::RecordDecl>("classdecl");
        if (RD->isAnonymousStructOrUnion()) return void();
        if (RD->isInjectedClassName()) {}
        else {return void();}
        //auto TD = RD->getCanonicalDecl();
        std::string varname = RD->getIdentifier()->getName().str();
        std::size_t hash = std::hash<std::string>{}(varname);
        std::string newname = "ID" + std::to_string(hash);
#ifdef DBG
        std::cout << "Record name: "  << varname << " Hash: " << hash << " New ID: " << newname << "\n";
#endif

        SourceLocation SL = Devi::getSLSpellingLoc(RD->getLocation(), Rewrite);
        SourceLocation SLE = Devi::getSLSpellingLoc(RD->getLocEnd(), Rewrite);

        std::string dummy = Rewrite.getRewrittenText(SourceRange(SL, SLE));
        Rewrite.ReplaceText(SourceRange(SL, SLE), StringRef(newname));
      }
    }

  private:
    Rewriter &Rewrite;
};
/**********************************************************************************************************************/
/**
 * @brief PPCallbacks for replacing Macro identifiers with their hash digest Along with changing the Header filenames.
 */
class PPInclusion : public PPCallbacks
{
public:
  explicit PPInclusion (SourceManager *SM, Rewriter *Rewrite) : SM(*SM), Rewrite(*Rewrite) {}

  virtual void MacroDefined(const Token &MacroNameTok, const MacroDirective *MD) {
    SourceLocation SL = Devi::getSLSpellingLoc(MacroNameTok.getLocation(), Rewrite);
    if (!SM.isInMainFile(SL)) return void();
    if (!SM.isWrittenInMainFile(SL)) return void();
    CheckSLValidity(SL);
    std::string macroname = MacroNameTok.getIdentifierInfo()->getName().str();
    std::size_t hash = std::hash<std::string>{}(macroname);
    std::string newname = "ID" + std::to_string(hash);
#ifdef DBG
    std::cout << "Macro name: " << macroname << " Hash: " << hash << " New ID: " << newname << "\n";
#endif

#ifdef DBG
    std::string dummy = Rewrite.getRewrittenText(SourceRange(MacroNameTok.getLocation(), MacroNameTok.getLocation().getLocWithOffset(MacroNameTok.getLength())));
    std::cout << dummy << "\n";
#endif
    Rewrite.ReplaceText(SourceRange(SL, SL.getLocWithOffset(MacroNameTok.getLength() - 1)), newname);
  }

  virtual void MacroExpands (const Token &MacroNameTok, const MacroDefinition &MD, SourceRange Range, const MacroArgs *Args) {
    SourceLocation SL = Devi::getSLSpellingLoc(MacroNameTok.getLocation(), Rewrite);
    if (!SM.isInMainFile(SL)) return void();
    if (!SM.isWrittenInMainFile(SL)) return void();
    CheckSLValidity(SL);
    std::string macroname = MacroNameTok.getIdentifierInfo()->getName().str();
    std::size_t hash = std::hash<std::string>{}(macroname);
    std::string newname = "ID" + std::to_string(hash);

    Rewrite.ReplaceText(SourceRange(SL, SL.getLocWithOffset(MacroNameTok.getLength() - 1)), newname);
  }

  virtual void  InclusionDirective (SourceLocation HashLoc, const Token &IncludeTok, 
      StringRef FileName, bool IsAngled, CharSourceRange FilenameRange, const FileEntry *File, 
      StringRef SearchPath, StringRef RelativePath, const clang::Module *Imported) {
    std::cout << "Include filename: " << FileName.str() << "\n";
    // name, extension, path
    auto header_ = getNameFromPath(FileName.str());
    std::string hashedName_ = getHashedName(std::get<0>(header_));
    auto hashedNameX_ = nameMaker(hashedName_, std::get<1>(header_), "");

    auto SL = Devi::getSLSpellingLoc(FilenameRange.getBegin(), Rewrite);
    auto SLE = Devi::getSLSpellingLoc(FilenameRange.getEnd(), Rewrite);
    auto SR = SourceRange(SL, SLE);

    std::string new_name_;
    if (IsAngled) {new_name_ = "<" + hashedNameX_ + ">";}
    else {new_name_ = "\"" + hashedNameX_ + "\"";}

    Rewrite.ReplaceText(SR, new_name_);
#ifdef DBG
    std::cout << hashedNameX_ << "\n";
#endif
  }

private:
  const SourceManager &SM;
  Rewriter &Rewrite;
};
/**********************************************************************************************************************/
/**
 * @brief A Clang Diagnostic Consumer that does nothing.
 */
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
  MyASTConsumer(Rewriter &R) : funcDeclHandler(R), HandlerForVar(R), HandlerForClass(R), HandlerForCalledFunc(R), HandlerForCalledVar(R) {
#if 1
    Matcher.addMatcher(functionDecl().bind("funcdecl"), &funcDeclHandler);
    Matcher.addMatcher(varDecl(anyOf(unless(hasDescendant(expr(anything()))), hasDescendant(expr(anything()).bind("expr")))).bind("vardecl"), &HandlerForVar);
    Matcher.addMatcher(recordDecl(isClass()).bind("classdecl"), &HandlerForClass);
    Matcher.addMatcher(declRefExpr().bind("calledvar"), &HandlerForCalledVar);
#endif
  }

  void HandleTranslationUnit(ASTContext &Context) override {
    Matcher.matchAST(Context);
  }

private:
  FuncDecl funcDeclHandler;
  VDecl HandlerForVar;
  ClassDecl HandlerForClass;
  CalledFunc HandlerForCalledFunc;
  CalledVar HandlerForCalledVar;
  MatchFinder Matcher;
};
/**********************************************************************************************************************/
class ObfFrontendAction : public ASTFrontendAction {
public:
  ObfFrontendAction() {}
  ~ObfFrontendAction() {
    delete BDCProto;
    delete tee;
  }

  void EndSourceFileAction() override {
    std::error_code EC;
    std::string OutputFilename = TEMP_FILE;
    TheRewriter.getEditBuffer(TheRewriter.getSourceMgr().getMainFileID()).write(llvm::outs());
    tee = new raw_fd_ostream(StringRef(OutputFilename), EC, sys::fs::F_None);
    TheRewriter.getEditBuffer(TheRewriter.getSourceMgr().getMainFileID()).write(*tee);
  }

  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, StringRef file) override {
    CI.getPreprocessor().addPPCallbacks(llvm::make_unique<PPInclusion>(&CI.getSourceManager(), &TheRewriter));
    DiagnosticsEngine &DE = CI.getPreprocessor().getDiagnostics();
    DE.setClient(BDCProto, false);
    TheRewriter.setSourceMgr(CI.getSourceManager(), CI.getLangOpts());
    return llvm::make_unique<MyASTConsumer>(TheRewriter);
  }

private:
  BlankDiagConsumer* BDCProto = new BlankDiagConsumer;
  Rewriter TheRewriter;
  raw_ostream *tee = &llvm::outs();
};
/**********************************************************************************************************************/
/**
 * @brief Deletes all comment from source-code.
 */
class CommentWiper {
  public:
    CommentWiper(std::vector<std::string> SourceList) : sourcelist(SourceList) {}

    int run(void) {
      for (auto &filepath : sourcelist) {
        std::ifstream sourcefile;
        sourcefile.open(TEMP_FILE);
        std::ofstream dupe;
        auto filename_ = getNameFromPath(filepath);
        dupe.open(nameMaker(getHashedName(std::get<0>(filename_)), std::get<1>(filename_), ""));
        std::string line;

        int d_quote = 0;
        bool skip = false;

        while (std::getline(sourcefile, line)) {
          std::string dummy;
          line += "\n";
          for (unsigned int ch = 0; ch < line.length(); ++ch) {
            if (!skip) {
              /*@DEVI-FIXME-it coutns double qoutes and qoutes as one*/
              if ((line[ch] == '\"' || line[ch] == '\'')) {
                if (ch > 1) {
                  if (line[ch - 1] != '\\') {
                    d_quote++;
                    if (d_quote % 1 == 1) {
                      continue;
                    }
                  }
                }
              }
            }
            if (line[ch] == '/') {
              if (ch > 1) {
                if (line[ch - 1] == '/' && !skip) {
                  if (dummy.length() > 1) {
                    dummy.erase(dummy.length() - 1);
                  }
                  break;
                }
                if (line[ch - 1] == '*') {
                  skip = false;
                  continue;
                }
              }
              if (ch < line.length() - 1) {
                if (line[ch + 1] == '*' && !skip) {
                  skip = true;
                  continue;
                }
              }
            }
            if (!skip) dummy.push_back(line[ch]);
          }
          d_quote = 0;
          if (dummy.length() > 1) {
            if (dummy[dummy.length() - 1] != '\n') {dummy.push_back('\n');}
          }
          dupe << dummy;
          dummy.clear();
        }
        sourcefile.close();
        dupe.close();
      }
      return 0;
    }

  private:
    std::vector<std::string> sourcelist;
};
/**********************************************************************************************************************/
/*@DEVI-not in a hurry to implement this. reverting this is as simple as running something like clang-format.*/
class WhitespaceWarper {
  public:
    WhitespaceWarper(std::vector<std::string> SourceList) : sourcelist(SourceList) {}

    int run(void) {
      for (auto &filepath : sourcelist) {
        std::ifstream sourcefile;
        sourcefile.open(TEMP_FILE);
        auto filename_ = getNameFromPath(filepath);
        std::ofstream dupe;
        dupe.open("./dupe2.cpp");
        std::string line;
        while (std::getline(sourcefile, line)) {
          for (auto &character : line) {
            if (std::to_string(character) == "\t") {
              dupe << "\t";
            }
            else if (std::to_string(character) == "\n") {
            }
            else if (std::to_string(character) == " ") {
              dupe << " ";
            }
            else {
              dupe << character;
            }
          }
        }
        dupe.close();
      }
      return 0;
    }

  private:
    std::vector<std::string> sourcelist;
};
/**********************************************************************************************************************/
/**********************************************************************************************************************/
/*Main*/
int main(int argc, const char **argv) {
#if 1
  CommonOptionsParser op(argc, argv, ObfuscatorCat);
  const std::vector<std::string> &SourcePathList = op.getSourcePathList();
  ClangTool Tool(op.getCompilations(), op.getSourcePathList());
  int ret = Tool.run(newFrontendActionFactory<ObfFrontendAction>().get());
  CommentWiper CW(SourcePathList);
  CW.run();
  dumpHashFilenames(hashFilenames(SourcePathList));
  dumpDirList(listDirs("./test"));
#endif
#if 0
  for (auto &iter : SourcePathList) {
    std::cout << "name: " << std::get<0>(getNameFromPath(iter)) << "\t" << "extension: " << std::get<1>(getNameFromPath(iter)) << "\tpath: " << std::get<2>(getNameFromPath(iter)) << "\n";
  }
#endif
  return ret;
}
/**********************************************************************************************************************/
/*last line intentionally left blank.*/

