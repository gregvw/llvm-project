// RUN: %clang_cc1 -std=c++20 -fcustomizable-functions -triple x86_64-unknown-linux-gnu -emit-llvm -o - %s | FileCheck %s

// A normal function: must produce regular IR with NO customizable markers.
int normal(int x) {
  return x + 1;
}

// CHECK-LABEL: define{{.*}} i32 @_Z6normali(i32 noundef %x)
// CHECK-NOT: .default
// CHECK-NOT: "clang-customizable-function"
// CHECK: ret i32

// Verify no customizable metadata is generated for normal functions
// CHECK-NOT: !clang.customizable
// CHECK-NOT: !clang.custom.default
