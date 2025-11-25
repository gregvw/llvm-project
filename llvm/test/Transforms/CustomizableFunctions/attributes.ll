; RUN: opt -passes=customizable-functions -S < %s | FileCheck %s

; Test that attributes are correctly propagated from the override function
; to the call site in the wrapper.

; Wrapper function
define linkonce_odr i32 @_Z3fooi(i32 noundef %x) #0 {
; CHECK-LABEL: define {{.*}}i32 @_Z3fooi(i32 noundef %x)
; CHECK: [[CALL:%.*]] = tail call signext i32 @__custom_override_foo(i32 noundef %x) #[[ATTR:[0-9]+]]
; CHECK: ret i32 [[CALL]]
entry:
  %call = tail call i32 @_Z3fooi.default(i32 %x)
  ret i32 %call
}

; Default implementation (should not be called)
define internal i32 @_Z3fooi.default(i32 noundef %x) {
entry:
  %add = add nsw i32 %x, 42
  ret i32 %add
}

; Override function with specific attributes:
; - Return value: signext
; - Argument: noundef
; - Function: noinline
define signext i32 @__custom_override_foo(i32 noundef %x) #1 {
entry:
  ret i32 %x
}

attributes #0 = { "clang-customizable-function"="foo" }
attributes #1 = { noinline }

; CHECK: attributes #[[ATTR]] = { noinline }
