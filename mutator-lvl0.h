
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
/*code structure inspired by Eli Bendersky's tutorial on Rewriters.*/
/**********************************************************************************************************************/
/*inclusion guard*/
#ifndef MUTATOR_0_H
#define MUTATOR_0_H
/**********************************************************************************************************************/
/*included modules*/
/*project headers*/
#include "mutator_report.h"
/*standard library headers*/
#include <iostream>
#include <map>
#include <string>
#include <cassert>
#include <vector>
#include <unordered_map>
/*clang headers*/
#include "clang/AST/ASTContext.h"
#include "clang/AST/ASTTypeTraits.h"
/**********************************************************************************************************************/
/*externals*/
/**********************************************************************************************************************/
struct WeakPoint
{
  WeakPoint(std::string  __psft, std::string __file, unsigned int __ln, unsigned int __cn) 
  {
    PlaceHolderStringForType = __psft;
    File = __file;
    LineNumber = __ln;
    ColumnNumber = __cn;
  }

  std::string PlaceHolderStringForType;
  std::string File;
  unsigned int LineNumber;
  unsigned int ColumnNumber;
};
/**********************************************************************************************************************/
std::map<std::string,bool> MC2OptsMap = {
  {"1.1", false},
  {"1.2", false},
  {"1.3", false},
  {"1.4", false},
  {"1.5", false},
  {"2.1", false},
  {"2.2", false},
  {"2.3", false},
  {"2.4", false},
  {"3.1", false},
  {"3.2", false},
  {"3.3", false},
  {"3.4", false},
  {"3.5", false},
  {"3.6", false},
  {"4.1", false},
  {"4.2", false},
  {"5.1", false},
  {"5.2", false},
  {"5.3", false},
  {"5.4", false},
  {"5.5", false},
  {"5.6", false},
  {"5.7", false},
  {"6.1", false},
  {"6.2", false},
  {"6.3", false},
  {"6.4", false},
  {"6.5", false},
  {"7.1", false},
  {"8.1", false},
  {"8.2", false},
  {"8.3", false},
  {"8.4", false},
  {"8.5", false},
  {"8.6", false},
  {"8.7", false},
  {"8.8", false},
  {"8.9", false},
  {"8.10", false},
  {"8.11", false},
  {"8.12", false},
  {"9.1", false},
  {"9.2", false},
  {"9.3", false},
  {"10.1", false},
  {"10.2", false},
  {"10.3", false},
  {"10.4", false},
  {"10.5", false},
  {"10.6", false},
  {"11.1", false},
  {"11.2", false},
  {"11.3", false},
  {"11.4", false},
  {"11.5", false},
  {"12.1", false},
  {"12.2", false},
  {"12.3", false},
  {"12.4", false},
  {"12.5", false},
  {"12.6", false},
  {"12.7", false},
  {"12.8", false},
  {"12.9", false},
  {"12.10", false},
  {"12.11", false},
  {"12.12", false},
  {"12.13", false},
  {"13.1", false},
  {"13.2", false},
  {"13.3", false},
  {"13.4", false},
  {"13.5", false},
  {"13.6", false},
  {"13.7", false},
  {"14.1", false},
  {"14.2", false},
  {"14.3", false},
  {"14.4", false},
  {"14.5", false},
  {"14.6", false},
  {"14.7", false},
  {"14.8", false},
  {"14.9", false},
  {"14.10", false},
  {"15.0", false},
  {"15.1", false},
  {"15.2", false},
  {"15.3", false},
  {"15.4", false},
  {"15.5", false},
  {"16.1", false},
  {"16.2", false},
  {"16.3", false},
  {"16.4", false},
  {"16.5", false},
  {"16.6", false},
  {"16.7", false},
  {"16.8", false},
  {"16.9", false},
  {"16.10", false},
  {"17.1", false},
  {"17.2", false},
  {"17.3", false},
  {"17.4", false},
  {"17.5", false},
  {"17.6", false},
  {"18.1", false},
  {"18.2", false},
  {"18.3", false},
  {"18.4", false},
  {"19.1", false},
  {"19.2", false},
  {"19.3", false},
  {"19.4", false},
  {"19.5", false},
  {"19.6", false},
  {"19.7", false},
  {"19.8", false},
  {"19.9", false},
  {"19.10", false},
  {"19.11", false},
  {"19.12", false},
  {"19.13", false},
  {"19.14", false},
  {"19.15", false},
  {"19.16", false},
  {"19.17", false},
  {"20.1", false},
  {"20.2", false},
  {"20.3", false},
  {"20.4", false},
  {"20.5", false},
  {"20.6", false},
  {"20.7", false},
  {"20.8", false},
  {"20.9", false},
  {"20.10", false},
  {"20.11", false},
  {"20.12", false},
  {"21.1", false}
};

std::multimap<std::string,std::string> MC1EquivalencyMap = {
  {"1","1.1"},
  {"1","1.2"},
  {"1","2.2"},
  {"1","3.1"},
  {"2","1.3"},
  {"3","2.1"},
  {"4","21.1"},
  {"5","4.1"},
  {"6","3.2"},
  {"7","4.2"},
  {"8","rsc"},
  {"9","2.3"},
  {"10","2.4"},
  {"11","1.4"},
  {"12","5.5"},
  {"12","5.6"},
  {"12","5.7"},
  {"13","6.3"},
  {"14","6.1"},
  {"14","6.2"},
  {"15","1.5"},
  {"16","12.12"},
  {"17","5.3"},
  {"18","rsc"},
  {"19","7.1"},
  {"20","rsc"},
  {"21","5.2"},
  {"22","8.7"},
  {"23","8.10"},
  {"24","8.11"},
  {"25","8.9"},
  {"26","8.4"},
  {"27","8.8"},
  {"28","rsc"},
  {"29","5.4"},
  {"30","9.1"},
  {"31","9.2"},
  {"32","9.3"},
  {"33","12.4"},
  {"34","12.5"},
  {"35","13.1"},
  {"36","12.6"},
  {"37","10.5"},
  {"37","12.7"},
  {"38","12.8"},
  {"39","12.9"},
  {"40","12.3"},
  {"41","3.3"},
  {"42","12.10"},
  {"43","10.1"},
  {"44","rsc"},
  {"45","11.1"},
  {"45","11.2"},
  {"45","11.3"},
  {"45","11.4"},
  {"45","11.5"},
  {"46","12.2"},
  {"47","12.1"},
  {"48","10.4"},
  {"49","13.2"},
  {"50","13.3"},
  {"51","12.11"},
  {"52","14.1"},
  {"53","14.2"},
  {"54","14.3"},
  {"55","rsc"},
  {"56","14.4"},
  {"57","14.5"},
  {"58","rsc"},
  {"59","14.8"},
  {"59","14.9"},
  {"60","14.10"},
  {"61","15.1"},
  {"61","15.2"},
  {"62","15.3"},
  {"63","15.4"},
  {"64","15.5"},
  {"65","13.4"},
  {"66","13.5"},
  {"67","13.6"},
  {"68","8.6"},
  {"69","16.1"},
  {"70","16.2"},
  {"71","8.1"},
  {"72","8.3"},
  {"73","16.3"},
  {"74","16.4"},
  {"75","8.2"},
  {"76","16.5"},
  {"77","10.2"},
  {"78","16.6"},
  {"79","rsc"},
  {"80","rsc"},
  {"81","16.7"},
  {"82","14.7"},
  {"83","16.8"},
  {"84","rsc"},
  {"85","16.9"},
  {"86","16.10"},
  {"87","8.5"},
  {"87","19.1"},
  {"88","19.2"},
  {"89","19.3"},
  {"90","19.4"},
  {"91","19.5"},
  {"92","19.6"},
  {"93","19.7"},
  {"94","19.8"},
  {"95","19.9"},
  {"96","19.10"},
  {"97","19.11"},
  {"98","19.12"},
  {"98","19.13"},
  {"99","3.4"},
  {"100","19.14"},
  {"101","17.1"},
  {"101","17.2"},
  {"101","17.4"},
  {"102","17.5"},
  {"103","17.3"},
  {"104","rsc"},
  {"105","rsc"},
  {"106","17.6"},
  {"107","rsc"},
  {"108","18.1"},
  {"109","18.2"},
  {"109","18.3"},
  {"110","18.4"},
  {"111","6.4"},
  {"112","6.5"},
  {"113","rsc"},
  {"114","20.1"},
  {"115","20.2"},
  {"116","3.6"},
  {"117","20.3"},
  {"118","20.4"},
  {"119","20.5"},
  {"120","20.6"},
  {"121","rsc"},
  {"122","20.7"},
  {"123","20.8"},
  {"124","20.9"},
  {"125","20.10"},
  {"126","20.11"},
  {"127","20.12"}
};

std::pair<std::string, bool> Mutator0RuleChecks;

std::multimap<std::string, std::string> MC3EquivalencyMap;

std::unordered_map<std::string, std::string> SaferCPPEquivalencyMap;
/**********************************************************************************************************************/
class MutatorLVL0Tests
{
  public:
    MutatorLVL0Tests() {}

    void run(void)
    {

    }

  private:

};
/**********************************************************************************************************************/
class mutagenAncestryReport// : public Devi::XMLReportBase
{
  public:
    mutagenAncestryReport(std::vector<std::vector<std::string>> __dss, std::vector<WeakPoint> __wps) : DoomedStrains(__dss), WeakPoints(__wps) 
    {
      RootPointer = Doc.NewElement("mutagen:Report");
      RootPointer->SetAttribute("xmlns:mutator", "http://www.w3.org/2001/XMLSchema");
    }

    ~mutagenAncestryReport() 
    {
      Doc.InsertEndChild(RootPointer);
    }

    virtual void AddNode(void)
    {
      XMLElement* MGene = Doc.NewElement("DoomedStrains");

      for (auto &iter : DoomedStrains)
      {
        XMLElement* NodeDoomedStrain = Doc.NewElement("DoomedStrain");

        for (auto &iterer : iter)
        {
          XMLElement* Child = Doc.NewElement("Strain");
          Child->SetText(iterer.c_str());
          NodeDoomedStrain->InsertEndChild(Child);
        }
        
        MGene->InsertEndChild(NodeDoomedStrain);
      }

      RootPointer->InsertEndChild(MGene);
    }

    void AddNodeWeakPoint(void)
    {
      XMLElement* WeakStrain = Doc.NewElement("WeakStrains");

      for (auto &iter : WeakPoints)
      {
        XMLElement* Child = Doc.NewElement("WeakStrain");
        Child->SetAttribute("WeakStrainType", iter.PlaceHolderStringForType.c_str());
        Child->SetAttribute("File", iter.File.c_str());
        Child->SetAttribute("LineNumber", iter.LineNumber);
        Child->SetAttribute("ColumnNumber", iter.ColumnNumber);
        WeakStrain->InsertEndChild(Child);
      }

      RootPointer->InsertEndChild(WeakStrain);
    }

#if 1
    void CreateReport()
    {
      Doc.InsertFirstChild(RootPointer);
    }

    void SaveReport(const char* __filename)
    {
      Doc.InsertEndChild(RootPointer);

      XMLError XMLErrorResult = Doc.SaveFile(__filename);

      if (XMLErrorResult != XML_SUCCESS)
      {
        std::cerr << "could not write xml misra report.\n";
      }
    }
#endif

  private:
    std::vector<std::vector<std::string>> DoomedStrains;
    std::vector<WeakPoint> WeakPoints;
#if 1
    XMLElement* RootPointer;
    XMLDocument Doc;
#endif
};
/**********************************************************************************************************************/
#define EXTRACT_MUTAGEN 

class MutagenExtraction
{
  public:
    MutagenExtraction() {}

    ~MutagenExtraction() {}

    void ExtractAncestry(clang::ast_type_traits::DynTypedNode __dtn, clang::ASTContext &__astx)
    {
      clang::ASTContext::DynTypedNodeList DNL = __astx.getParents(__dtn);

      /*FIXME-a LastStrain. obviously well end up losing some parents in cpp if we're just picking up the 
       * first parent from the list.*/
      LastStrain.push_back(DNL[0].getNodeKind().asStringRef().str());
      clang::ast_type_traits::DynTypedNode DTN = DNL[0];

      /*FIXME-what does getparents return when there are no more parents to return?*/
      while (DTN.getNodeKind().asStringRef().str() != "FunctionDecl")
      {
        DNL = __astx.getParents(DTN);
        DTN = DNL[0];
        LastStrain.push_back(DTN.getNodeKind().asStringRef().str());
      }

      MutantStrainsAncestry.push_back(LastStrain);
      LastStrain.clear();
    }

    void ExtractWeakPoints(SourceLocation __sl, SourceManager &__sm, std::string __type)
    {
      WeakPoint tmp = WeakPoint(__type, __sm.getFilename(__sl).str(), \
          __sm.getSpellingLineNumber(__sl), __sm.getSpellingColumnNumber(__sl));
      WeakPoints.push_back(tmp);
    }

    void ExtractWeakPoints(FullSourceLoc __fsl, SourceLocation __sl, std::string __type)
    {
      WeakPoint tmp = WeakPoint(__type, __fsl.getManager().getFilename(__sl).str(), \
          __fsl.getSpellingLineNumber(), __fsl.getSpellingColumnNumber());
      WeakPoints.push_back(tmp);
    }

    void ExtractWeakPoints(std::string __type, unsigned int __ln, unsigned int __cn, std::string __file)
    {
      WeakPoint tmp = WeakPoint(__type, __file, __ln, __cn);
      WeakPoints.push_back(tmp);
    }

    void DumpLast(void)
    {
      for (auto &iter : LastStrain)
      {
        std::cout << "DoomedStrain : " << iter << "\n";
      }
    }

    void DumpAll(void)
    {
      for (auto &iter : MutantStrainsAncestry)
      {
        for (auto &iterer : iter)
        {
          std::cout << "DoomedStrainAll : " << iterer << "\n";
        }

        std::cout << "\n";
      }
    }

    void XMLReportAncestry(void)
    {
      mutagenAncestryReport MAR(MutantStrainsAncestry, WeakPoints);
      MAR.CreateReport();
      MAR.AddNode();
      MAR.AddNodeWeakPoint();
      MAR.SaveReport("m0.xml");
    }

  private:
    std::vector<std::string> LastStrain;
    std::vector<std::vector<std::string>> MutantStrainsAncestry;
    std::vector<WeakPoint> WeakPoints;
};
/**********************************************************************************************************************/
#endif
/**********************************************************************************************************************/
/*last line intentionally left blank*/

