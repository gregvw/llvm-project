//===- CustomizableFunctions.h - 'custom' function overrides ----*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This pass rewrites wrappers for customizable functions to call override
// implementations when available.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_IPO_CUSTOMIZABLEFUNCTIONS_H
#define LLVM_TRANSFORMS_IPO_CUSTOMIZABLEFUNCTIONS_H

#include "llvm/IR/PassManager.h"

namespace llvm {

class Module;

class CustomizableFunctionsPass
    : public PassInfoMixin<CustomizableFunctionsPass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);
};

} // end namespace llvm

#endif // LLVM_TRANSFORMS_IPO_CUSTOMIZABLEFUNCTIONS_H
