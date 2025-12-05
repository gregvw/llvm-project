; RUN: opt -passes=customizable-functions -S < %s 2>&1 | FileCheck %s

; Test that signature mismatches are handled correctly.
; When the override has a different signature than the wrapper,
; the pass should NOT rewrite the wrapper - it should keep calling .default.

;=============================================================================
; Test 1: Return type mismatch (wrapper: i32, override: i64)
;=============================================================================

define linkonce_odr i32 @_Z3fooi(i32 noundef %x) #0 {
; CHECK-LABEL: define linkonce_odr i32 @_Z3fooi(
; CHECK:       tail call i32 @_Z3fooi.default(i32 noundef %x)
; CHECK-NOT:   @__custom_override_foo
; CHECK:       ret i32
entry:
  %call = tail call i32 @_Z3fooi.default(i32 noundef %x)
  ret i32 %call
}

define internal i32 @_Z3fooi.default(i32 noundef %x) {
entry:
  %add = add nsw i32 %x, 1
  ret i32 %add
}

; Override has wrong return type (i64 instead of i32)
define i64 @__custom_override_foo(i32 noundef %x) {
; CHECK: define i64 @__custom_override_foo(
entry:
  %ext = sext i32 %x to i64
  ret i64 %ext
}

;=============================================================================
; Test 2: Argument type mismatch (wrapper: i32, override: i64 arg)
;=============================================================================

define linkonce_odr i32 @_Z3bari(i32 noundef %x) #1 {
; CHECK-LABEL: define linkonce_odr i32 @_Z3bari(
; CHECK:       tail call i32 @_Z3bari.default(i32 noundef %x)
; CHECK-NOT:   @__custom_override_bar
; CHECK:       ret i32
entry:
  %call = tail call i32 @_Z3bari.default(i32 noundef %x)
  ret i32 %call
}

define internal i32 @_Z3bari.default(i32 noundef %x) {
entry:
  %add = add nsw i32 %x, 2
  ret i32 %add
}

; Override has wrong argument type (i64 instead of i32)
define i32 @__custom_override_bar(i64 noundef %x) {
; CHECK: define i32 @__custom_override_bar(
entry:
  %trunc = trunc i64 %x to i32
  ret i32 %trunc
}

;=============================================================================
; Test 3: Argument count mismatch (wrapper: 1 arg, override: 2 args)
;=============================================================================

define linkonce_odr i32 @_Z3bazi(i32 noundef %x) #2 {
; CHECK-LABEL: define linkonce_odr i32 @_Z3bazi(
; CHECK:       tail call i32 @_Z3bazi.default(i32 noundef %x)
; CHECK-NOT:   @__custom_override_baz
; CHECK:       ret i32
entry:
  %call = tail call i32 @_Z3bazi.default(i32 noundef %x)
  ret i32 %call
}

define internal i32 @_Z3bazi.default(i32 noundef %x) {
entry:
  %add = add nsw i32 %x, 3
  ret i32 %add
}

; Override has wrong number of arguments (2 instead of 1)
define i32 @__custom_override_baz(i32 noundef %x, i32 noundef %y) {
; CHECK: define i32 @__custom_override_baz(
entry:
  %add = add nsw i32 %x, %y
  ret i32 %add
}

;=============================================================================
; Test 4: Matching signature should still work (sanity check)
;=============================================================================

define linkonce_odr i32 @_Z4quuxi(i32 noundef %x) #3 {
; CHECK-LABEL: define {{.*}}i32 @_Z4quuxi(
; CHECK:       tail call i32 @__custom_override_quux(i32 noundef %x)
; CHECK-NOT:   @_Z4quuxi.default
; CHECK:       ret i32
entry:
  %call = tail call i32 @_Z4quuxi.default(i32 noundef %x)
  ret i32 %call
}

define internal i32 @_Z4quuxi.default(i32 noundef %x) {
entry:
  %add = add nsw i32 %x, 4
  ret i32 %add
}

; Override has matching signature - should be used
define i32 @__custom_override_quux(i32 noundef %x) {
; CHECK: define i32 @__custom_override_quux(
entry:
  %mul = mul nsw i32 %x, 10
  ret i32 %mul
}

attributes #0 = { "clang-customizable-function"="foo" }
attributes #1 = { "clang-customizable-function"="bar" }
attributes #2 = { "clang-customizable-function"="baz" }
attributes #3 = { "clang-customizable-function"="quux" }
