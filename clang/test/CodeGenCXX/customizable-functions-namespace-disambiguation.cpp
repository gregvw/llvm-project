// RUN: %clang_cc1 -std=c++20 -fcustomizable-functions -triple x86_64-unknown-linux-gnu -emit-llvm -o - %s | FileCheck %s

// Test that different namespaces with the same function name get
// distinct mangled names in the attribute, preventing collisions.

namespace lib1 {
  template<typename T>
  custom T compute(T x) {
    return x * 2;
  }
}

namespace lib2 {
  template<typename T>
  custom T compute(T x) {
    return x * 3;
  }
}

int test() {
  return lib1::compute(10) + lib2::compute(20);
}

// CHECK-LABEL: define {{.*}} @_ZN4lib17computeIiEET_S1_(
// CHECK-LABEL: define {{.*}} @_ZN4lib27computeIiEET_S1_(

// Verify that attributes contain distinct mangled names (lib1 vs lib2)
// CHECK: attributes {{.*}} "clang-customizable-function"="_ZN4lib17computeIiEET_S1_" "clang-customizable-function-name"="compute"
// CHECK: attributes {{.*}} "clang-customizable-function"="_ZN4lib27computeIiEET_S1_" "clang-customizable-function-name"="compute"
