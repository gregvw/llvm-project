// RUN: %clang_cc1 -std=c++20 -fcustomizable-functions -triple x86_64-unknown-linux-gnu -emit-llvm -o - %s | FileCheck %s

// Test that custom works in namespaces

namespace math {
  // CHECK-LABEL: define linkonce_odr{{.*}} i32 @_ZN4math3addEii(i32 noundef %a, i32 noundef %b)
  // CHECK-SAME: #[[ATTR:[0-9]+]]
  // CHECK: tail call i32 @_ZN4math3addEii.default

  // CHECK-LABEL: define internal{{.*}} i32 @_ZN4math3addEii.default

  custom int add(int a, int b) {
    return a + b;
  }

  // CHECK: attributes #[[ATTR]] = {
  // CHECK-SAME: "clang-customizable-function"="_ZN4math3addEii"
  // CHECK-SAME: "clang-customizable-function-name"="add"
}

int test() {
  return math::add(1, 2);
}
