// RUN: %clang_cc1 -std=c++20 -fsyntax-only -verify %s

// Test parsing of custom keyword in requires expressions

custom void f();

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
  { f() } custom -> std::same_as<void>;
};

// All three together
template<typename T>
concept C4 = requires {
  { f() } noexcept custom -> std::same_as<void>;
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
  { f() } custom -> std::same_as<void>;
};

// expected-no-diagnostics
