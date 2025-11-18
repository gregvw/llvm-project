// RUN: %clang_cc1 -triple aarch64 -emit-llvm -fcustomizable-functions -o - %s | FileCheck %s --check-prefix=CUSTOM
// RUN: %clang_cc1 -triple aarch64 -emit-llvm -o - %s | FileCheck %s --check-prefix=NO-CUSTOM

// CUSTOM: define{{.*}} custom {{.*}} @foo()
// NO-CUSTOM: define{{.*}} @foo()
// NO-CUSTOM-NOT: custom
void foo() {
}

// CUSTOM: define{{.*}} custom {{.*}} @bar()
// NO-CUSTOM: define{{.*}} @bar()
int bar() {
  return 42;
}
