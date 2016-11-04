
#include <string>
#include <iostream>
#include "clang/AST/AST.h"
#include "mutator_aux.h"
#include "clang/Rewrite/Core/Rewriter.h"

using namespace clang;

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
}
}