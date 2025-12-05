// RUN: not %clang_cc1 -std=c++20 -fcustomizable-functions -triple x86_64-unknown-linux-gnu -emit-llvm -o /dev/null %s 2>&1 | FileCheck %s

// Regression test: These used to cause ICE in CodeGen because getName() asserts
// on non-identifier function names. Now they produce clean diagnostics.

// Test 1: Operator overload
custom int operator+(int a, int b) {
  return a + b + 1;
}
// CHECK: error: 'custom' can only be applied to functions with identifier names, not operators

// Test 2: User-defined literal
custom long double operator""_km(long double x) {
  return x * 1000.0;
}
// CHECK: error: 'custom' can only be applied to functions with identifier names, not user-defined literals

// Test 3: Another operator
custom bool operator==(int a, int b) {
  return a == b;
}
// CHECK: error: 'custom' can only be applied to functions with identifier names, not operators
