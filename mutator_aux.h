
/*********************************************************************************************************************/
/*first line intentionally left blank*/
/*********************************************************************************************************************/
/*inclusion guard*/
#ifndef MUTATOR_AUX_H
#define MUTATOR_AUX_H
/*********************************************************************************************************************/
/*inclusion directives*/
#include <string>
#include <fstream>
#include "clang/AST/AST.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "tinyxml2/tinyxml2.h"
/*********************************************************************************************************************/
using namespace clang;
using namespace tinyxml2;
/*********************************************************************************************************************/
namespace Devi {
SourceLocation SourceLocationHasMacro (SourceLocation SL, Rewriter &Rewrite, std::string Kind);

bool IsTheMatchInSysHeader(bool SysHeaderFlag, const ast_matchers::MatchFinder::MatchResult &MR, SourceLocation SL);

bool IsTheMatchInSysHeader(bool SysHeaderFlag, const SourceManager &SM, SourceLocation SL);

bool IsTheMatchInSysHeader(bool SysHeaderFlag, bool SysHeader, SourceLocation SL);

bool IsTheMatchInMainFile(bool MainFileFlag, const ast_matchers::MatchFinder::MatchResult &MR, SourceLocation SL);

bool IsTheMatchInMainFile(bool MainFileFlag, const SourceManager &SM, SourceLocation SL);

bool IsTheMatchInMainFile(bool MainFileFlag, bool MainFile, SourceLocation SL);

/*@DEVI- for both report classes, if the program gets terminated, since the destructor does not close
the report files, what happens to them is implementation-defined in case of let's say an exit, but since
we erase the files everytime a new instance of mutator-lvl0 is called, we are fine. or so i think.*/
/*@DEVI- in case of a crash, the XML report will only hold the base node, while the JSON report will
contain all the reports up until the crash. tinyxml2 writes the nodes to file on SaveFile which is
called in SaveReport so that's why.*/
class XMLReport
{
public:
  XMLReport();

  void XMLCreateReport(void);
  void XMLAddNode(ASTContext* ASTC, SourceLocation SL, std::string MisraRule, std::string Description);
  /*overloaded for rule checks that announce the result on onendoftranslation instead of run
  since they dont have access to matchresult or astcontext.*/
  void XMLAddNode(FullSourceLoc FSL, SourceLocation SL, std::string MisraRule, std::string Description);
  /*another overload to support the xml output for PPCallbacks.*/
  void XMLAddNode(const SourceManager &SM, SourceLocation SL, std::string MisraRule, std::string Description);
  void SaveReport(void);

private:
  XMLDocument XMLReportDoc;
  XMLNode* RootPointer;
};

class JSONReport
{
public:
  JSONReport();

  void JSONCreateReport(void);
  void JSONAddElement(ASTContext* ASTC, SourceLocation SL, std::string MisraRule, std::string Description);
  /*overload for checks that announce the result in onendoftranslation unit.*/
  void JSONAddElement(FullSourceLoc FSL, SourceLocation SL, std::string MisraRule, std::string Description);
  /*overload for PPCallbacks.*/
  void JSONAddElement(const SourceManager &SM, SourceLocation SL, std::string MisraRule, std::string Description);
  void CloseReport(void);

private:
  std::ofstream JSONRepFile;
};
/*********************************************************************************************************************/
/*end of namespace Devi*/
}

#endif
/*********************************************************************************************************************/
/*last line intentionally left blank.*/
