// RUN: %clang_cc1 -std=c++20 -fcustomizable-functions -fsyntax-only -verify %s

// Test that custom inline is rejected (mutually exclusive)

custom inline int add(int a, int b) { // expected-error {{'custom' and 'inline' cannot be combined}}
  return a + b;
}

// Test that custom on member functions is rejected

struct Foo {
  custom void method(); // expected-error {{'custom' can only be applied to free functions, not member functions}}
  custom Foo(); // expected-error {{'custom' cannot be applied to constructors}}
  custom ~Foo(); // expected-error {{'custom' cannot be applied to destructors}}
};

// Test that custom works on free functions
custom void valid_free_function() { }  // OK
