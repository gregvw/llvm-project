// RUN: %clang_cc1 -std=c++20 -fcustomizable-functions -triple x86_64-unknown-linux-gnu -emit-llvm -o - %s | FileCheck %s

// Test basic customizable function codegen

// CHECK-LABEL: define linkonce_odr{{.*}} i32 @_Z3fooi(i32 noundef %x)
// CHECK-SAME: #[[ATTR:[0-9]+]]
// CHECK: entry:
// CHECK:   %call = tail call i32 @_Z3fooi.default(i32 %x)
// CHECK:   ret i32 %call

// CHECK-LABEL: define internal i32 @_Z3fooi.default(i32 noundef %x)
// CHECK: entry:
// CHECK:   %add = add nsw i32 %x, 42
// CHECK:   ret i32 %add

custom int foo(int x) {
  return x + 42;
}

// CHECK: attributes #[[ATTR]] = {
// CHECK-SAME: "clang-customizable-function"="foo"

// CHECK: !clang.customizable = !{
// CHECK: !clang.custom.default = !{

int main() {
  return foo(10);
}
