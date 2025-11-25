// RUN: %clang -### -c -fcustomizable-functions %s 2>&1 | FileCheck %s
// CHECK: "-fcustomizable-functions"

// RUN: %clang -### -flto -fcustomizable-functions %s 2>&1 | FileCheck %s --check-prefix=LTO
// LTO: "-fcustomizable-functions"
