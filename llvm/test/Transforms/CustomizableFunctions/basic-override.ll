; RUN: opt -passes=customizable-functions -S < %s | FileCheck %s

; A customizable function wrapper + default implementation.
; Logical name is "foo".

define linkonce_odr i32 @_Z3fooi(i32 noundef %x)
        #0 {
; CHECK-LABEL: define linkonce_odr i32 @_Z3fooi(
; CHECK-SAME: #[[ATTR:[0-9]+]]
; CHECK:       [[NEWCALL:%.*]] = tail call i32 @__custom_override_foo(i32 noundef %x)
; CHECK-NOT:   tail call i32 @_Z3fooi.default(
; CHECK:       ret i32 [[NEWCALL]]

entry:
  %call = tail call i32 @_Z3fooi.default(i32 noundef %x)
  ret i32 %call
}

define internal i32 @_Z3fooi.default(i32 noundef %x) {
; CHECK: define internal i32 @_Z3fooi.default(
; CHECK: ret i32
entry:
  %add = add nsw i32 %x, 42
  ret i32 %add
}

; Override function: same signature, name derived from logical name "foo".
define i32 @__custom_override_foo(i32 noundef %x) {
; CHECK: define i32 @__custom_override_foo(
; CHECK: ret i32
entry:
  %mul = mul nsw i32 %x, 2
  ret i32 %mul
}

attributes #0 = { "clang-customizable-function"="foo" }

!0 = !{ptr @_Z3fooi}
!1 = !{ptr @_Z3fooi.default}
!clang.customizable = !{!0}
!clang.custom.default = !{!1}
