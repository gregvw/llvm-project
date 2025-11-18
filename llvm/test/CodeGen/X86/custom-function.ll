; RUN: llc -mtriple=x86_64 -o - %s | FileCheck %s

; CHECK-LABEL: custom_fn
; CHECK: .custom custom_fn
; CHECK: custom_fn:
define void @custom_fn() custom {
entry:
  ret void
}

; CHECK-LABEL: normal_fn
; CHECK-NOT: .custom
; CHECK: normal_fn:
define void @normal_fn() {
entry:
  ret void
}
