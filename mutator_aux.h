
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
#include "clang/Basic/SourceManager.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "tinyxml2/tinyxml2.h"
/*********************************************************************************************************************/
using namespace clang;
using namespace tinyxml2;
/*********************************************************************************************************************/
namespace Devi {
SourceLocation SourceLocationHasMacro (SourceLocation SL, Rewriter &Rewrite, std::string Kind);

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

}

#endif
/*********************************************************************************************************************/
/*last line intentionally left blank.*/
