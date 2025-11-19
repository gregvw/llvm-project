// RUN: %clang_cc1 -std=c++20 -fsyntax-only -fcustomizable-functions -verify %s

// Test that custom and inline cannot be used together

inline custom void foo();  // expected-error {{'custom' and 'inline' cannot both be specified}}

custom inline void bar();  // expected-error {{'custom' and 'inline' cannot both be specified}}

// These should be fine
custom void baz();
inline void qux();

class Test {
  inline custom void method1();  // expected-error {{'custom' and 'inline' cannot both be specified}}
  custom inline void method2();  // expected-error {{'custom' and 'inline' cannot both be specified}}

  custom void method3();  // OK
  inline void method4();  // OK
};
