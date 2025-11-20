// RUN: %clang_cc1 -std=c++20 -fcustomizable-functions -triple x86_64-unknown-linux-gnu -emit-llvm -o - %s | FileCheck %s

// Test that custom works with function overloading

// CHECK-LABEL: define linkonce_odr{{.*}} i32 @_Z3addii(i32 noundef %a, i32 noundef %b)
// CHECK-SAME: #[[ATTR:[0-9]+]]
// CHECK: tail call i32 @_Z3addii.default

// CHECK-LABEL: define internal{{.*}} i32 @_Z3addii.default

custom int add(int a, int b) {
  return a + b;
}

// CHECK-LABEL: define linkonce_odr{{.*}} double @_Z3adddd(double noundef %a, double noundef %b)
// CHECK-SAME: #[[ATTR]]
// CHECK: tail call double @_Z3adddd.default

// CHECK-LABEL: define internal{{.*}} double @_Z3adddd.default

custom double add(double a, double b) {
  return a + b;
}

// Both overloads share the same attributes, so they use the same attribute group
// CHECK-DAG: attributes #[[ATTR]] = {
// CHECK-SAME: "clang-customizable-function"="add"

int test_int() {
  return add(1, 2);
}

double test_double() {
  return add(1.5, 2.5);
}
