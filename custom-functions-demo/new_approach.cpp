// New Approach: Using custom function attribute for customization
// This shows how we can replace tag_invoke with simpler direct function calls

#include <vector>
#include <span>
#include <iostream>

namespace rvf {
  // Customizable functions that can be replaced at link time
  // When compiled with -fcustomizable-functions, these automatically get the
  // LLVM 'custom' attribute, which emits .custom directives in assembly

  // Inner product - default implementation for std::vector
  // NOTE: Must not be inline for customization to work
  __attribute__((noinline))
  double inner_product(std::vector<double> const& v1, std::vector<double> const& v2) {
    double result = 0.0;
    for (size_t i = 0; i < v1.size(); ++i) {
      result += v1[i] * v2[i];
    }
    return result;
  }

  // AXPY: y = a*x + y - default implementation for std::vector
  // NOTE: Must not be inline for customization to work
  __attribute__((noinline))
  void axpy(double a, std::vector<double> const& x, std::vector<double>& y) {
    for (size_t i = 0; i < x.size(); ++i) {
      y[i] += a * x[i];
    }
  }
}

// When compiled with -fcustomizable-functions:
// 1. All non-inline functions get the LLVM 'custom' attribute
// 2. The backend emits '.custom <function_name>' directives in assembly
// 3. The linker can replace these functions with alternative implementations
// 4. This happens at link time without any runtime overhead

// A user could provide an optimized version in a separate compilation unit:
// (This would be in a different .cpp file, compiled separately)
/*
namespace rvf {
  // Optimized SIMD version
  double inner_product(std::vector<double> const& v1, std::vector<double> const& v2) {
    // ... AVX2/NEON implementation ...
  }

  void axpy(double a, std::vector<double> const& x, std::vector<double>& y) {
    // ... vectorized implementation ...
  }
}
*/

// Usage example - looks like regular function calls!
int main() {
  std::vector<double> x = {1.0, 2.0, 3.0};
  std::vector<double> y = {4.0, 5.0, 6.0};

  // Simple function calls - no CPO machinery needed
  double dot = rvf::inner_product(x, y);
  std::cout << "Inner product: " << dot << "\n";

  // y = 2*x + y
  rvf::axpy(2.0, x, y);
  std::cout << "After AXPY: [" << y[0] << ", " << y[1] << ", " << y[2] << "]\n";

  return 0;
}

// Advantages of custom functions approach:
// 1. Simpler code - just regular function calls
// 2. No complex template machinery or concepts
// 3. Faster compile times
// 4. Easier to understand and debug
// 5. Link-time customization instead of compile-time
// 6. Better error messages
// 7. Can provide default implementations easily
