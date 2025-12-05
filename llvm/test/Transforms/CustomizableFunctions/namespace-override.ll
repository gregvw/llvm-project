; RUN: opt -passes=customizable-functions -S < %s | FileCheck %s

; Test that overrides using mangled names only affect the intended function,
; not other functions with the same unmangled name in different namespaces.

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

; Default implementations
define internal i32 @_ZN4lib17computeEi.default(i32 %x) {
  %mul = mul i32 %x, 2
  ret i32 %mul
}

define internal i32 @_ZN4lib27computeEi.default(i32 %x) {
  %mul = mul i32 %x, 3
  ret i32 %mul
}

; Override for lib1::compute only - uses mangled name
define i32 @__custom_override__ZN4lib17computeEi(i32 %x) {
  %mul = mul i32 %x, 10
  ret i32 %mul
}

; The attribute now stores the mangled name
attributes #0 = { "clang-customizable-function"="_ZN4lib17computeEi" "clang-customizable-function-name"="compute" }
attributes #1 = { "clang-customizable-function"="_ZN4lib27computeEi" "clang-customizable-function-name"="compute" }

; CHECK-LABEL: define i32 @_ZN4lib17computeEi(i32 %x)
; CHECK-NEXT:  entry:
; CHECK-NEXT:    %0 = {{.*}}call i32 @__custom_override__ZN4lib17computeEi(i32 %x)
; CHECK-NEXT:    ret i32 %0
; CHECK-NEXT:  }

; lib2::compute should NOT be overridden (no matching override symbol)
; CHECK-LABEL: define i32 @_ZN4lib27computeEi(i32 %x)
; CHECK-NEXT:    %call = {{.*}}call i32 @_ZN4lib27computeEi.default(i32 %x)
; CHECK-NEXT:    ret i32 %call
; CHECK-NEXT:  }
