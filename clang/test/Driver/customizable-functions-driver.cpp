// Test that -fcustomizable-functions-sema is forwarded by driver to cc1
//
// RUN: %clang -### -fcustomizable-functions %s 2>&1 | FileCheck %s --check-prefix=CHECK-BASE
// RUN: %clang -### -fcustomizable-functions -fcustomizable-functions-sema %s 2>&1 | FileCheck %s --check-prefix=CHECK-SEMA

// CHECK-BASE: "-fcustomizable-functions"
// CHECK-BASE-NOT: "-fcustomizable-functions-sema"

// CHECK-SEMA: "-fcustomizable-functions"
// CHECK-SEMA: "-fcustomizable-functions-sema"
