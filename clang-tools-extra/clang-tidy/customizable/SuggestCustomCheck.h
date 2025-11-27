//===--- SuggestCustomCheck.h - clang-tidy ---------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_CUSTOMIZABLE_SUGGESTCUSTOMCHECK_H
#define LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_CUSTOMIZABLE_SUGGESTCUSTOMCHECK_H

#include "../ClangTidyCheck.h"

namespace clang::tidy::customizable {

/// Suggests adding 'custom' keyword to functions that appear to be
/// customization points based on naming patterns.
///
/// Functions with names like 'allocate', 'deallocate', or ending in '_hook',
/// '_impl', '_override', or '_customization_point' are likely intended as
/// customization points and should be marked with the 'custom' keyword.
///
/// For the user-facing documentation see:
/// http://clang.llvm.org/extra/clang-tidy/checks/customizable/suggest-custom.html
class SuggestCustomCheck : public ClangTidyCheck {
public:
  SuggestCustomCheck(StringRef Name, ClangTidyContext *Context)
      : ClangTidyCheck(Name, Context) {}

  void registerMatchers(ast_matchers::MatchFinder *Finder) override;
  void check(const ast_matchers::MatchFinder::MatchResult &Result) override;
  bool isLanguageVersionSupported(const LangOptions &LangOpts) const override {
    return LangOpts.CPlusPlus20 && LangOpts.CustomizableFunctions;
  }
};

} // namespace clang::tidy::customizable

#endif // LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_CUSTOMIZABLE_SUGGESTCUSTOMCHECK_H
