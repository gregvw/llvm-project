// RUN: %clang_cc1 -std=c++20 -fcustomizable-functions -E %s -o - | FileCheck %s --check-prefix=CHECK-ON
// RUN: %clang_cc1 -std=c++20 -E %s -o - | FileCheck %s --check-prefix=CHECK-OFF

// Test that __has_feature(customizable_functions) and __has_extension(customizable_functions)
// return the correct values based on the -fcustomizable-functions flag.

#if __has_feature(customizable_functions)
int feature_on = 1;
#else
int feature_off = 0;
#endif

#if __has_extension(customizable_functions)
int extension_on = 1;
#else
int extension_off = 0;
#endif

// CHECK-ON: int feature_on = 1;
// CHECK-ON: int extension_on = 1;

// CHECK-OFF: int feature_off = 0;
// CHECK-OFF: int extension_off = 0;
