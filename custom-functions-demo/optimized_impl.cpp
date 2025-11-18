// Optimized implementations that can replace the default versions
// This file would be compiled separately and linked with new_approach.cpp

#include <vector>
#include <iostream>

namespace rvf {
  // Optimized inner product with loop unrolling
  [[gnu::noinline]]
  double inner_product(std::vector<double> const& v1, std::vector<double> const& v2) {
    std::cout << "[Using OPTIMIZED inner_product]\n";

    double result = 0.0;
    size_t i = 0;
    size_t n = v1.size();

    // Unroll by 4
    for (; i + 3 < n; i += 4) {
      result += v1[i] * v2[i];
      result += v1[i+1] * v2[i+1];
      result += v1[i+2] * v2[i+2];
      result += v1[i+3] * v2[i+3];
    }

    // Handle remaining elements
    for (; i < n; ++i) {
      result += v1[i] * v2[i];
    }

    return result;
  }

  // Optimized AXPY with loop unrolling
  [[gnu::noinline]]
  void axpy(double a, std::vector<double> const& x, std::vector<double>& y) {
    std::cout << "[Using OPTIMIZED axpy]\n";

    size_t i = 0;
    size_t n = x.size();

    // Unroll by 4
    for (; i + 3 < n; i += 4) {
      y[i] += a * x[i];
      y[i+1] += a * x[i+1];
      y[i+2] += a * x[i+2];
      y[i+3] += a * x[i+3];
    }

    // Handle remaining elements
    for (; i < n; ++i) {
      y[i] += a * x[i];
    }
  }
}

// With the custom function mechanism:
// 1. Both files compiled with -fcustomizable-functions
// 2. At link time, the linker can choose which implementation to use
// 3. The .custom directive tells the linker these functions are replaceable
// 4. This happens without any runtime overhead or vtables
