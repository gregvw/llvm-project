//===--- CustomizableTidyModule.cpp - clang-tidy -------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "../ClangTidy.h"
#include "../ClangTidyModule.h"
#include "../ClangTidyModuleRegistry.h"
#include "SuggestCustomCheck.h"

namespace clang::tidy {
namespace customizable {

class CustomizableModule : public ClangTidyModule {
public:
  void addCheckFactories(ClangTidyCheckFactories &CheckFactories) override {
    CheckFactories.registerCheck<SuggestCustomCheck>(
        "customizable-suggest-custom");
  }
};

} // namespace customizable

// Register the CustomizableModule using this statically initialized variable.
static ClangTidyModuleRegistry::Add<customizable::CustomizableModule>
    X("customizable-module", "Adds customizable functions related checks.");

// This anchor is used to force the linker to link in the generated object file
// and thus register the CustomizableModule.
volatile int CustomizableModuleAnchorSource = 0;

} // namespace clang::tidy
