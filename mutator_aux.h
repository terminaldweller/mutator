
#ifndef MUTATOR_AUX_H
#define MUTATOR_AUX_H

#include <string>
#include "clang/AST/AST.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "tinyxml2/tinyxml2.h"

using namespace clang;
using namespace tinyxml2;

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
  void SaveReport(void);

private:
  XMLDocument XMLReportDoc;
  XMLNode* RootPointer;
};

}

#endif