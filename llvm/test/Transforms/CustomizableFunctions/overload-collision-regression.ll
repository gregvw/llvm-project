; RUN: opt -passes=customizable-functions -S < %s | FileCheck %s

; Regression test for overload collision bug.
; Before fix: process(int) and process(double) would share __custom_override_process,
; so providing an override for one signature would affect BOTH overloads if types matched.
; After fix: Each overload has its own override symbol based on mangled signature.

; Wrapper for process(int) - mangled name _Z7processi
define i32 @_Z7processi(i32 %x) #0 {
  %call = call i32 @_Z7processi.default(i32 %x)
  ret i32 %call
}

; Wrapper for process(double) - mangled name _Z7processd
define double @_Z7processd(double %x) #1 {
  %call = call double @_Z7processd.default(double %x)
  ret double %call
}

; Default: process(int) returns x * 2
define internal i32 @_Z7processi.default(i32 %x) {
  %mul = mul i32 %x, 2
  ret i32 %mul
}

; Default: process(double) returns x * 3.0
define internal double @_Z7processd.default(double %x) {
  %mul = fmul double %x, 3.0
  ret double %mul
}

; User provides override ONLY for int version - uses mangled name
define i32 @__custom_override__Z7processi(i32 %x) {
  %mul = mul i32 %x, 100
  ret i32 %mul
}

; OLD BUG: With __custom_override_process (unmangled), we couldn't provide
; separate overrides for int and double versions. Only one override symbol
; existed, and whichever matched would be used.

; NEW BEHAVIOR: With mangled names:
; - process(int) looks for __custom_override__Z7processi (found, overridden)
; - process(double) looks for __custom_override__Z7processd (not found, uses default)

attributes #0 = { "clang-customizable-function"="_Z7processi" "clang-customizable-function-name"="process" }
attributes #1 = { "clang-customizable-function"="_Z7processd" "clang-customizable-function-name"="process" }

; Verify process(int) is overridden
; CHECK-LABEL: define i32 @_Z7processi(i32 %x)
; CHECK-NEXT:  entry:
; CHECK-NEXT:    %0 = {{.*}}call i32 @__custom_override__Z7processi(i32 %x)
; CHECK-NEXT:    ret i32 %0

; Verify process(double) is NOT overridden (uses default)
; CHECK-LABEL: define double @_Z7processd(double %x)
; CHECK-NEXT:    %call = {{.*}}call double @_Z7processd.default(double %x)
; CHECK-NEXT:    ret double %call

; With the old implementation, users could not selectively override just the int
; or double version - they were forced to override both or neither (depending on
; which override symbol existed and which signature matched).
