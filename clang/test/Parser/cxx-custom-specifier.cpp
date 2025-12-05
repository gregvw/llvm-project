// RUN: %clang_cc1 -fsyntax-only -verify -std=c++20 %s

// Test basic parsing of custom function specifier

// Free functions with custom specifier
custom void f1();
custom int f2();
custom auto f3() -> int;

// Member functions with custom specifier
struct S {
  custom void m1();
  custom int m2();
  custom static void m3();
  virtual custom void m4();
  custom virtual void m5();
};

// Custom with other specifiers
inline custom void f4();
custom inline void f5();
constexpr custom int f6() { return 42; }
custom constexpr int f7() { return 42; }

// Custom with templates
template<typename T>
custom T f8(T t) { return t; }

template<typename T>
struct TemplateStruct {
  custom void method();
};

// Duplicate custom (should warn)
custom custom void f9(); // expected-warning {{duplicate 'custom' declaration specifier}}

// Custom on different declaration types
namespace N {
  custom void f10();
}

class C {
public:
  custom void m();
  custom C();  // custom constructor
  custom ~C(); // custom destructor
};
