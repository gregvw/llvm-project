; RUN: opt -disable-verify -verify-analysis-invalidation=0 -eagerly-invalidate-analyses=0 -debug-pass-manager \
; RUN:     -passes='lto<O2>' -S %s 2>&1 \
; RUN:     | FileCheck %s

; RUN: opt -disable-verify -verify-analysis-invalidation=0 -eagerly-invalidate-analyses=0 -debug-pass-manager \
; RUN:     -passes='thinlto<O2>' -S %s 2>&1 \
; RUN:     | FileCheck %s

; CHECK: Running pass: CustomizableFunctionsPass
; CHECK-NOT: Running pass: CustomizableFunctionsPass

define void @foo() {
  ret void
}
