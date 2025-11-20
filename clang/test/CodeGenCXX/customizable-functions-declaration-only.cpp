// RUN: %clang_cc1 -std=c++20 -fcustomizable-functions -triple x86_64-unknown-linux-gnu -emit-llvm -o - %s | FileCheck %s

// A declaration *only*, with no definition in this TU.
// This must not generate any IR function bodies or metadata.
custom int foo(int);

// CHECK-NOT: foo.default
// CHECK-NOT: "clang-customizable-function"
// CHECK-NOT: !clang.customizable
// CHECK-NOT: !clang.custom.default
// CHECK-NOT: define
