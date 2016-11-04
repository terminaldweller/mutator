
#ifndef MUTATOR_AUX_H
#define MUTATOR_AUX_H

#include <string>
#include "clang/AST/AST.h"
#include "clang/Rewrite/Core/Rewriter.h"

using namespace clang;

namespace Devi {
SourceLocation SourceLocationHasMacro (SourceLocation SL, Rewriter &Rewrite, std::string Kind);
}

#endif