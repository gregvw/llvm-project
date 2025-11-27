// RUN: %clang_cc1 -std=c++20 -fcustomizable-functions -fcustomizable-functions-sema \
// RUN:   -emit-llvm -o - %s | FileCheck %s --check-prefix=CHECK-SEMA

// RUN: %clang_cc1 -std=c++20 -fcustomizable-functions \
// RUN:   -emit-llvm -o - %s | FileCheck %s --check-prefix=CHECK-NO-SEMA

// Test the experimental Sema-level override resolution for 'custom' functions.
// When -fcustomizable-functions-sema is enabled, calls to 'custom' functions
// are ADL-enhanced: even qualified calls consider overloads in namespaces
// associated with the argument types.

// ============================================================================
// Test 1: Basic ADL override with user-defined type
// ============================================================================
namespace lib {
  template<typename T>
  custom T add(const T& x, const T& y) {
    return x + y;  // default implementation
  }
}

namespace user {
  struct Number {
    int value;
    Number operator+(const Number& other) const {
      return Number{value + other.value};
    }
  };

  // User override - found via ADL based on argument types
  Number add(const Number& x, const Number& y) {
    return Number{x.value * 100 + y.value};  // different implementation
  }
}

user::Number call_add(user::Number x, user::Number y) {
  return lib::add(x, y);  // Qualified call, but ADL should find user::add
}

// With -fcustomizable-functions-sema, the call should go to user::add
// CHECK-SEMA-LABEL: define {{.*}}void @_Z8call_addN4user6NumberES0_(
// CHECK-SEMA: call {{.*}}void @_ZN4user3addERKNS_6NumberES2_(
// CHECK-SEMA-NOT: call {{.*}}void @_ZN3lib3addINS_6NumberEEET_RKS2_S4_(

// Without -fcustomizable-functions-sema, the call goes to lib::add wrapper
// CHECK-NO-SEMA-LABEL: define {{.*}}void @_Z8call_addN4user6NumberES0_(
// CHECK-NO-SEMA: call {{.*}}void @_ZN3lib3addIN4user6NumberEEET_RKS2_S4_(
// CHECK-NO-SEMA-NOT: call {{.*}}void @_ZN4user3addERKNS_6NumberES2_(

// ============================================================================
// Test 2: No ADL override available - use default
// ============================================================================
namespace math {
  template<typename T>
  custom T multiply(const T& x, const T& y) {
    return x * y;
  }
}

namespace types {
  struct Value {
    int v;
    Value operator*(const Value& other) const {
      return Value{v * other.v};
    }
  };
  // No multiply override defined here
}

types::Value call_multiply(types::Value x, types::Value y) {
  return math::multiply(x, y);
}

// No override available, so both modes call the custom function
// CHECK-SEMA-LABEL: define {{.*}}void @_Z13call_multiplyN5types5ValueES0_(
// CHECK-SEMA: call {{.*}}void @_ZN4math8multiplyIN5types5ValueEEET_RKS3_S5_(

// CHECK-NO-SEMA-LABEL: define {{.*}}void @_Z13call_multiplyN5types5ValueES0_(
// CHECK-NO-SEMA: call {{.*}}void @_ZN4math8multiplyIN5types5ValueEEET_RKS3_S5_(

// ============================================================================
// Test 3: ADL override with wrong signature should not be selected
// ============================================================================
namespace lib2 {
  template<typename T>
  custom T transform(const T& x) {
    return x;
  }
}

namespace user2 {
  struct Data { int d; };

  // Wrong signature - takes two arguments instead of one
  Data transform(const Data& x, const Data& y) {
    return Data{x.d + y.d};
  }
}

user2::Data call_transform(user2::Data x) {
  return lib2::transform(x);
}

// Even with sema override, wrong signature means no override
// CHECK-SEMA-LABEL: define {{.*}}void @_Z14call_transformN5user24DataE(
// CHECK-SEMA: call {{.*}}void @_ZN4lib29transformIN5user24DataEEET_RKS3_(

// ============================================================================
// Test 4: ADL override that is itself custom should not be used
// ============================================================================
namespace base {
  template<typename T>
  custom T process(const T& x) {
    return x;
  }
}

namespace derived {
  struct Item { int i; };

  // This is also custom - should NOT be used as override to prevent layering
  custom Item process(const Item& x) {
    return Item{x.i + 1};
  }
}

derived::Item call_process(derived::Item x) {
  return base::process(x);
}

// Custom-to-custom override should not happen
// CHECK-SEMA-LABEL: define {{.*}}void @_Z12call_processN7derived4ItemE(
// CHECK-SEMA: call {{.*}}void @_ZN4base7processIN7derived4ItemEEET_RKS3_(

// ============================================================================
// Test 5: Non-custom function should not do ADL resolution
// ============================================================================
namespace lib3 {
  template<typename T>
  T compute(const T& x, const T& y) {  // Not custom!
    return x + y;
  }
}

namespace user3 {
  struct Num {
    int n;
    Num operator+(const Num& other) const { return Num{n + other.n}; }
  };

  // Override is present but should not be used because lib3::compute is not custom
  Num compute(const Num& x, const Num& y) {
    return Num{x.n * y.n};
  }
}

user3::Num call_compute(user3::Num x, user3::Num y) {
  return lib3::compute(x, y);
}

// Non-custom functions should not trigger ADL override resolution
// CHECK-SEMA-LABEL: define {{.*}}void @_Z12call_computeN5user33NumES0_(
// CHECK-SEMA: call {{.*}}void @_ZN4lib37computeIN5user33NumEEET_RKS3_S5_(
// CHECK-SEMA-NOT: call {{.*}}void @_ZN5user37computeERKNS_3NumES2_(

// CHECK-NO-SEMA-LABEL: define {{.*}}void @_Z12call_computeN5user33NumES0_(
// CHECK-NO-SEMA: call {{.*}}void @_ZN4lib37computeIN5user33NumEEET_RKS3_S5_(

// ============================================================================
// Test 6: Better match via ADL is preferred over template
// ============================================================================
namespace generic {
  template<typename X, typename Y>
  custom auto inner_product(const X& x, const Y& y) {
    return x * y;
  }
}

namespace gpu {
  struct GPUVector {
    float data[4];
    float dot(const GPUVector& other) const {
      return data[0] * other.data[0];  // simplified
    }
  };

  // Non-template override - should be preferred by overload resolution
  float inner_product(const GPUVector& x, const GPUVector& y) {
    return x.dot(y);
  }
}

float call_inner_product(gpu::GPUVector a, gpu::GPUVector b) {
  return generic::inner_product(a, b);
}

// With sema, the specific gpu::inner_product should be chosen
// CHECK-SEMA-LABEL: define {{.*}}float @_Z18call_inner_productN3gpu9GPUVectorES0_(
// CHECK-SEMA: call {{.*}}float @_ZN3gpu13inner_productERKNS_9GPUVectorES2_(

// Without sema, the generic template wrapper is called
// CHECK-NO-SEMA-LABEL: define {{.*}}float @_Z18call_inner_productN3gpu9GPUVectorES0_(
// CHECK-NO-SEMA: call {{.*}}float @_ZN7generic13inner_productINS_9GPUVectorES1_EEDaRKT_RKT0_(
