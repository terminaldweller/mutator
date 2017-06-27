
/***************************************************Project Mutator****************************************************/
//-*-c++-*-
/*first line intentionally left blank.*/
/*a second way of running the mutants. experimental.*/
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
#ifndef ORC_MUTATION_H
#define ORC_MUTATION_H
#if 0
/**********************************************************************************************************************/
/*included modules*/
/*project headers*/
/*standard library headers*/
#include <vector>
/*clang headers*/
/*llvm headers*/
#include "llvm/ExecutionEngine/SectionMemoryManager.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/RTDyldMemoryManager.h"
#include "llvm/ExecutionEngine/Orc/CompileUtils.h"
#include "llvm/ExecutionEngine/Orc/IRCompileLayer.h"
#include "llvm/ExecutionEngine/Orc/IRTransformLayer.h"
#include "llvm/ExecutionEngine/Orc/LambdaResolver.h"
#include "llvm/ExecutionEngine/Orc/RTDyldObjectLinkingLayer.h"
#include "llvm/ExecutionEngine/Orc/CompileOnDemandLayer.h"
#include "llvm/IR/Mangler.h"
#include "llvm/Support/DynamicLibrary.h"
/**********************************************************************************************************************/
/*using*/
using namespace llvm;
/**********************************************************************************************************************/
/*globals*/
/**********************************************************************************************************************/
class MutatorJIT
{
  private:
    std::unique_ptr<TargetMachine> TM;
    const DataLayout DL;
    std::unique_ptr<orc::JITCompileCallbackManager> CompileCallbackManager;
    orc::RTDyldObjectLinkingLayer<> ObjectLinkingLayer;
    orc::IRCompileLayer<decltype(ObjectLinkingLayer)> CompileLayer;

    typedef std::function<std::unique_ptr<llvm::Module>(std::unique_ptr<llvm::Module>)> OptimizeFunction;

    orc::IRTransformLayer<decltype(CompileLayer), OptimizeFunction> OptimizeLayer;
    orc::CompileOnDemandLayer<decltype(OptimizeLayer)> CODL;

  public:
    typedef decltype(CODL)::ModuleSetHandleT ModuleHandle;
};
/**********************************************************************************************************************/
/**********************************************************************************************************************/
/**********************************************************************************************************************/
/**********************************************************************************************************************/
#endif
#endif
/*last line intentionally left blank*/

