
#include <string>
#include <iostream>
#include "clang/AST/AST.h"
#include "mutator_aux.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "tinyxml2/tinyxml2.h"

using namespace clang;
using namespace tinyxml2;

namespace Devi {
/*a simple function that checks the sourcelocations for a macro expansion. returns the sourcelocation without
macro expansion address.*/
SourceLocation SourceLocationHasMacro (SourceLocation SL, Rewriter &Rewrite, std::string Kind)
{
  /*does the sourcelocation include a macro expansion?*/
  if ( SL.isMacroID() )
  {
    /*get the expansion range which is startloc and endloc*/
    std::pair <SourceLocation, SourceLocation> expansionRange = Rewrite.getSourceMgr().getImmediateExpansionRange(SL);

    if (Kind == "start")
    {
      /*get the startloc.*/
      return (expansionRange.first);
    }
    else if (Kind == "end")
    {
      return (expansionRange.second);
    }
    else
    {
      std::cout << "the third argument of Devi::SourceLocationHasMacro is invalid." << std::endl;
    }

  }
  else
  {
    return (SL);
  }

  return (SL);
}
/*********************************************************************************************************************/
XMLReport::XMLReport()
{
  RootPointer = XMLReportDoc.NewElement("Report");
}

void XMLReport::XMLCreateReport(void)
{

  XMLReportDoc.InsertFirstChild(RootPointer);
}

/*it is the caller's responsibility to make sure the sourcelocation passed to this member function
contains only the spelling location.*/
void XMLReport::XMLAddNode(ASTContext* ASTC, SourceLocation SL, std::string MisraRule, std::string Description)
{
  FullSourceLoc FSL = ASTC->getFullLoc(SL);

  unsigned LineNumber = FSL.getSpellingLineNumber();
  unsigned ColumnNumber = FSL.getSpellingColumnNumber();

  const SourceManager& SM = FSL.getManager();
  std::string FileNameString = SM.getFilename(SL).str();

  XMLElement* MisraElement = XMLReportDoc.NewElement("MisraDiag");
  MisraElement->SetText(Description.c_str());
  MisraElement->SetAttribute("Misra-C:2004Rule", MisraRule.c_str());
  MisraElement->SetAttribute("FileName", FileNameString.c_str());
  MisraElement->SetAttribute("SpellingLineNumber", LineNumber);
  MisraElement->SetAttribute("SpellingColumnNumber", ColumnNumber);
  RootPointer->InsertEndChild(MisraElement);
}

void XMLReport::XMLAddNode(FullSourceLoc FullSrcLoc, SourceLocation SL, std::string MisraRule, std::string Description)
{
  unsigned LineNumber = FullSrcLoc.getSpellingLineNumber();
  unsigned ColumnNumber = FullSrcLoc.getSpellingColumnNumber();

  const SourceManager& SM = FullSrcLoc.getManager();
  std::string FileNameString = SM.getFilename(SL).str();

  XMLElement* MisraElement = XMLReportDoc.NewElement("MisraDiag");
  MisraElement->SetText(Description.c_str());
  MisraElement->SetAttribute("Misra-C:2004Rule", MisraRule.c_str());
  MisraElement->SetAttribute("FileName", FileNameString.c_str());
  MisraElement->SetAttribute("SpellingLineNumber", LineNumber);
  MisraElement->SetAttribute("SpellingColumnNumber", ColumnNumber);
  RootPointer->InsertEndChild(MisraElement);
}

void XMLReport::SaveReport(void)
{
  XMLError XMLErrorResult = XMLReportDoc.SaveFile("/home/bloodstalker/devi/hell2/test/misrareport.xml");

  if (XMLErrorResult != XML_SUCCESS)
  {
    std::cout << "could not write xml misra report." << std::endl;
  }
}
/*********************************************************************************************************************/
}