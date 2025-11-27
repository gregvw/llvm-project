// RUN: %clang_cc1 -std=c++20 -fcustomizable-functions -triple x86_64-unknown-linux-gnu -emit-llvm -o - %s | FileCheck %s

// Test customizable function with void return type

// CHECK-LABEL: define linkonce_odr{{.*}} void @_Z7processPii(ptr noundef %data, i32 noundef %size)
// CHECK: #[[ATTR:[0-9]+]]
// CHECK: entry:
// CHECK:   tail call void @_Z7processPii.default(ptr %data, i32 %size)
// CHECK:   ret void

// CHECK-LABEL: define internal void @_Z7processPii.default(ptr noundef %data, i32 noundef %size)
// CHECK: entry:

custom void process(int* data, int size) {
  for (int i = 0; i < size; ++i) {
    data[i] *= 2;
  }
}

// CHECK: attributes #[[ATTR]] = {
// CHECK-SAME: "clang-customizable-function"="_Z7processPii"
// CHECK-SAME: "clang-customizable-function-name"="process"

void test() {
  int arr[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  process(arr, 10);
}
