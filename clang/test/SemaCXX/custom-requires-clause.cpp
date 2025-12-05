// RUN: %clang_cc1 -std=c++20 -fcustomizable-functions -fsyntax-only -verify %s

// Test custom keyword in requires clauses

namespace N {
  custom void customFunc();
  void nonCustomFunc();
}

// Basic custom requirement - should satisfy
template<typename T>
concept HasCustomFunction = requires {
  { N::customFunc() } custom;
};

static_assert(HasCustomFunction<int>);

// Custom requirement with non-custom function - should fail
template<typename T>
concept RequiresCustom = requires {
  { N::nonCustomFunc() } custom; // expected-note {{because N::nonCustomFunc() is not marked with 'custom' specifier}}
};

static_assert(!RequiresCustom<int>); // expected-error {{static assertion failed}}
                                      // expected-note@-1 {{because 'int' does not satisfy 'RequiresCustom'}}

// Custom with noexcept
custom void customNoexcept() noexcept;

template<typename T>
concept HasCustomNoexcept = requires {
  { customNoexcept() } noexcept custom;
};

static_assert(HasCustomNoexcept<int>);

// Custom with type constraint
custom int customReturnsInt();

template<typename T>
concept HasCustomInt = requires {
  { customReturnsInt() } custom -> std::same_as<int>;
};

static_assert(HasCustomInt<int>);

// Member function with custom
struct S {
  // Note: member functions cannot be marked custom per the restrictions
  void method();
};

custom void freeCustom();

template<typename T>
concept HasFreeCustom = requires(T t) {
  { freeCustom() } custom;
};

static_assert(HasFreeCustom<int>);

// Template function with custom
template<typename T>
custom T customTemplate(T t);

template<typename T>
concept HasCustomTemplate = requires(T t) {
  { customTemplate(t) } custom;
};

static_assert(HasCustomTemplate<int>);
static_assert(HasCustomTemplate<double>);

// Combined requirements
template<typename T>
concept ComplexRequirement = requires(T t) {
  { customFunc() } custom;
  { customReturnsInt() } custom -> std::same_as<int>;
  { customNoexcept() } noexcept custom;
};

static_assert(ComplexRequirement<int>);

// Negative test: expression that doesn't call custom function
template<typename T>
concept NotCustom = requires {
  { N::nonCustomFunc() } custom; // expected-note {{because N::nonCustomFunc() is not marked with 'custom' specifier}}
};

static_assert(!NotCustom<void>); // expected-error {{static assertion failed}}
                                  // expected-note@-1 {{because 'void' does not satisfy 'NotCustom'}}
