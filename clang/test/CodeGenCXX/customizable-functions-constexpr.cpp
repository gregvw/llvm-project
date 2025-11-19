// RUN: %clang_cc1 -std=c++20 -fcustomizable-functions -triple x86_64-unknown-linux-gnu -emit-llvm -o - %s | FileCheck %s

// Test that custom works with constexpr (constexpr implies inline, but doesn't conflict)

// CHECK-LABEL: define linkonce_odr{{.*}} i32 @_Z3mulii(i32 noundef %a, i32 noundef %b)
// CHECK-SAME: #[[ATTR:[0-9]+]]
// CHECK: entry:
// CHECK:   %call = tail call i32 @_Z3mulii.default(i32 %a, i32 %b)
// CHECK:   ret i32 %call

// CHECK-LABEL: define internal{{.*}} i32 @_Z3mulii.default(i32 noundef %a, i32 noundef %b)

custom constexpr int mul(int a, int b) {
  return a * b;
}

// CHECK: attributes #[[ATTR]] = {
// CHECK-SAME: "clang-customizable-function"="mul"

int test() {
  return mul(3, 4);
}
