// RUN: %clang_cc1 -std=c++20 -fcustomizable-functions -triple x86_64-unknown-linux-gnu -emit-llvm -o - %s | FileCheck %s

// Test custom with trailing return type

// CHECK-LABEL: define linkonce_odr{{.*}} i32 @_Z3getv()
// CHECK-SAME: #[[ATTR:[0-9]+]]
// CHECK: tail call i32 @_Z3getv.default

// CHECK-LABEL: define internal{{.*}} i32 @_Z3getv.default

custom auto get() -> int {
  return 42;
}

// CHECK: attributes #[[ATTR]] = {
// CHECK-SAME: "clang-customizable-function"="_Z3getv"
// CHECK-SAME: "clang-customizable-function-name"="get"

int test() {
  return get();
}
