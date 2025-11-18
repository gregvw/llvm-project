# Comparison: tag_invoke vs Custom Functions

## Overview

This demonstrates two approaches to customization in C++:
1. **TInCUp's tag_invoke**: Compile-time customization using templates and ADL
2. **Custom Functions**: Link-time customization using the new `custom` attribute

## Example: Vector Operations

Both approaches implement the same operations:
- `inner_product(v1, v2)` - compute dot product
- `axpy(a, x, y)` - compute y = a*x + y

## Code Complexity Comparison

### tag_invoke Approach (old_approach.cpp)

```cpp
// Define CPO infrastructure
namespace tincup {
  template<typename CPO>
  struct cpo_tag {};
  void tag_invoke() = delete;
}

// Define customization point object
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

// Provide implementation via ADL
double tag_invoke(
  tincup::cpo_tag<inner_product_fn>,
  std::vector<double> const& v1,
  std::vector<double> const& v2
) {
  // implementation
}

// Usage
double dot = rvf::inner_product(x, y);
```

**Lines of boilerplate**: ~30 per customization point

### Custom Functions Approach (new_approach.cpp)

```cpp
// Define customizable function
double inner_product(std::vector<double> const& v1,
                    std::vector<double> const& v2) {
  // implementation
}

// Usage
double dot = rvf::inner_product(x, y);
```

**Lines of boilerplate**: 0 (just a normal function!)

## Key Differences

| Aspect | tag_invoke | Custom Functions |
|--------|-----------|------------------|
| **Customization Time** | Compile-time | Link-time |
| **Code Complexity** | High (CPO infrastructure) | Low (normal functions) |
| **Compile Time** | Slower (template instantiation) | Faster |
| **Binary Size** | Larger (template bloat) | Smaller |
| **Error Messages** | Complex template errors | Simple function errors |
| **Debugging** | Difficult (template traces) | Easy (normal call stack) |
| **Performance** | Zero overhead | Zero overhead |
| **Flexibility** | Type-based dispatch | Implementation swap |

## Customization Mechanism

### tag_invoke
- Uses Argument-Dependent Lookup (ADL)
- Customization via function overloading
- Selected at compile time based on types
- Each type combination creates new instantiation

### Custom Functions
- Uses `.custom` assembly directive
- Customization via symbol replacement
- Selected at link time
- Same binary code for all uses

## Use Cases

### When tag_invoke is Better
- Need different implementations for different types
- Type-based compile-time dispatch
- Header-only libraries
- Complex type trait requirements

### When Custom Functions are Better
- Single implementation type, multiple backends
- Platform-specific optimizations (SSE, AVX, NEON)
- Plugin architectures
- Dynamic library replacement
- Simpler codebases
- Faster build times

## Real-World Example

### RealVectorFramework with tag_invoke
```cpp
// Must define CPO for each operation
namespace rvf {
  struct clone_fn { /* CPO boilerplate */ };
  inline constexpr clone_fn clone{};

  struct scale_in_place_fn { /* CPO boilerplate */ };
  inline constexpr scale_in_place_fn scale_in_place{};

  struct axpy_in_place_fn { /* CPO boilerplate */ };
  inline constexpr axpy_in_place_fn axpy_in_place{};

  // ... 20+ more operations
}
```

### RealVectorFramework with Custom Functions
```cpp
// Just normal functions
namespace rvf {
  Vector clone(Vector const& v);
  void scale_in_place(double a, Vector& v);
  void axpy_in_place(double a, Vector const& x, Vector& y);
  // ... 20+ more operations
}

// Compile with -fcustomizable-functions
// Linker can swap in optimized implementations
```

## Build Process Comparison

### tag_invoke
```bash
# All customization happens at compile time
clang++ -std=c++20 main.cpp -o program
```

### Custom Functions
```bash
# Compile with -fcustomizable-functions flag
clang++ -std=c++20 -fcustomizable-functions main.cpp -c
clang++ -std=c++20 -fcustomizable-functions optimized.cpp -c

# Link - linker chooses which implementation to use
clang++ main.o optimized.o -o program

# Or use default implementation
clang++ main.o -o program
```

## Migration Path

For existing RealVectorFramework code:

1. **Phase 1**: Keep tag_invoke for type dispatch, add custom functions for backend selection
2. **Phase 2**: Gradually replace CPOs with simple functions
3. **Phase 3**: Full migration to custom functions where appropriate

This allows incremental adoption without breaking existing code.
