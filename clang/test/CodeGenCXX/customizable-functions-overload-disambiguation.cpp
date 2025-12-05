// RUN: %clang_cc1 -std=c++20 -fcustomizable-functions -triple x86_64-unknown-linux-gnu -emit-llvm -o - %s | FileCheck %s

// Test that different overloads of the same function get distinct
// mangled names in the attribute, allowing independent customization.

custom int process(int x) {
  return x * 2;
}

custom double process(double x) {
  return x * 3.0;
}

void test() {
  process(42);
  process(3.14);
}

// CHECK-LABEL: define {{.*}} @_Z7processi(
// CHECK-LABEL: define {{.*}} @_Z7processd(

// Verify that attributes contain distinct mangled names for overloads
// CHECK: attributes {{.*}} "clang-customizable-function"="_Z7processi" "clang-customizable-function-name"="process"
// CHECK: attributes {{.*}} "clang-customizable-function"="_Z7processd" "clang-customizable-function-name"="process"
