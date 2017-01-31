
/*first line intentionally left blank.*/
/*********************************************************************************************************************/
/*inclusion directives*/
#include "mutator_aux.h"
#include <string>
#include <cassert>
#include <iostream>
#include <fstream>
#include "clang/AST/AST.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "tinyxml2/tinyxml2.h"
#include "json/json.hpp"
/*********************************************************************************************************************/
using namespace clang;
using namespace tinyxml2;
using json = nlohmann::json;
/*********************************************************************************************************************/
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
/*********************************************************************************************************************/
/*********************************************************************************************************************/
/*the first argument is the option SysHeader from the mutator-lvl0 cl.*/
bool IsTheMatchInSysHeader(bool SysHeaderFlag, const ast_matchers::MatchFinder::MatchResult &MR, SourceLocation SL)
{
  ASTContext *const ASTC = MR.Context;
  const SourceManager &SM = ASTC->getSourceManager();

  if (SM.isInSystemHeader(SL) && !SysHeaderFlag)
  {
    return true;
  }
  else
  {
    return false;
  }
}

bool IsTheMatchInSysHeader(bool SysHeaderFlag, const SourceManager &SM, SourceLocation SL)
{
  if (SM.isInSystemHeader(SL) && !SysHeaderFlag)
  {
    return true;
  }
  else
  {
    return false;
  }
}

bool IsTheMatchInSysHeader(bool SysHeaderFlag, bool SysHeader, SourceLocation SL)
{
  if (SysHeader && !SysHeaderFlag)
  {
    return true;
  }
  else
  {
    return false;
  }
}

bool IsTheMatchInSysHeader(bool SysHeaderFlag, bool SysHeader)
{
  if (SysHeader && !SysHeaderFlag)
  {
    return true;
  }
  else
  {
    return false;
  }
}
/*********************************************************************************************************************/
/*********************************************************************************************************************/
/*********************************************************************************************************************/
bool IsTheMatchInMainFile(bool MainFileFlag, const ast_matchers::MatchFinder::MatchResult &MR, SourceLocation SL)
{
  ASTContext *const ASTC = MR.Context;
  const SourceManager &SM = ASTC->getSourceManager();

  if (SM.isInMainFile(SL) || (!SM.isInMainFile(SL) && !MainFileFlag))
  {
    return true;
  }
  else
  {
    return false;
  }
}

bool IsTheMatchInMainFile(bool MainFileFlag, const SourceManager &SM, SourceLocation SL)
{
  if (SM.isInMainFile(SL) || (!SM.isInMainFile(SL) && !MainFileFlag))
  {
    return true;
  }
  else
  {
    return false;
  }
}

bool IsTheMatchInMainFile(bool MainFileFlag, bool MainFile, SourceLocation SL)
{
  if (MainFile || (!MainFile && !MainFileFlag))
  {
    return true;
  }
  else
  {
    return false;
  }
}

bool IsTheMatchInMainFile(bool MainFileFlag, bool MainFile)
{
  if (MainFile || (!MainFile && !MainFileFlag))
  {
    return true;
  }
  else
  {
    return false;
  }
}
/*********************************************************************************************************************/
/*********************************************************************************************************************/
/******************************************************XMLReport******************************************************/
XMLReport::XMLReport()
{
  RootPointer = XMLReportDoc.NewElement("mutator:Report");
#if 1
  RootPointer->SetAttribute("xmlns:mutator", "http://www.w3.org/2001/XMLSchema");
#endif
}

void XMLReport::XMLCreateReport()
{
  XMLReportDoc.InsertFirstChild(RootPointer);
}

/*it is the caller's responsibility to make sure the sourcelocation passed to this overload of the member function
contains only the spelling location.*/
void XMLReport::XMLAddNode(ASTContext* ASTC, SourceLocation SL, std::string MisraRule, std::string Description)
{
  //assert(SL.isValid() && "SourceLocation passed as function parameter in an overload(1) of XMLAddNode is not valid.");

  FullSourceLoc FSL = ASTC->getFullLoc(SL);

  unsigned LineNumber = FSL.getSpellingLineNumber();
  unsigned ColumnNumber = FSL.getSpellingColumnNumber();

  const SourceManager& SM = FSL.getManager();
  std::string FileNameString = SM.getFilename(SL).str();

  XMLElement* MisraElement = XMLReportDoc.NewElement("MisraDiag");
  MisraElement->SetText(Description.c_str());
  MisraElement->SetAttribute("Misra-C-2004Rule", MisraRule.c_str());
  MisraElement->SetAttribute("FileName", FileNameString.c_str());
  MisraElement->SetAttribute("SpellingLineNumber", LineNumber);
  MisraElement->SetAttribute("SpellingColumnNumber", ColumnNumber);
  RootPointer->InsertEndChild(MisraElement);
}

void XMLReport::XMLAddNode(FullSourceLoc FullSrcLoc, SourceLocation SL, std::string MisraRule, std::string Description)
{
  //assert(SL.isValid() && "SourceLocation passed as function parameter in an overload(2) of XMLAddNode is not valid.");

  unsigned LineNumber = FullSrcLoc.getSpellingLineNumber();
  unsigned ColumnNumber = FullSrcLoc.getSpellingColumnNumber();

  const SourceManager& SM = FullSrcLoc.getManager();
  std::string FileNameString = SM.getFilename(SL).str();

  XMLElement* MisraElement = XMLReportDoc.NewElement("MisraDiag");
  MisraElement->SetText(Description.c_str());
  MisraElement->SetAttribute("Misra-C-2004Rule", MisraRule.c_str());
  MisraElement->SetAttribute("FileName", FileNameString.c_str());
  MisraElement->SetAttribute("SpellingLineNumber", LineNumber);
  MisraElement->SetAttribute("SpellingColumnNumber", ColumnNumber);
  RootPointer->InsertEndChild(MisraElement);
}

void XMLReport::XMLAddNode(const SourceManager &SM, SourceLocation SL, std::string MisraRule, std::string Description)
{
  SL = SM.getSpellingLoc(SL);

  //assert(SL.isValid() && "SourceLocation Acquired by SourceManager in an overload(3) of XMLAddNode is not valid.");

  unsigned LineNumber = SM.getSpellingLineNumber(SL);
  unsigned ColumnNumber = SM.getSpellingColumnNumber(SL);

  std::string FileNameString = SM.getFilename(SL).str();

  XMLElement* MisraElement = XMLReportDoc.NewElement("MisraDiag");
  MisraElement->SetText(Description.c_str());
  MisraElement->SetAttribute("Misra-C-2004Rule", MisraRule.c_str());
  MisraElement->SetAttribute("FileName", FileNameString.c_str());
  MisraElement->SetAttribute("SpellingLineNumber", LineNumber);
  MisraElement->SetAttribute("SpellingColumnNumber", ColumnNumber);
  RootPointer->InsertEndChild(MisraElement);
}

void XMLReport::XMLAddNode(std::string FilePath, std::string MisraRule, std::string Description)
{
  //assert(SL.isValid() && "SourceLocation Acquired by SourceManager in an overload(3) of XMLAddNode is not valid.");

  XMLElement* MisraElement = XMLReportDoc.NewElement("MisraDiag");
  MisraElement->SetText(Description.c_str());
  MisraElement->SetAttribute("Misra-C-2004Rule", MisraRule.c_str());
  MisraElement->SetAttribute("FileName", FilePath.c_str());
  RootPointer->InsertEndChild(MisraElement);
}

void XMLReport::XMLAddNode(unsigned Line, unsigned Column, std::string FileName, std::string MisraRule, std::string Description)
{
  //assert(SL.isValid() && "SourceLocation Acquired by SourceManager in an overload(3) of XMLAddNode is not valid.");

  XMLElement* MisraElement = XMLReportDoc.NewElement("MisraDiag");
  MisraElement->SetText(Description.c_str());
  MisraElement->SetAttribute("Misra-C-2004Rule", MisraRule.c_str());
  MisraElement->SetAttribute("FileName", FileName.c_str());
  MisraElement->SetAttribute("SpellingLineNumber", Line);
  MisraElement->SetAttribute("SpellingColumnNumber", Column);
  RootPointer->InsertEndChild(MisraElement);
}

void XMLReport::SaveReport(void)
{
  XMLError XMLErrorResult = XMLReportDoc.SaveFile("./test/misrareport.xml");

  if (XMLErrorResult != XML_SUCCESS)
  {
    std::cout << "could not write xml misra report." << std::endl;
  }
}
/***************************************************End of XMLReport**************************************************/
/*********************************************************************************************************************/
/*****************************************************JSONReport******************************************************/
JSONReport::JSONReport() {}

void JSONReport::JSONCreateReport(void)
{
  JSONRepFile.open("./test/misrareport.json", std::ios::out);
}

void JSONReport::JSONAddElement(ASTContext* ASTC, SourceLocation SL, std::string MisraRule, std::string Description)
{
  //assert(SL.isValid() && "SourceLocation passed as function parameter in an overload(1) of JSONAddElement is not valid.");

  FullSourceLoc FSL = ASTC->getFullLoc(SL);

  unsigned LineNumber = FSL.getSpellingLineNumber();
  unsigned ColumnNumber = FSL.getSpellingColumnNumber();

  const SourceManager& SM = FSL.getManager();
  std::string FileNameString = SM.getFilename(SL).str();

  json RepJ;

  RepJ["MisraDiag"]["Description"] = Description.c_str();
  RepJ["MisraDiag"]["Misra-C-2004Rule"] = MisraRule.c_str();
  RepJ["MisraDiag"]["FileName"] = FileNameString.c_str();
  RepJ["MisraDiag"]["SpellingLineNumber"] = LineNumber;
  RepJ["MisraDiag"]["SpellingColumnNumber"] = ColumnNumber;

  JSONRepFile << RepJ << std::endl;
}

void JSONReport::JSONAddElement(FullSourceLoc FullSrcLoc, SourceLocation SL, std::string MisraRule, std::string Description)
{
  //assert(SL.isValid() && "SourceLocation passed as function parameter in an overload(2) of XMLAddNode is not valid.");

  unsigned LineNumber = FullSrcLoc.getSpellingLineNumber();
  unsigned ColumnNumber = FullSrcLoc.getSpellingColumnNumber();

  const SourceManager& SM = FullSrcLoc.getManager();
  std::string FileNameString = SM.getFilename(SL).str();

  json RepJ;

  RepJ["MisraDiag"]["Description"] = Description.c_str();
  RepJ["MisraDiag"]["Misra-C-2004Rule"] = MisraRule.c_str();
  RepJ["MisraDiag"]["FileName"] = FileNameString.c_str();
  RepJ["MisraDiag"]["SpellingLineNumber"] = LineNumber;
  RepJ["MisraDiag"]["SpellingColumnNumber"] = ColumnNumber;

  JSONRepFile << RepJ << std::endl;
}

void JSONReport::JSONAddElement(const SourceManager &SM, SourceLocation SL, std::string MisraRule, std::string Description)
{
  SL = SM.getSpellingLoc(SL);

  //assert(SL.isValid() && "SourceLocation Acquired by SourceManager in an overload(3) of XMLAddNode is not valid.");

  unsigned LineNumber = SM.getSpellingLineNumber(SL);
  unsigned ColumnNumber = SM.getSpellingColumnNumber(SL);

  std::string FileNameString = SM.getFilename(SL).str();

  json RepJ;

  RepJ["MisraDiag"]["Description"] = Description.c_str();
  RepJ["MisraDiag"]["Misra-C-2004Rule"] = MisraRule.c_str();
  RepJ["MisraDiag"]["FileName"] = FileNameString.c_str();
  RepJ["MisraDiag"]["SpellingLineNumber"] = LineNumber;
  RepJ["MisraDiag"]["SpellingColumnNumber"] = ColumnNumber;

  JSONRepFile << RepJ << std::endl;
}

void JSONReport::JSONAddElement(std::string FilePath, std::string MisraRule, std::string Description)
{
  //assert(SL.isValid() && "SourceLocation Acquired by SourceManager in an overload(3) of XMLAddNode is not valid.");

  json RepJ;

  RepJ["MisraDiag"]["Description"] = Description.c_str();
  RepJ["MisraDiag"]["Misra-C-2004Rule"] = MisraRule.c_str();
  RepJ["MisraDiag"]["FileName"] = FilePath.c_str();


  JSONRepFile << RepJ << std::endl;
}

void JSONReport::JSONAddElement(unsigned Line, unsigned Column, std::string FileName, std::string MisraRule, std::string Description)
{
  //assert(SL.isValid() && "SourceLocation Acquired by SourceManager in an overload(3) of XMLAddNode is not valid.");

  json RepJ;

  RepJ["MisraDiag"]["Description"] = Description.c_str();
  RepJ["MisraDiag"]["Misra-C-2004Rule"] = MisraRule.c_str();
  RepJ["MisraDiag"]["FileName"] = FileName.c_str();
  RepJ["MisraDiag"]["SpellingLineNumber"] = Line;
  RepJ["MisraDiag"]["SpellingColumnNumber"] = Column;

  JSONRepFile << RepJ << std::endl;
}

void JSONReport::CloseReport(void)
{
  JSONRepFile.close();
}
/*********************************************************************************************************************/
/****************************************************End Of JSONReport************************************************/
/*********************************************************************************************************************/
/*End of namespace Devi*/
}
/*********************************************************************************************************************/
/*last line intentionally left blank.*/
