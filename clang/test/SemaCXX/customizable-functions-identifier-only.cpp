// RUN: %clang_cc1 -fsyntax-only -verify -std=c++20 -fcustomizable-functions %s

// Test that 'custom' is rejected on non-identifier function names
// to prevent ICE in CodeGen where getName() asserts on non-identifiers

// Free function operator overloads - should be rejected
custom int operator+(int a, int b); // expected-error {{'custom' can only be applied to functions with identifier names, not operators}}

custom int operator-(int a, int b); // expected-error {{'custom' can only be applied to functions with identifier names, not operators}}

struct S {
  int value;
};

custom S operator*(const S& a, const S& b); // expected-error {{'custom' can only be applied to functions with identifier names, not operators}}

// User-defined literals - should be rejected
custom long double operator""_km(long double); // expected-error {{'custom' can only be applied to functions with identifier names, not user-defined literals}}
custom int operator""_deg(unsigned long long); // expected-error {{'custom' can only be applied to functions with identifier names, not user-defined literals}}

// Regular functions with identifiers - should be accepted (no errors)
custom int add(int a, int b);
custom double multiply(double x, double y);

template<typename T>
custom T max(T a, T b);
