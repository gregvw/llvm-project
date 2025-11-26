// RUN: %clang_cc1 -std=c++20 -fcustomizable-functions -ast-dump %s | FileCheck %s --check-prefix=CHECK-DUMP
// RUN: %clang_cc1 -std=c++20 -fcustomizable-functions -ast-print %s | FileCheck %s --check-prefix=CHECK-PRINT

// Test that -ast-dump correctly prints the 'custom' specifier

custom void f();
// CHECK-DUMP: FunctionDecl{{.*}} f 'void ()' custom

custom int add(int a, int b) {
  return a + b;
}
// CHECK-DUMP: FunctionDecl{{.*}} add 'int (int, int)' custom

// With constexpr (constexpr has implicit inline, but that's allowed with custom)
custom constexpr int compute() { return 42; }
// CHECK-DUMP: FunctionDecl{{.*}} compute 'int ()' custom constexpr

// In a namespace
namespace ns {
  custom void bar();
  // CHECK-DUMP: FunctionDecl{{.*}} bar 'void ()' custom
}

// Template
template<typename T>
custom T identity(T x) { return x; }
// CHECK-DUMP: FunctionTemplateDecl
// CHECK-DUMP: FunctionDecl{{.*}} identity 'T (T)' custom

// Test that -ast-print emits the 'custom' keyword
// CHECK-PRINT: custom void f();
// CHECK-PRINT: custom int add(int a, int b)
// CHECK-PRINT: custom constexpr int compute()
// CHECK-PRINT: custom void bar();
// CHECK-PRINT: custom T identity(T x)
