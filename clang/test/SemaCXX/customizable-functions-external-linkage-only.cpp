// RUN: %clang_cc1 -fsyntax-only -verify -std=c++20 -fcustomizable-functions %s

// Test that 'custom' is rejected on functions with internal linkage
// to prevent silent linkage upgrade and ABI/visibility violations.

// static (internal linkage) - should be rejected
static custom int foo(int x) { // expected-error {{'custom' can only be applied to functions with external linkage}}
  return x * 2;
}

// Anonymous namespace (internal linkage) - should be rejected
namespace {
  custom int bar(int x) { // expected-error {{'custom' can only be applied to functions with external linkage}}
    return x * 3;
  }
}

// Regular function (external linkage) - should be accepted (no error)
custom int baz(int x) {
  return x * 4;
}

// extern "C" (external linkage) - should be accepted (no error)
extern "C" custom int qux(int x) {
  return x * 5;
}

// Namespace function (external linkage) - should be accepted (no error)
namespace ns {
  custom int process(int x) {
    return x * 6;
  }
}
