// RUN: %clang_cc1 -std=c++20 -fcustomizable-functions -triple x86_64-unknown-linux-gnu -emit-llvm -o - %s | FileCheck %s

// Test that custom works with function templates

template<class T>
custom T id(T x) { return x; }

// CHECK-LABEL: define linkonce_odr{{.*}} i32 @_Z2idIiET_S0_(i32 noundef %x)
// CHECK-SAME: #[[ATTR_INT:[0-9]+]]
// CHECK-SAME: comdat
// CHECK: entry:
// CHECK:   %call = tail call i32 @_Z2idIiET_S0_.default(i32 %x)
// CHECK:   ret i32 %call

// CHECK-LABEL: define internal{{.*}} i32 @_Z2idIiET_S0_.default(i32 noundef %x)

int use_int(int x) {
  return id(x);
}

// CHECK-LABEL: define linkonce_odr{{.*}} double @_Z2idIdET_S0_(double noundef %x)
// CHECK-SAME: #[[ATTR_DOUBLE:[0-9]+]]
// CHECK-SAME: comdat
// CHECK: entry:
// CHECK:   %call = tail call double @_Z2idIdET_S0_.default(double %x)
// CHECK:   ret double %call

// CHECK-LABEL: define internal{{.*}} double @_Z2idIdET_S0_.default(double noundef %x)

double use_double(double x) {
  return id(x);
}

// CHECK-DAG: attributes #[[ATTR_INT]] = {
// CHECK-SAME: "clang-customizable-function"="id"

// CHECK-DAG: attributes #[[ATTR_DOUBLE]] = {
// CHECK-SAME: "clang-customizable-function"="id"
