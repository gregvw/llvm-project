// RUN: %check_clang_tidy -std=c++20 -fcustomizable-functions %s customizable-suggest-custom %t

// Functions that should trigger the check

void allocate(int size) {
  // CHECK-MESSAGES: :[[@LINE-1]]:6: warning: function 'allocate' appears to be a customization point; consider marking it 'custom' [customizable-suggest-custom]
  // CHECK-MESSAGES: :[[@LINE-2]]:6: note: customizable functions enable link-time override via LTO
  // CHECK-FIXES: custom void allocate(int size) {
}

int deallocate(void *ptr) {
  // CHECK-MESSAGES: :[[@LINE-1]]:5: warning: function 'deallocate' appears to be a customization point
  // CHECK-FIXES: custom int deallocate(void *ptr) {
  return 0;
}

void process_hook(int x) {
  // CHECK-MESSAGES: :[[@LINE-1]]:6: warning: function 'process_hook' appears to be a customization point
  // CHECK-FIXES: custom void process_hook(int x) {
}

int validate_impl(int value) {
  // CHECK-MESSAGES: :[[@LINE-1]]:5: warning: function 'validate_impl' appears to be a customization point
  // CHECK-FIXES: custom int validate_impl(int value) {
  return value > 0;
}

void custom_handler(int code) {
  // CHECK-MESSAGES: :[[@LINE-1]]:6: warning: function 'custom_handler' appears to be a customization point
  // CHECK-FIXES: custom void custom_handler(int code) {
}

// Functions that should NOT trigger the check

// Already marked custom
custom void already_custom() {
}

// Member function (not supported)
class MyClass {
  void allocate(int size) {
  }
};

// Inline function (not supported)
inline void inline_allocate() {
}

// Static linkage (not supported)
static void static_allocate() {
}

// Anonymous namespace (internal linkage)
namespace {
void anonymous_allocate() {
}
}

// Regular function with no customization-point-like name
void regular_function() {
}

// Function that doesn't match any pattern
void do_something() {
}
