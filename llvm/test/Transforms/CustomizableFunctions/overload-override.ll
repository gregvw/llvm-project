; RUN: opt -passes=customizable-functions -S < %s | FileCheck %s

; Test that overrides using mangled names can distinguish between overloads,
; allowing independent customization of process(int) and process(double).

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

; Default implementations
define internal i32 @_Z7processi.default(i32 %x) {
  %mul = mul i32 %x, 2
  ret i32 %mul
}

define internal double @_Z7processd.default(double %x) {
  %mul = fmul double %x, 3.0
  ret double %mul
}

; Override only the int version
define i32 @__custom_override__Z7processi(i32 %x) {
  %mul = mul i32 %x, 100
  ret i32 %mul
}

; The attributes store different mangled names for different overloads
attributes #0 = { "clang-customizable-function"="_Z7processi" "clang-customizable-function-name"="process" }
attributes #1 = { "clang-customizable-function"="_Z7processd" "clang-customizable-function-name"="process" }

; process(int) should be overridden
; CHECK-LABEL: define i32 @_Z7processi(i32 %x)
; CHECK-NEXT:  entry:
; CHECK-NEXT:    %0 = {{.*}}call i32 @__custom_override__Z7processi(i32 %x)
; CHECK-NEXT:    ret i32 %0
; CHECK-NEXT:  }

; process(double) should NOT be overridden (no matching override)
; CHECK-LABEL: define double @_Z7processd(double %x)
; CHECK-NEXT:    %call = {{.*}}call double @_Z7processd.default(double %x)
; CHECK-NEXT:    ret double %call
; CHECK-NEXT:  }
