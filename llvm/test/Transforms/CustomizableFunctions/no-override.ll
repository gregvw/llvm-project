; RUN: opt -passes=customizable-functions -S < %s | FileCheck %s

; If there is no override function, the pass must not change the body.

define linkonce_odr i32 @_Z3bari(i32 noundef %x)
        #0 {
; CHECK-LABEL: define linkonce_odr i32 @_Z3bari(
; CHECK-SAME: #[[ATTR:[0-9]+]]
; CHECK:       [[CALL:%.*]] = tail call i32 @_Z3bari.default(i32 noundef %x)
; CHECK:       ret i32 [[CALL]]

entry:
  %call = tail call i32 @_Z3bari.default(i32 noundef %x)
  ret i32 %call
}

define internal i32 @_Z3bari.default(i32 noundef %x) {
; CHECK: define internal i32 @_Z3bari.default(
; CHECK: ret i32
entry:
  %add = add nsw i32 %x, 1
  ret i32 %add
}

attributes #0 = { "clang-customizable-function"="bar" }

!0 = !{ptr @_Z3bari}
!1 = !{ptr @_Z3bari.default}
!clang.customizable = !{!0}
!clang.custom.default = !{!1}
