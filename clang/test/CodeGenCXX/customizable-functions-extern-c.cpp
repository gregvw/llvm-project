// RUN: %clang_cc1 -std=c++20 -fcustomizable-functions -triple x86_64-unknown-linux-gnu -emit-llvm -o - %s | FileCheck %s

extern "C" custom int foo(int x) {
  return x + 1;
}

// Wrapper for the customizable function. Must be unmangled and have custom attribute.
// CHECK-LABEL: define linkonce_odr{{.*}} i32 @foo(i32 noundef %x)
// CHECK-SAME: #[[ATTR:[0-9]+]]
// CHECK: tail call i32 @foo.default(i32 %x)
// CHECK: ret i32

// Default implementation. Must be internal.
// CHECK-LABEL: define internal i32 @foo.default(i32 noundef %x)

// Attribute definition.
// CHECK: attributes #[[ATTR]] = {
// CHECK-SAME: "clang-customizable-function"="foo"
// CHECK-SAME: "clang-customizable-function-name"="foo"

// Metadata nodes for customizable function.
// CHECK: !clang.customizable
// CHECK: !clang.custom.default
