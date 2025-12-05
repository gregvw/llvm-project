//===--- SuggestCustomCheck.cpp - clang-tidy -----------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "SuggestCustomCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang::tidy::customizable {

namespace {

AST_MATCHER(FunctionDecl, isCustom) {
  return Node.isCustom();
}

AST_MATCHER(FunctionDecl, looksLikeCustomizationPoint) {
  StringRef Name = Node.getName();

  // Check for common customization point naming patterns
  if (Name == "allocate" || Name == "deallocate" ||
      Name == "construct" || Name == "destroy" ||
      Name == "initialize" || Name == "finalize" ||
      Name == "configure" || Name == "setup" ||
      Name == "cleanup" || Name == "validate")
    return true;

  // Check for common suffixes
  if (Name.ends_with("_hook") || Name.ends_with("_impl") ||
      Name.ends_with("_override") || Name.ends_with("_customization_point") ||
      Name.ends_with("_custom") || Name.ends_with("_policy"))
    return true;

  // Check for common prefixes
  if (Name.starts_with("custom_") || Name.starts_with("hook_") ||
      Name.starts_with("override_"))
    return true;

  return false;
}

} // namespace

void SuggestCustomCheck::registerMatchers(MatchFinder *Finder) {
  // Match free functions (not member functions) that:
  // 1. Are not already marked custom
  // 2. Have external linkage
  // 3. Have names suggesting they're customization points
  // 4. Are not inline
  Finder->addMatcher(
      functionDecl(
          unless(isCustom()),
          unless(cxxMethodDecl()),
          unless(isInline()),
          hasExternalFormalLinkage(),
          looksLikeCustomizationPoint()
      ).bind("func"),
      this);
}

void SuggestCustomCheck::check(const MatchFinder::MatchResult &Result) {
  const auto *FD = Result.Nodes.getNodeAs<FunctionDecl>("func");
  if (!FD)
    return;

  // Skip if this is just a declaration (wait for definition)
  if (!FD->isThisDeclarationADefinition())
    return;

  // Get the location where 'custom' should be inserted
  SourceLocation InsertLoc = FD->getBeginLoc();

  // If there's a storage class specifier, insert after it
  if (FD->getStorageClass() != SC_None) {
    InsertLoc = FD->getTypeSpecStartLoc();
  }

  diag(FD->getLocation(),
       "function '%0' appears to be a customization point; "
       "consider marking it 'custom'")
      << FD->getName()
      << FixItHint::CreateInsertion(InsertLoc, "custom ");

  diag(FD->getLocation(),
       "customizable functions enable link-time override via LTO",
       DiagnosticIDs::Note);
}

} // namespace clang::tidy::customizable
