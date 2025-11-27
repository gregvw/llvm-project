// RUN: %clang_cc1 -std=c++20 -fcustomizable-functions -fcustomizable-functions-sema \
// RUN:   -emit-llvm -o - %s | FileCheck %s --check-prefix=CHECK

// Test: Verify that 'custom' functions can replicate tag_invoke patterns
// like those used in TInCuP/RealVectorFramework

using size_t = decltype(sizeof(0));

// ============================================================================
// Pattern 1: Library defines generic customization points (like RVF CPOs)
// ============================================================================
namespace lib {
  // Generic 'clone' operation - similar to rvf::clone CPO
  template<typename T>
  custom T clone(const T& x) {
    // Default: copy constructor
    return T(x);
  }

  // Generic 'scale_in_place' operation - similar to rvf::scale_in_place CPO
  template<typename V, typename S>
  custom void scale_in_place(V& vec, S scalar) {
    // Default: iterate and scale (simplified)
    for (auto& elem : vec) {
      elem *= scalar;
    }
  }

  // Generic 'inner_product' - similar to rvf::inner_product CPO
  template<typename V>
  custom auto inner_product(const V& x, const V& y) -> typename V::value_type {
    typename V::value_type sum = 0;
    for (size_t i = 0; i < x.size(); ++i) {
      sum += x[i] * y[i];
    }
    return sum;
  }
}

// ============================================================================
// Pattern 2: User provides custom types and specializations (like RVF does)
// ============================================================================
namespace user {
  // Custom vector type - analogous to providing implementations for std::vector
  struct MyVector {
    double* data;
    size_t len;

    MyVector(size_t n) : len(n) {
      data = new double[n];
      for (size_t i = 0; i < n; ++i) data[i] = 0.0;
    }

    MyVector(const MyVector& other) : len(other.len) {
      data = new double[len];
      for (size_t i = 0; i < len; ++i) data[i] = other.data[i];
    }

    ~MyVector() { delete[] data; }

    using value_type = double;
    size_t size() const { return len; }
    double& operator[](size_t i) { return data[i]; }
    double operator[](size_t i) const { return data[i]; }

    struct iterator {
      double* ptr;
      iterator(double* p) : ptr(p) {}
      iterator& operator++() { ++ptr; return *this; }
      double& operator*() { return *ptr; }
      bool operator!=(const iterator& other) const { return ptr != other.ptr; }
    };

    iterator begin() { return iterator(data); }
    iterator end() { return iterator(data + len); }
  };

  // User-defined customization: optimized clone using memcpy
  // This is analogous to tag_invoke(rvf::clone_ftor, const std::vector<T>&)
  MyVector clone(const MyVector& x) {
    MyVector result(x.len);
    // Optimized implementation using memcpy
    for (size_t i = 0; i < x.len; ++i) {
      result.data[i] = x.data[i];
    }
    return result;
  }

  // User-defined customization: SIMD-friendly scale_in_place
  // This is analogous to tag_invoke(rvf::scale_in_place_ftor, std::vector<T>&, T)
  void scale_in_place(MyVector& vec, double scalar) {
    // Could be SIMD optimized in real implementation
    for (size_t i = 0; i < vec.len; ++i) {
      vec.data[i] *= scalar;
    }
  }

  // User-defined customization: optimized dot product
  // This is analogous to tag_invoke(rvf::inner_product_ftor, ...)
  double inner_product(const MyVector& x, const MyVector& y) {
    double sum = 0.0;
    // Could call BLAS ddot in real implementation
    for (size_t i = 0; i < x.len; ++i) {
      sum += x.data[i] * y.data[i];
    }
    return sum;
  }
}

// ============================================================================
// Pattern 3: Generic algorithm using the customization points (like RVF algorithms)
// ============================================================================
namespace algorithms {
  // Generic AXPY: y = alpha*x + y
  // This is analogous to RVF algorithms that use CPOs
  template<typename V>
  void axpy(double alpha, const V& x, V& y) {
    // Use lib::clone to make a temporary copy
    auto temp = lib::clone(x);

    // Use lib::scale_in_place to scale the temporary
    lib::scale_in_place(temp, alpha);

    // Add to y (simplified - would use add_in_place CPO in real RVF)
    for (size_t i = 0; i < y.size(); ++i) {
      y[i] += temp[i];
    }
  }

  // Generic dot product computation
  template<typename V>
  auto compute_dot(const V& x, const V& y) -> typename V::value_type {
    return lib::inner_product(x, y);
  }
}

// ============================================================================
// Test the pattern
// ============================================================================
void test_customization_pattern() {
  user::MyVector v1(3);
  v1[0] = 1.0; v1[1] = 2.0; v1[2] = 3.0;

  user::MyVector v2(3);
  v2[0] = 4.0; v2[1] = 5.0; v2[2] = 6.0;

  // When called with MyVector, these should resolve to user::clone, etc.
  // due to ADL with -fcustomizable-functions-sema
  auto v3 = lib::clone(v1);
  lib::scale_in_place(v2, 2.0);
  double dot = lib::inner_product(v1, v2);

  // Generic algorithms work with any type
  algorithms::axpy(1.5, v1, v2);
  double result = algorithms::compute_dot(v1, v2);
}

// With -fcustomizable-functions-sema:
// - lib::clone(MyVector) should call user::clone (ADL override)
// - lib::scale_in_place(MyVector&, double) should call user::scale_in_place
// - lib::inner_product(MyVector, MyVector) should call user::inner_product

// CHECK-LABEL: define {{.*}}void @_Z26test_customization_patternv(
// CHECK: call {{.*}} @_ZN4user5cloneERKNS_8MyVectorE(
// CHECK: call {{.*}}void @_ZN4user14scale_in_placeERNS_8MyVectorEd(
// CHECK: call {{.*}}double @_ZN4user13inner_productERKNS_8MyVectorES2_(
