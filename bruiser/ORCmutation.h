
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
#if __clang_major__ == 5
/**********************************************************************************************************************/
/*included modules*/
/*project headers*/
/*standard library headers*/
#include <vector>
/*clang headers*/
/*llvm headers*/
#include "llvm/ExecutionEngine/SectionMemoryManager.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/JITSymbol.h"
#include "llvm/ExecutionEngine/RTDyldMemoryManager.h"
#include "llvm/ExecutionEngine/SectionMemoryManager.h"
#include "llvm/ExecutionEngine/Orc/CompileUtils.h"
#include "llvm/ExecutionEngine/Orc/IRCompileLayer.h"
#include "llvm/ExecutionEngine/Orc/IRTransformLayer.h"
#include "llvm/ExecutionEngine/Orc/LambdaResolver.h"
#include "llvm/ExecutionEngine/Orc/RTDyldObjectLinkingLayer.h"
#include "llvm/ExecutionEngine/Orc/CompileOnDemandLayer.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Mangler.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/DynamicLibrary.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"
/**********************************************************************************************************************/
/*using*/
using namespace llvm;
/**********************************************************************************************************************/
/*globals*/
/**********************************************************************************************************************/
namespace mutatorjit{
class MutatorJIT
{
  private:
    std::unique_ptr<TargetMachine> TM;
    const DataLayout DL;
    std::unique_ptr<orc::JITCompileCallbackManager> CompileCallbackManager;
    orc::RTDyldObjectLinkingLayer ObjectLayer;
    orc::IRCompileLayer<decltype(ObjectLayer), orc::SimpleCompiler> CompileLayer;

    using OptimizeFunction = std::function<std::shared_ptr<llvm::Module>(std::shared_ptr<llvm::Module>)>;

    orc::IRTransformLayer<decltype(CompileLayer), OptimizeFunction> OptimizeLayer;
    //orc::CompileOnDemandLayer<decltype(OptimizeLayer)> CODL;

  public:
    using ModuleHandle =  decltype(OptimizeLayer)::ModuleHandleT;

    MutatorJIT() : TM(EngineBuilder().selectTarget()), DL(TM->createDataLayout()), \
                   ObjectLayer([]() {return std::make_shared<SectionMemoryManager>();}),\
                   CompileLayer(ObjectLayer, orc::SimpleCompiler(*TM)), \
                   OptimizeLayer(CompileLayer, [this](std::shared_ptr<llvm::Module> M){return optimizeModule(std::move(M));})
                   {
                     llvm::sys::DynamicLibrary::LoadLibraryPermanently(nullptr);
                   }

    TargetMachine &getTargetMachine()
    {
      return *TM;
    }

    ModuleHandle addModule(std::unique_ptr<llvm::Module> M)
    {
      auto Resolver = orc::createLambdaResolver
      (
        [&](const std::string &Name)
        {
          if (auto Sym = OptimizeLayer.findSymbol(Name, false))
          {
            return Sym;
          }
          else
          {
            return JITSymbol(nullptr);
          }
        },
        [](const std::string &Name)
        {
          if (auto SymAddr = RTDyldMemoryManager::getSymbolAddressInProcess(Name))
          {
            return JITSymbol(SymAddr, JITSymbolFlags::Exported);
          }
          else
          {
            return JITSymbol(nullptr);
          }
        }
      );

      return cantFail(OptimizeLayer.addModule(std::move(M), std::move(Resolver)));
    }

    JITSymbol findSymbol(const std::string Name)
    {
      std::string MangledName;
      raw_string_ostream MangledNameStream(MangledName);
      Mangler::getNameWithPrefix(MangledNameStream, Name, DL);
      return OptimizeLayer.findSymbol(MangledNameStream.str(), true);
    }

    void  removeModule(ModuleHandle H)
    {
      cantFail(OptimizeLayer.removeModule(H));
    }

  private:
    std::shared_ptr<llvm::Module> optimizeModule(std::shared_ptr<llvm::Module> M)
    {
      auto FPM = llvm::make_unique<legacy::FunctionPassManager>(M.get());

      FPM->add(createInstructionCombiningPass());
      FPM->add(createReassociatePass());
      FPM->add(createGVNPass());
      FPM->add(createCFGSimplificationPass());
      FPM->doInitialization();

      for (auto &F : *M)
      {
        FPM->run(F);
      }

      return M;
    }
};

class runMainOnJit
{
#if 0
  std::unique_ptr<llvm::Module> M = buildModule();
  MutatorJIT jit;
  Handle H = jit.addModule(*M);
  int (*Main)(int, char*[]) =
    (int(*)(int*, char*[]))jit.findSymbol("main").getSymbolAddressInProcess();
  int result = Main();
  jit.removeModule(H);
#endif
};
} //end of namespace mutatorjit
/**********************************************************************************************************************/
/**********************************************************************************************************************/
/**********************************************************************************************************************/
/**********************************************************************************************************************/
#endif
#endif
/*last line intentionally left blank*/

