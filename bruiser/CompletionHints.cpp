
/***************************************************Project Mutator****************************************************/
//-*-c++-*-
/*first line intentionally left blank.*/
/*the source code for bruiser's auto-completion and suggestions.*/
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
#include "bruiser-extra.h"
/*project headers*/
/*standard headers*/
#include <iostream>
#include <string>
/*LLVM headers*/
/*other*/
#include "linenoise/linenoise.h"
/**********************************************************************************************************************/
/*used namespaces*/
/**********************************************************************************************************************/
/*macros*/

/**********************************************************************************************************************/
namespace bruiser
{
  void ShellCompletion(const char* __buf, linenoiseCompletions* __lc)
  {
    if (__buf != NULL)
    {
      for(auto &iter : LUA_FUNCS)
      {
        if (iter.find(__buf) == 0U)
        {
          linenoiseAddCompletion(__lc, iter.c_str());
        }
      }
    }
  }

  char* ShellHints(const char* __buf, int* __color, int* __bold)
  {
    if (__buf != NULL)
    {
      auto dummy = std::string(__buf);

      for(auto &iter : LUA_FUNCS)
      {
        if (dummy == "")
        {
          break;
        }

        if (iter.find(__buf) == 0U)
        {
          *__color = 35;
          *__bold = 1;
          int sizet = dummy.length();

          std::string dummy2 = iter.substr(sizet, std::string::npos);

          /*@DEVI-apparently linenoise expects the return value to live past the hints callback function returning, 
           * i mean that's why our vector version returns junk. linenoise later frees the returned hint so there 
           * should be no leaked memory(it calls freeHintsCallback).*/
          char* returnchar = new char[dummy2.size() + 1];
          std::copy(dummy2.begin(), dummy2.end(), returnchar);
          returnchar[dummy2.size()] = '\0';

#if 0
          std::vector<char> retchar(dummy2.begin(), dummy2.end());
          retchar.push_back('\0');
          //std::cout << "\n" << retchar.data() << "\n";
          char* c = (char*)retchar.data();
          std::cout << "\n" << c << "\n";
#endif

          return returnchar;
          //return c;
          //return retchar.data();
          //return &retchar[0];
        }
      }
    }

    return NULL;
  }
} //end of namespace bruiser
/**********************************************************************************************************************/
/*last line intentionally left blank*/

