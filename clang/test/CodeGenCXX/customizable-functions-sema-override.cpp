// RUN: %clang_cc1 -std=c++20 -fcustomizable-functions -fcustomizable-functions-sema \
// RUN:   -emit-llvm -o - %s | FileCheck %s --check-prefix=CHECK-SEMA

// RUN: %clang_cc1 -std=c++20 -fcustomizable-functions \
// RUN:   -emit-llvm -o - %s | FileCheck %s --check-prefix=CHECK-NO-SEMA

// Test the experimental Sema-level override resolution for 'custom' functions.
// When -fcustomizable-functions-sema is enabled, calls to 'custom' functions
// should be redirected to <name>_override if one exists with a matching signature.

custom int foo(int x) {
  return x + 1;  // default implementation
}

int foo_override(int x) {
  return x + 100;  // override implementation
}

int call_foo(int x) {
  return foo(x);
}

// With -fcustomizable-functions-sema, the call should go to foo_override
// CHECK-SEMA-LABEL: define {{.*}}i32 @_Z8call_fooi(
// CHECK-SEMA: call {{.*}}i32 @_Z12foo_overridei(
// CHECK-SEMA-NOT: call {{.*}}i32 @_Z3fooi(

// Without -fcustomizable-functions-sema, the call should go to the wrapper
// CHECK-NO-SEMA-LABEL: define {{.*}}i32 @_Z8call_fooi(
// CHECK-NO-SEMA: call {{.*}}i32 @_Z3fooi(
// CHECK-NO-SEMA-NOT: call {{.*}}i32 @_Z12foo_overridei(


// Test: Override with different signature should NOT be used
custom int bar(int x) {
  return x * 2;
}

// Wrong signature (takes different type)
int bar_override(double x) {
  return static_cast<int>(x * 3);
}

int call_bar(int x) {
  return bar(x);
}

// Even with sema override enabled, mismatched signature means no override
// CHECK-SEMA-LABEL: define {{.*}}i32 @_Z8call_bari(
// CHECK-SEMA: call {{.*}}i32 @_Z3bari(
// CHECK-SEMA-NOT: call {{.*}}i32 @_Z12bar_overrided(


// Test: Multiple valid overrides - best match should be used
custom int baz(int x) {
  return x;
}

int baz_override(int x) {
  return x + 1000;
}

int call_baz(int x) {
  return baz(x);
}

// CHECK-SEMA-LABEL: define {{.*}}i32 @_Z8call_bazi(
// CHECK-SEMA: call {{.*}}i32 @_Z12baz_overridei(


// Test: Non-custom function should not use _override
int regular(int x) {
  return x + 5;
}

int regular_override(int x) {
  return x + 500;
}

int call_regular(int x) {
  return regular(x);
}

// Non-custom functions should not be affected
// CHECK-SEMA-LABEL: define {{.*}}i32 @_Z12call_regulari(
// CHECK-SEMA: call {{.*}}i32 @_Z7regulari(
// CHECK-SEMA-NOT: call {{.*}}i32 @_Z16regular_overridei(
