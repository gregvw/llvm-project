//===- CustomizableFunctions.cpp - Handle 'custom' functions --------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This pass rewrites calls from customizable function wrappers to override
// functions when present.
//
// For a wrapper function F with attribute
//   "clang-customizable-function"="foo",
// and IR like:
//   define linkonce_odr i32 @_Z3fooi(i32 %x) #0 {
//     %call = call i32 @_Z3fooi.default(i32 %x)
//     ret i32 %call
//   }
// and an override
//   define i32 @__custom_override_foo(i32 %x) { ... }
//
// The pass rewrites the body of @_Z3fooi to call @__custom_override_foo
// instead of @_Z3fooi.default.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/IPO/CustomizableFunctions.h"
#include "llvm/IR/Attributes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/Debug.h"

using namespace llvm;

#define DEBUG_TYPE "customizable-functions"

namespace {

static bool processWrapper(Function &Wrapper, Module &M) {
  // Look for the attribute "clang-customizable-function".
  if (!Wrapper.hasFnAttribute("clang-customizable-function"))
    return false;

  auto Attr = Wrapper.getFnAttribute("clang-customizable-function");
  StringRef LogicalName = Attr.getValueAsString();

  // Form the override function name. For now we use a simple convention:
  //   "__custom_override_" + logical-name
  // This ignores namespaces and mangling on purpose and matches our tests.
  std::string OverrideName = ("__custom_override_" + LogicalName).str();
  Function *Override = M.getFunction(OverrideName);
  if (!Override || Override->isDeclaration())
    return false;

  // Types must match exactly for now.
  if (Override->getFunctionType() != Wrapper.getFunctionType())
    return false;

  LLVM_DEBUG(dbgs() << "CustomizableFunctions: overriding wrapper "
                    << Wrapper.getName() << " with " << Override->getName()
                    << "\n");

  // Rewrite the body of Wrapper to forward all arguments to Override.
  // For now we discard the old body entirely.
  BasicBlock *Entry = nullptr;
  if (!Wrapper.empty()) {
    // Clear existing blocks.
    SmallVector<BasicBlock *, 8> OldBlocks;
    for (BasicBlock &BB : Wrapper)
      OldBlocks.push_back(&BB);
    for (BasicBlock *BB : OldBlocks)
      BB->eraseFromParent();
  }

  Entry = BasicBlock::Create(M.getContext(), "entry", &Wrapper);
  IRBuilder<> B(Entry);

  SmallVector<Value *, 8> Args;
  for (Argument &Arg : Wrapper.args())
    Args.push_back(&Arg);

  CallInst *Call = B.CreateCall(Override, Args);
  if (Wrapper.getReturnType()->isVoidTy())
    B.CreateRetVoid();
  else
    B.CreateRet(Call);

  // We modified the function body; any analysis on it is invalidated.
  return true;
}

} // end anonymous namespace

PreservedAnalyses
CustomizableFunctionsPass::run(Module &M, ModuleAnalysisManager &AM) {
  bool Changed = false;

  for (Function &F : M) {
    if (F.isDeclaration())
      continue;
    Changed |= processWrapper(F, M);
  }

  return Changed ? PreservedAnalyses::none() : PreservedAnalyses::all();
}

// Registration with the new pass manager.
llvm::PassPluginLibraryInfo getCustomizableFunctionsPluginInfo() {
  return {LLVM_PLUGIN_API_VERSION, "CustomizableFunctions", LLVM_VERSION_STRING,
          [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, ModulePassManager &MPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                  if (Name == "customizable-functions") {
                    MPM.addPass(CustomizableFunctionsPass());
                    return true;
                  }
                  return false;
                });
          }};
}

// This is used when building as a plugin. In-tree you'll instead register
// in PassBuilder's default pipeline setup, but keeping this allows plugin use.
extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
  return getCustomizableFunctionsPluginInfo();
}
