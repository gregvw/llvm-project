; RUN: opt -passes=customizable-functions -S < %s | FileCheck %s

; Regression test demonstrating that old-style unmangled override symbols
; are now correctly IGNORED (they don't match the mangled attribute).
; This proves the fix works: before, these unmangled symbols would have
; incorrectly matched multiple functions.

; Wrapper with mangled attribute
define i32 @_Z3fooi(i32 %x) #0 {
  %call = call i32 @_Z3fooi.default(i32 %x)
  ret i32 %call
}

define internal i32 @_Z3fooi.default(i32 %x) {
  %mul = mul i32 %x, 2
  ret i32 %mul
}

; Old-style override (using unmangled name) - should be IGNORED
define i32 @__custom_override_foo(i32 %x) {
  %mul = mul i32 %x, 999
  ret i32 %mul
}

; Correct override (using mangled name) - should be USED
define i32 @__custom_override__Z3fooi(i32 %x) {
  %mul = mul i32 %x, 100
  ret i32 %mul
}

; Attribute now contains mangled name
attributes #0 = { "clang-customizable-function"="_Z3fooi" "clang-customizable-function-name"="foo" }

; The wrapper should call the MANGLED override, not the unmangled one
; CHECK-LABEL: define i32 @_Z3fooi(i32 %x)
; CHECK-NEXT:  entry:
; CHECK-NEXT:    %0 = {{.*}}call i32 @__custom_override__Z3fooi(i32 %x)
; CHECK-NEXT:    ret i32 %0

; Verify the old unmangled symbol is NOT used
; CHECK-NOT: call i32 @__custom_override_foo

; This test proves that:
; 1. Old-style unmangled overrides are ignored (backward compatibility break, but necessary)
; 2. Only the correctly mangled override is recognized
; 3. The attribute-to-symbol matching is precise
