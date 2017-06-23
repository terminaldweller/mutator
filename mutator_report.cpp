
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
/*********************************************************************************************************************/
/*inclusion directives*/
#include "mutator_report.h"
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
/*********************************************************************************************************************/
/*********************************************************************************************************************/
/****************************************************XMLReportBase****************************************************/
  XMLReportBase::XMLReportBase()
  {
    RootPointer = Doc.NewElement("mutagen:Report");
    RootPointer->SetAttribute("xmlns:mutator", "http://www.w3.org/2001/XMLSchema");
  }

  XMLReportBase::~XMLReportBase()
  {
    Doc.InsertEndChild(RootPointer);
  }

  void XMLReportBase::CreateReport()
  {
    Doc.InsertFirstChild(RootPointer);
  }

  void XMLReportBase::SaveReport(const char* __filename)
  {
    Doc.InsertEndChild(RootPointer);

    XMLError XMLErrorResult = Doc.SaveFile(__filename);

    if (XMLErrorResult != XML_SUCCESS)
    {
      std::cerr << "could not write xml misra report(base).\n";
    }
  }
/************************************************end of XMLReportBase*************************************************/
/*********************************************************************************************************************/
/******************************************************XMLReport******************************************************/
XMLReport::XMLReport()
{
  RootPointer = XMLReportDoc.NewElement("mutator:Report");
  RootPointer->SetAttribute("xmlns:mutator", "http://www.w3.org/2001/XMLSchema");
}

XMLReport::~XMLReport()
{
  XMLReportDoc.InsertEndChild(RootPointer);
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

bool XMLReport::isReportEmpty(void)
{
  return false;
}

void XMLReport::SaveReport(void)
{
  if(this->isReportEmpty())
  {
    return void();
  }

  XMLReportDoc.InsertEndChild(RootPointer);

  XMLError XMLErrorResult = XMLReportDoc.SaveFile("./misrareport.xml");

  if (XMLErrorResult != XML_SUCCESS)
  {
    std::cerr << "could not write xml misra report.\n";
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
/*************************************************End Of JSONReport***************************************************/
/*********************************************************************************************************************/

/*********************************************************************************************************************/
/*End of namespace Devi*/
/*********************************************************************************************************************/
}
/*********************************************************************************************************************/
/*last line intentionally left blank.*/

