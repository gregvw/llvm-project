// RUN: %clang_cc1 -fsyntax-only -verify -std=c++20 %s

// Test semantic analysis of custom function specifier

// Basic custom functions
custom void f1() {}
custom int f2() { return 42; }

struct S {
  custom void method1();
  custom int method2();
  custom static void static_method();
};

void S::method1() {}  // Definition doesn't need to repeat custom
int S::method2() { return 0; }

// Custom with virtual
struct Base {
  virtual custom void vf1();
  custom virtual void vf2();
};

struct Derived : Base {
  custom void vf1() override;  // Override of custom virtual function
};

// Custom with constexpr
constexpr custom int cf1() { return 42; }
custom constexpr int cf2() { return 43; }

// Custom with templates
template<typename T>
custom T tf1(T t) { return t; }

template<typename T>
struct TS {
  custom void method();
};

template<typename T>
custom void TS<T>::method() {}

// Test instantiation
void test() {
  tf1(42);
  tf1(3.14);

  TS<int> ts;
  ts.method();
}

// Custom on lambdas would require special handling in lambda parsing
// Currently not supported as custom is a function declaration specifier
// auto lambda = []() custom { return 42; };

// expected-no-diagnostics
