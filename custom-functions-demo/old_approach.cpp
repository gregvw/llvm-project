// Old Approach: Using TInCUp tag_invoke for customization
// This is conceptual - shows how RealVectorFramework currently works

#include <vector>
#include <span>
#include <concepts>

// Simplified tag_invoke infrastructure (mimicking TInCUp)
namespace tincup {
  // Tag type for customization point objects
  template<typename CPO>
  struct cpo_tag {};

  // Default fallback - not found
  void tag_invoke() = delete;
}

// Customization point objects
namespace rvf {
  // Inner product CPO
  struct inner_product_fn {
    template<typename V1, typename V2>
      requires requires(V1 const& v1, V2 const& v2) {
        { tag_invoke(tincup::cpo_tag<inner_product_fn>{}, v1, v2) }
          -> std::convertible_to<double>;
      }
    [[nodiscard]] auto operator()(V1 const& v1, V2 const& v2) const {
      return tag_invoke(tincup::cpo_tag<inner_product_fn>{}, v1, v2);
    }
  };

  inline constexpr inner_product_fn inner_product{};

  // AXPY CPO: y = a*x + y
  struct axpy_fn {
    template<typename V1, typename V2>
      requires requires(double a, V1 const& x, V2& y) {
        { tag_invoke(tincup::cpo_tag<axpy_fn>{}, a, x, y) } -> std::same_as<void>;
      }
    void operator()(double a, V1 const& x, V2& y) const {
      tag_invoke(tincup::cpo_tag<axpy_fn>{}, a, x, y);
    }
  };

  inline constexpr axpy_fn axpy{};
}

// Standard vector implementation
namespace rvf {
  // Tag invoke overload for std::vector inner product
  [[nodiscard]] double tag_invoke(
    tincup::cpo_tag<inner_product_fn>,
    std::vector<double> const& v1,
    std::vector<double> const& v2
  ) {
    double result = 0.0;
    for (size_t i = 0; i < v1.size(); ++i) {
      result += v1[i] * v2[i];
    }
    return result;
  }

  // Tag invoke overload for std::vector axpy
  void tag_invoke(
    tincup::cpo_tag<axpy_fn>,
    double a,
    std::vector<double> const& x,
    std::vector<double>& y
  ) {
    for (size_t i = 0; i < x.size(); ++i) {
      y[i] += a * x[i];
    }
  }
}

// Usage example
#include <iostream>

int main() {
  std::vector<double> x = {1.0, 2.0, 3.0};
  std::vector<double> y = {4.0, 5.0, 6.0};

  // Uses customization point
  double dot = rvf::inner_product(x, y);
  std::cout << "Inner product: " << dot << "\n";

  // y = 2*x + y
  rvf::axpy(2.0, x, y);
  std::cout << "After AXPY: [" << y[0] << ", " << y[1] << ", " << y[2] << "]\n";

  return 0;
}
