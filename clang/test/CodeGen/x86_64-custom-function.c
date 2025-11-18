// RUN: %clang_cc1 -triple x86_64 -emit-llvm -fcustomizable-functions -o - %s | FileCheck %s --check-prefix=CUSTOM
// RUN: %clang_cc1 -triple x86_64 -emit-llvm -o - %s | FileCheck %s --check-prefix=NO-CUSTOM

// CUSTOM: Function Attrs: custom
// CUSTOM: define {{.*}} @foo()
// NO-CUSTOM: define {{.*}} @foo()
// NO-CUSTOM-NOT: custom
void foo() {
}

// CUSTOM: Function Attrs: custom
// CUSTOM: define {{.*}} @bar()
// NO-CUSTOM: define {{.*}} @bar()
int bar() {
  return 42;
}
