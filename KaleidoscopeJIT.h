//===- KaleidoscopeJIT.h - A simple JIT for Kaleidoscope --------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Contains a simple JIT definition for use in the kaleidoscope tutorials.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_EXECUTIONENGINE_ORC_KALEIDOSCOPEJIT_H
#define LLVM_EXECUTIONENGINE_ORC_KALEIDOSCOPEJIT_H

#include "llvm-8.1/ADT/STLExtras.h"
#include "llvm-8.1/ADT/iterator_range.h"
#include "llvm-8.1/ExecutionEngine/ExecutionEngine.h"
#include "llvm-8.1/ExecutionEngine/JITSymbol.h"
#include "llvm-8.1/ExecutionEngine/Orc/CompileUtils.h"
#include "llvm-8.1/ExecutionEngine/Orc/IRCompileLayer.h"
#include "llvm-8.1/ExecutionEngine/Orc/LambdaResolver.h"
#include "llvm-8.1/ExecutionEngine/Orc/RTDyldObjectLinkingLayer.h"
#include "llvm-8.1/ExecutionEngine/RTDyldMemoryManager.h"
#include "llvm-8.1/ExecutionEngine/SectionMemoryManager.h"
#include "llvm-8.1/IR/DataLayout.h"
#include "llvm-8.1/IR/Mangler.h"
#include "llvm-8.1/Support/DynamicLibrary.h"
#include "llvm-8.1/Support/raw_ostream.h"
#include "llvm-8.1/Target/TargetMachine.h"
#include <algorithm>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace llvm {
namespace orc {

class KaleidoscopeJIT {
public:
  using ObjLayerT = LegacyRTDyldObjectLinkingLayer;
  using CompileLayerT = LegacyIRCompileLayer<ObjLayerT, SimpleCompiler>;

  KaleidoscopeJIT()
      : Resolver(createLegacyLookupResolver(
            ES,
            [this](const std::string &Name) {
              auto S =  ObjectLayer.findSymbol(Name, true);
              if (!S) {
                if (auto Sym = RTDyldMemoryManager::getSymbolAddressInProcess(Name)) {
                  return JITSymbol(Sym, JITSymbolFlags::Exported);
                }
              }
              return S;
            },
            [](Error Err) { cantFail(std::move(Err), "lookupFlags failed"); })),
        TM(EngineBuilder().selectTarget()), DL(TM->createDataLayout()),
        ObjectLayer(ES,
                    [this](VModuleKey) {
                      return ObjLayerT::Resources{
                          std::make_shared<SectionMemoryManager>(), Resolver};
                    }),
        CompileLayer(ObjectLayer, SimpleCompiler(*TM)) {
    llvm::sys::DynamicLibrary::LoadLibraryPermanently(nullptr);
  }

  TargetMachine &getTargetMachine() { return *TM; }

  VModuleKey addModule(std::unique_ptr<Module> M) {
    auto K = ES.allocateVModule();
    cantFail(CompileLayer.addModule(K, std::move(M)));
    ModuleKeys.push_back(K);
    return K;
  }

  void removeModule(VModuleKey K) {
    ModuleKeys.erase(find(ModuleKeys, K));
    cantFail(CompileLayer.removeModule(K));
  }

  JITSymbol findSymbol(const std::string Name) {
    return findMangledSymbol(mangle(Name));
  }

private:
  std::string mangle(const std::string &Name) {
    std::string MangledName;
    {
      raw_string_ostream MangledNameStream(MangledName);
      Mangler::getNameWithPrefix(MangledNameStream, Name, DL);
    }
    return MangledName;
  }

  JITSymbol findMangledSymbol(const std::string &Name) {
#ifdef _WIN32
    // The symbol lookup of ObjectLinkingLayer uses the SymbolRef::SF_Exported
    // flag to decide whether a symbol will be visible or not, when we call
    // IRCompileLayer::findSymbolIn with ExportedSymbolsOnly set to true.
    //
    // But for Windows COFF objects, this flag is currently never set.
    // For a potential solution see: https://reviews.llvm.org/rL258665
    // For now, we allow non-exported symbols on Windows as a workaround.
    const bool ExportedSymbolsOnly = false;
#else
    const bool ExportedSymbolsOnly = true;
#endif

    // Search modules in reverse order: from last added to first added.
    // This is the opposite of the usual search order for dlsym, but makes more
    // sense in a REPL where we want to bind to the newest available definition.
    for (auto H : make_range(ModuleKeys.rbegin(), ModuleKeys.rend()))
      if (auto Sym = CompileLayer.findSymbolIn(H, Name, ExportedSymbolsOnly))
        return Sym;

    // If we can't find the symbol in the JIT, try looking in the host process.
    if (auto SymAddr = RTDyldMemoryManager::getSymbolAddressInProcess(Name))
      return JITSymbol(SymAddr, JITSymbolFlags::Exported);

#ifdef _WIN32
    // For Windows retry without "_" at beginning, as RTDyldMemoryManager uses
    // GetProcAddress and standard libraries like msvcrt.dll use names
    // with and without "_" (for example "_itoa" but "sin").
    if (Name.length() > 2 && Name[0] == '_')
      if (auto SymAddr =
              RTDyldMemoryManager::getSymbolAddressInProcess(Name.substr(1)))
        return JITSymbol(SymAddr, JITSymbolFlags::Exported);
#endif

    return nullptr;
  }

  ExecutionSession ES;
  std::shared_ptr<SymbolResolver> Resolver;
  std::unique_ptr<TargetMachine> TM;
  const DataLayout DL;
  ObjLayerT ObjectLayer;
  CompileLayerT CompileLayer;
  std::vector<VModuleKey> ModuleKeys;
};

} // end namespace orc
} // end namespace llvm

#endif // LLVM_EXECUTIONENGINE_ORC_KALEIDOSCOPEJIT_H
