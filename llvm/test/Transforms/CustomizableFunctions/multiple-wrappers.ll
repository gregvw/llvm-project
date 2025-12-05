; RUN: opt -passes=customizable-functions -S < %s 2>&1 | FileCheck %s

; Test: Multiple wrappers with the same logical name but different signatures.
;
; Scenario:
;   - Wrapper 1: @_Z3fooi (takes i32) with logical name "foo"
;   - Wrapper 2: @_Z3fooPi (takes i32*) with logical name "foo"
;   - Single override: @__custom_override_foo with signature matching Wrapper 1
;
; Expected behavior:
;   - Only Wrapper 1 is rewritten (signature matches)
;   - Wrapper 2 keeps calling its .default (signature mismatch)
;
; This is the "only rewrite if function type matches exactly" rule.

;=============================================================================
; Wrapper 1: foo(int) - matches override signature
;=============================================================================

define linkonce_odr i32 @_Z3fooi(i32 noundef %x) #0 {
; CHECK-LABEL: define {{.*}}i32 @_Z3fooi(
; CHECK:       tail call i32 @__custom_override_foo(i32 noundef %x)
; CHECK-NOT:   @_Z3fooi.default
; CHECK:       ret i32
entry:
  %call = tail call i32 @_Z3fooi.default(i32 noundef %x)
  ret i32 %call
}

define internal i32 @_Z3fooi.default(i32 noundef %x) {
; CHECK: define internal i32 @_Z3fooi.default(
entry:
  %add = add nsw i32 %x, 1
  ret i32 %add
}

;=============================================================================
; Wrapper 2: foo(int*) - does NOT match override signature
;=============================================================================

define linkonce_odr i32 @_Z3fooPi(ptr noundef %p) #0 {
; CHECK-LABEL: define linkonce_odr i32 @_Z3fooPi(
; CHECK:       tail call i32 @_Z3fooPi.default(ptr noundef %p)
; CHECK-NOT:   @__custom_override_foo
; CHECK:       ret i32
entry:
  %call = tail call i32 @_Z3fooPi.default(ptr noundef %p)
  ret i32 %call
}

define internal i32 @_Z3fooPi.default(ptr noundef %p) {
; CHECK: define internal i32 @_Z3fooPi.default(
entry:
  %val = load i32, ptr %p, align 4
  ret i32 %val
}

;=============================================================================
; Override: matches foo(int), not foo(int*)
;=============================================================================

define i32 @__custom_override_foo(i32 noundef %x) {
; CHECK: define i32 @__custom_override_foo(
entry:
  %mul = mul nsw i32 %x, 100
  ret i32 %mul
}

; Both wrappers share the same logical name
attributes #0 = { "clang-customizable-function"="foo" }


;=============================================================================
; Second test case: namespace-qualified wrappers with same logical name
;=============================================================================

; Wrapper 3: ns1::bar(int)
define linkonce_odr i32 @_ZN3ns13barEi(i32 noundef %x) #1 {
; CHECK-LABEL: define {{.*}}i32 @_ZN3ns13barEi(
; CHECK:       tail call i32 @__custom_override_bar(i32 noundef %x)
; CHECK-NOT:   @_ZN3ns13barEi.default
; CHECK:       ret i32
entry:
  %call = tail call i32 @_ZN3ns13barEi.default(i32 noundef %x)
  ret i32 %call
}

define internal i32 @_ZN3ns13barEi.default(i32 noundef %x) {
entry:
  %add = add nsw i32 %x, 10
  ret i32 %add
}

; Wrapper 4: ns2::bar(int) - different namespace, same logical name and signature
define linkonce_odr i32 @_ZN3ns23barEi(i32 noundef %x) #1 {
; CHECK-LABEL: define {{.*}}i32 @_ZN3ns23barEi(
; CHECK:       tail call i32 @__custom_override_bar(i32 noundef %x)
; CHECK-NOT:   @_ZN3ns23barEi.default
; CHECK:       ret i32
entry:
  %call = tail call i32 @_ZN3ns23barEi.default(i32 noundef %x)
  ret i32 %call
}

define internal i32 @_ZN3ns23barEi.default(i32 noundef %x) {
entry:
  %add = add nsw i32 %x, 20
  ret i32 %add
}

; Override for bar - matches both ns1::bar and ns2::bar (same signature)
define i32 @__custom_override_bar(i32 noundef %x) {
; CHECK: define i32 @__custom_override_bar(
entry:
  %mul = mul nsw i32 %x, 200
  ret i32 %mul
}

attributes #1 = { "clang-customizable-function"="bar" }
