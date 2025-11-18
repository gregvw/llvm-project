; RUN: llc -mtriple=aarch64 -o - %s | FileCheck %s

; CHECK-LABEL: custom_fn:
; CHECK: .custom custom_fn
define custom void @custom_fn() {
entry:
  ret void
}

; CHECK-LABEL: normal_fn:
; CHECK-NOT: .custom
define void @normal_fn() {
entry:
  ret void
}
