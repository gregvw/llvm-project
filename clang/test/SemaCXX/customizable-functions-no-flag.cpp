// RUN: %clang_cc1 -std=c++20 -fsyntax-only -verify %s

// Test that without -fcustomizable-functions, 'custom' is treated as a normal
// identifier, not as a specifier. This ensures the contextual keyword logic
// only fires when the LangOpt is on.

// 'custom' should work as a variable name
int custom = 42;  // OK - 'custom' is just an identifier

// 'custom' should work as a type name
struct custom {
  int x;
};

// 'custom' used as a type
custom my_custom_var;

// 'custom' can be used in expressions
int use_custom() {
  return ::custom + 1;  // Uses the global int variable
}

// When 'custom' appears in declaration-specifier position without the flag,
// it should be treated as a type name (the struct above), not a specifier.
// This will cause a parse error because 'custom' (struct type) before 'int' is invalid.
// expected-error@+1 {{cannot combine with previous 'custom' declaration specifier}}
custom int f();

// A function returning our custom struct type should work
custom get_custom() {  // OK - returns struct custom
  custom c;
  c.x = 10;
  return c;
}

// 'custom' as a parameter name
void takes_custom(int custom) {
  (void)custom;
}

// 'custom' as a function name
void custom_func() {}  // OK

// 'custom' in a namespace
namespace ns {
  int custom = 100;
}

// Template with 'custom' as name
template<typename T>
T custom_template(T x) {
  return x;
}
