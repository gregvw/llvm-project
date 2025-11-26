; RUN: opt -disable-verify -verify-analysis-invalidation=0 -eagerly-invalidate-analyses=0 -debug-pass-manager \
; RUN:     -passes='lto<O2>' -S %s 2>&1 \
; RUN:     | FileCheck %s --check-prefix=CHECK-LTO

; RUN: opt -disable-verify -verify-analysis-invalidation=0 -eagerly-invalidate-analyses=0 -debug-pass-manager \
; RUN:     -passes='thinlto<O2>' -S %s 2>&1 \
; RUN:     | FileCheck %s --check-prefix=CHECK-THINLTO

; Verify that CustomizableFunctionsPass runs in both LTO and ThinLTO pipelines.
; Also verify it runs early (before the main inliner) so overrides can be inlined.

; CHECK-LTO: Running pass: CustomizableFunctionsPass
; The pass should run before the inliner so overrides get inlined
; CHECK-LTO: Running pass: InlinerPass
; Ensure the pass only runs once
; CHECK-LTO-NOT: Running pass: CustomizableFunctionsPass

; CHECK-THINLTO: Running pass: CustomizableFunctionsPass
; The pass should run before the inliner so overrides get inlined
; CHECK-THINLTO: Running pass: InlinerPass
; Ensure the pass only runs once
; CHECK-THINLTO-NOT: Running pass: CustomizableFunctionsPass

define void @foo() {
  ret void
}
