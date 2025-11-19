// RUN: %clang_cc1 -std=c++20 -fcustomizable-functions -triple x86_64-unknown-linux-gnu -emit-llvm -o - %s | FileCheck %s

// Test that custom works with noexcept

// CHECK-LABEL: define linkonce_odr{{.*}} noundef i32 @_Z4safePi(ptr noundef %x)
// CHECK-SAME: #[[ATTR:[0-9]+]]
// CHECK: entry:
// CHECK:   %call = tail call i32 @_Z4safePi.default(ptr %x)
// CHECK:   ret i32 %call

// CHECK-LABEL: define internal{{.*}} i32 @_Z4safePi.default(ptr noundef %x)

custom int safe(int* x) noexcept {
  return *x;
}

// CHECK: attributes #[[ATTR]] = {
// CHECK-SAME: "clang-customizable-function"="safe"

int test(int* p) {
  return safe(p);
}
