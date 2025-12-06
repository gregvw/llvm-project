// RUN: %clang_cc1 -std=c++20 -fcustomizable-functions -fsyntax-only -verify %s

// Test parsing of custom keyword in requires expressions

custom void f() noexcept;
custom int g();

// Simple type constraint helper
template<typename T, typename U>
concept same_as = __is_same(T, U);

// Basic custom in compound requirement
template<typename T>
concept C1 = requires {
  { f() } custom;
};

// Custom with noexcept
template<typename T>
concept C2 = requires {
  { f() } noexcept custom;
};

// Custom with type constraint
template<typename T>
concept C3 = requires {
  { g() } custom -> same_as<int>;
};

// All three together
template<typename T>
concept C4 = requires {
  { f() } noexcept custom -> same_as<void>;
};

// Custom in parameterized requires
template<typename T>
concept C5 = requires(T t) {
  { f() } custom;
};

// Multiple requirements with custom
template<typename T>
concept C6 = requires {
  { f() } custom;
  { f() } noexcept;
  { g() } custom -> same_as<int>;
};

// expected-no-diagnostics
