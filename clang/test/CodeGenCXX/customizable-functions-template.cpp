// RUN: %clang_cc1 -std=c++20 -fcustomizable-functions -triple x86_64-unknown-linux-gnu -emit-llvm -o - %s | FileCheck %s

// Test that custom works with function templates

template<class T>
custom T id(T x) { return x; }

// CHECK-DAG: define linkonce_odr{{.*}} i32 @_Z2idIiET_S0_(i32 noundef %x) #[[ATTR_INT:[0-9]+]] comdat
// CHECK-DAG: define internal{{.*}} i32 @_Z2idIiET_S0_.default(i32 noundef %x)

int use_int(int x) {
  return id(x);
}

// CHECK-DAG: define linkonce_odr{{.*}} double @_Z2idIdET_S0_(double noundef %x) #[[ATTR_DOUBLE:[0-9]+]] comdat
// CHECK-DAG: define internal{{.*}} double @_Z2idIdET_S0_.default(double noundef %x)

double use_double(double x) {
  return id(x);
}

// CHECK-DAG: attributes #[[ATTR_INT]] = {{{.*}}"clang-customizable-function"="id"{{.*}}}

// CHECK-DAG: attributes #[[ATTR_DOUBLE]] = {{{.*}}"clang-customizable-function"="id"{{.*}}}
