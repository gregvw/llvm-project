# Customizable Functions vs tag_invoke Pattern

This document demonstrates that LLVM's `custom` functions with `-fcustomizable-functions-sema` provide equivalent capabilities to the `tag_invoke` pattern used in TInCuP and RealVectorFramework.

## Background

### TInCuP/RealVectorFramework Pattern

TInCuP and RealVectorFramework use the **tag_invoke** pattern for customization points:

1. **Library defines Customization Point Objects (CPOs)**:
   ```cpp
   namespace rvf {
     inline constexpr struct clone_ftor final : tincup::cpo_base<clone_ftor> {
       template<typename V>
       constexpr auto operator()(const V& x) const {
         return tincup::tag_invoke_cpo(*this, x);  // Calls tag_invoke via ADL
       }
     } clone;
   }
   ```

2. **Users customize via ADL-visible tag_invoke overloads**:
   ```cpp
   namespace user {
     struct MyVector { /* ... */ };

     // Customization found via ADL
     auto tag_invoke(rvf::clone_ftor, const MyVector& x) -> MyVector {
       return MyVector(x);  // Custom implementation
     }
   }
   ```

3. **Generic algorithms use CPOs**:
   ```cpp
   template<typename V>
   void algorithm(const V& x) {
     auto copy = rvf::clone(x);  // Finds user::tag_invoke via ADL
   }
   ```

## Customizable Functions Equivalent

With `custom` functions and `-fcustomizable-functions-sema`, we achieve the same pattern:

### 1. Library Defines Customization Points

```cpp
namespace lib {
  // Generic customization point (like RVF CPO)
  template<typename T>
  custom T clone(const T& x) {
    return T(x);  // Default implementation
  }

  template<typename V, typename S>
  custom void scale_in_place(V& vec, S scalar) {
    for (auto& elem : vec) {
      elem *= scalar;  // Default implementation
    }
  }
}
```

### 2. Users Customize via ADL-Visible Overrides

```cpp
namespace user {
  struct MyVector { /* ... */ };

  // Customization found via ADL (just like tag_invoke)
  MyVector clone(const MyVector& x) {
    return MyVector(x);  // Custom implementation
  }

  void scale_in_place(MyVector& vec, double scalar) {
    // Optimized implementation (e.g., SIMD, BLAS, etc.)
    for (size_t i = 0; i < vec.len; ++i) {
      vec.data[i] *= scalar;
    }
  }
}
```

### 3. Generic Algorithms Use Customization Points

```cpp
namespace algorithms {
  template<typename V>
  void axpy(double alpha, const V& x, V& y) {
    auto temp = lib::clone(x);           // Finds user::clone via ADL
    lib::scale_in_place(temp, alpha);    // Finds user::scale_in_place via ADL
    // ... rest of algorithm
  }
}
```

## Verification

The test in `clang/test/CodeGenCXX/customizable-functions-rvf-comparison.cpp` verifies that:

**With `-fcustomizable-functions-sema`:**
- `lib::clone(user::MyVector)` → calls `user::clone` (ADL override)
- `lib::scale_in_place(user::MyVector&, double)` → calls `user::scale_in_place` (ADL override)
- `lib::inner_product(user::MyVector, user::MyVector)` → calls `user::inner_product` (ADL override)

**Generated IR confirms this:**
```llvm
call void @_ZN4user5cloneERKNS_8MyVectorE(...)           ; user::clone
call void @_ZN4user14scale_in_placeERNS_8MyVectorEd(...) ; user::scale_in_place
call double @_ZN4user13inner_productERKNS_8MyVectorES2_(...) ; user::inner_product
```

## Key Advantages

Both approaches solve the same problems identified in WG21 papers P1895R0 and P2279R0:

| Problem | tag_invoke Solution | custom Functions Solution |
|---------|-------------------|--------------------------|
| Namespace pollution | Tag-based dispatch isolates names | `custom` keyword isolates behavior |
| Library composition | Tag types prevent conflicts | Function templates in different namespaces |
| ADL customization | Explicit via `tag_invoke` | Automatic via ADL + Sema resolution |
| Boilerplate | Requires CPO infrastructure | Built into language |

## Comparison Summary

| Aspect | TInCuP/RVF (tag_invoke) | Customizable Functions |
|--------|------------------------|----------------------|
| **Library definition** | CPO struct with `operator()` | `custom` function template |
| **User customization** | `tag_invoke` overload | Regular function overload |
| **Discovery mechanism** | ADL on `tag_invoke` | ADL with Sema enhancement |
| **Infrastructure needed** | TInCuP library (~1000+ LoC) | Compiler built-in |
| **Runtime overhead** | Zero (inline CPOs) | Zero (direct calls) |
| **Compile-time overhead** | Template instantiations | Template instantiations |

## Conclusion

**Yes, we have equivalent capability!** The `custom` functions with `-fcustomizable-functions-sema` provide the same functionality as TInCuP's `tag_invoke` pattern but with:

1. **Less boilerplate** - No need to define CPO structs and tag types
2. **Native language support** - Built into the compiler rather than library-based
3. **Same semantics** - ADL-based customization point discovery
4. **Same guarantees** - Compile-time dispatch, zero runtime overhead

The main difference is syntactic: instead of writing `tag_invoke(cpo_tag, args...)`, users write regular function overloads that are automatically discovered via ADL when the library's `custom` function is called.

## Example Use Cases

Both approaches enable:

✅ **Generic numerical libraries** (like RealVectorFramework)
✅ **Backend-agnostic algorithms** (CPU, GPU, distributed)
✅ **Non-intrusive customization** (no modification of third-party types)
✅ **Zero-overhead abstractions** (compile-time polymorphism)
✅ **Safe library composition** (no global namespace pollution)

The `custom` functions approach makes these patterns accessible without requiring external libraries or boilerplate infrastructure.
