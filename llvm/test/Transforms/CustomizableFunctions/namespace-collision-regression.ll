; RUN: opt -passes=customizable-functions -S < %s | FileCheck %s

; Regression test for namespace collision bug.
; Before fix: Both lib1::compute and lib2::compute would share the override
; symbol __custom_override_compute, causing the wrong function to be overridden.
; After fix: Each gets its own override based on mangled name.

; Wrapper for lib1::compute(int) - mangled name _ZN4lib17computeEi
define i32 @_ZN4lib17computeEi(i32 %x) #0 {
  %call = call i32 @_ZN4lib17computeEi.default(i32 %x)
  ret i32 %call
}

; Wrapper for lib2::compute(int) - mangled name _ZN4lib27computeEi
define i32 @_ZN4lib27computeEi(i32 %x) #1 {
  %call = call i32 @_ZN4lib27computeEi.default(i32 %x)
  ret i32 %call
}

; Default implementation: lib1::compute multiplies by 2
define internal i32 @_ZN4lib17computeEi.default(i32 %x) {
  %mul = mul i32 %x, 2
  ret i32 %mul
}

; Default implementation: lib2::compute multiplies by 3
define internal i32 @_ZN4lib27computeEi.default(i32 %x) {
  %mul = mul i32 %x, 3
  ret i32 %mul
}

; User provides override for lib1::compute only - uses MANGLED name
define i32 @__custom_override__ZN4lib17computeEi(i32 %x) {
  %mul = mul i32 %x, 10
  ret i32 %mul
}

; OLD BUG: If we had used __custom_override_compute (unmangled), BOTH wrappers
; would have been overridden because they both looked for the same symbol.

; NEW BEHAVIOR: With mangled names, each wrapper looks for its own symbol:
; - lib1::compute looks for __custom_override__ZN4lib17computeEi (found, overridden)
; - lib2::compute looks for __custom_override__ZN4lib27computeEi (not found, uses default)

attributes #0 = { "clang-customizable-function"="_ZN4lib17computeEi" "clang-customizable-function-name"="compute" }
attributes #1 = { "clang-customizable-function"="_ZN4lib27computeEi" "clang-customizable-function-name"="compute" }

; Verify lib1::compute is overridden
; CHECK-LABEL: define i32 @_ZN4lib17computeEi(i32 %x)
; CHECK-NEXT:  entry:
; CHECK-NEXT:    %0 = {{.*}}call i32 @__custom_override__ZN4lib17computeEi(i32 %x)
; CHECK-NEXT:    ret i32 %0

; Verify lib2::compute is NOT overridden (uses default)
; CHECK-LABEL: define i32 @_ZN4lib27computeEi(i32 %x)
; CHECK-NEXT:    %call = {{.*}}call i32 @_ZN4lib27computeEi.default(i32 %x)
; CHECK-NEXT:    ret i32 %call

; This test would have FAILED with the old implementation because both functions
; would have been overridden, making lib2::compute return x*10 instead of x*3.
