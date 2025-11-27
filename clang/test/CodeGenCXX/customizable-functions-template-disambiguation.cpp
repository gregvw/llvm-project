// RUN: %clang_cc1 -std=c++20 -fcustomizable-functions -triple x86_64-unknown-linux-gnu -emit-llvm -o - %s | FileCheck %s

// Test that different template instantiations get distinct mangled names,
// allowing each instantiation to be independently customized.

template<typename T>
custom T transform(T x) {
  return x + x;
}

void test() {
  transform(42);        // int instantiation
  transform(3.14);      // double instantiation
  transform<long>(5L);  // long instantiation
}

// CHECK-LABEL: define {{.*}} @_Z9transformIiET_S0_(
// CHECK-LABEL: define {{.*}} @_Z9transformIdET_S0_(
// CHECK-LABEL: define {{.*}} @_Z9transformIlET_S0_(

// Verify each template instantiation has a distinct mangled name in attributes
// CHECK: attributes {{.*}} "clang-customizable-function"="_Z9transformIiET_S0_" "clang-customizable-function-name"="transform"
// CHECK: attributes {{.*}} "clang-customizable-function"="_Z9transformIdET_S0_" "clang-customizable-function-name"="transform"
// CHECK: attributes {{.*}} "clang-customizable-function"="_Z9transformIlET_S0_" "clang-customizable-function-name"="transform"
