// RUN: %clang_cc1 -std=c++20 -fcustomizable-functions -triple x86_64-unknown-linux-gnu -emit-llvm -o - %s | FileCheck %s

// Test custom with multiple parameters and complex types

struct Point {
  int x, y;
};

// CHECK-LABEL: define linkonce_odr{{.*}} void @_Z9transformR5PointS0_(ptr noundef {{.*}} %p1, ptr noundef {{.*}} %p2)
// CHECK-SAME: #[[ATTR:[0-9]+]]
// CHECK: tail call void @_Z9transformR5PointS0_.default

// CHECK-LABEL: define internal{{.*}} void @_Z9transformR5PointS0_.default

custom void transform(Point& p1, Point& p2) {
  int temp = p1.x;
  p1.x = p2.x;
  p2.x = temp;
}

// CHECK: attributes #[[ATTR]] = {
// CHECK-SAME: "clang-customizable-function"="_Z9transformR5PointS0_"
// CHECK-SAME: "clang-customizable-function-name"="transform"

void test() {
  Point a{1, 2}, b{3, 4};
  transform(a, b);
}
