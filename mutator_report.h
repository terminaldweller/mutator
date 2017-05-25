
/***************************************************Project Mutator****************************************************/
//-*-c++-*-
/*********************************************************************************************************************/
/*first line intentionally left blank*/
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
/*inclusion guard*/
#ifndef MUTATOR_REPORT_H
#define MUTATOR_REPORT_H
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
/*********************************************************************************************************************/
  class XMLReportBase
  {
    public:
    XMLReportBase();
    virtual ~XMLReportBase();

    void CreateReport(void);

    virtual void AddNode(void) = 0;

    void SaveReport(const char*);

    protected:
    XMLDocument Doc;
    XMLElement* RootPointer;
  };
/*********************************************************************************************************************/
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
    ~XMLReport();

    void XMLCreateReport(void);
    void XMLAddNode(ASTContext* ASTC, SourceLocation SL, std::string MisraRule, std::string Description);
    /*overloaded for rule checks that announce the result on onendoftranslation instead of run
    since they dont have access to matchresult or astcontext.*/
    void XMLAddNode(FullSourceLoc FSL, SourceLocation SL, std::string MisraRule, std::string Description);
    /*another overload to support the xml output for PPCallbacks.*/
    void XMLAddNode(const SourceManager &SM, SourceLocation SL, std::string MisraRule, std::string Description);

    void XMLAddNode(std::string FilePath, std::string MisraRule, std::string Description);

    void XMLAddNode(unsigned Line, unsigned Column, std::string FileName, std::string MisraRule, std::string Description);

    bool isReportEmpty(void);

    void SaveReport(void);

  private:
    XMLDocument XMLReportDoc;
    XMLElement* RootPointer;
  };
/*********************************************************************************************************************/
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

    void JSONAddElement(std::string FilePath, std::string MisraRule, std::string Description);

    void JSONAddElement(unsigned Line, unsigned Column, std::string FileName, std::string MisraRule, std::string Description);

    void CloseReport(void);

  private:
    std::ofstream JSONRepFile;
  };
/*********************************************************************************************************************/
/*********************************************************************************************************************/
} //end of namespace devi
#endif
/*********************************************************************************************************************/
/*last line intentionally left blank.*/

