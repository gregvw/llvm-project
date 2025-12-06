# Mangled Name Fix for Customizable Functions

## Critical Bug Fixed

The override discovery mechanism now uses **mangled names** as keys, eliminating namespace/overload/template collision hazards.

## The Problem (Before)

**Attribute stored only unqualified identifier:**
```cpp
PublicFn->addFnAttr("clang-customizable-function", D->getName());
// D->getName() = "compute" (unmangled)
```

**Result:** All `custom` functions with the same unmangled name targeted **ONE** override symbol:
- `namespace1::compute` → `__custom_override_compute`
- `namespace2::compute` → `__custom_override_compute` (**SAME!**)
- `compute(int)` → `__custom_override_compute`
- `compute(double)` → `__custom_override_compute` (**SAME!**)

**Collision hazards:**
1. **Namespace collisions**: `lib1::foo` and `lib2::foo` both map to `__custom_override_foo`
2. **Overload collisions**: `process(int)` and `process(double)` share `__custom_override_process`
3. **Template collisions**: Different instantiations `transform<int>`, `transform<double>` use one override

## The Solution (After)

**Attribute now stores mangled name:**
```cpp
std::string MangledName = getMangledName(GD).str();
PublicFn->addFnAttr("clang-customizable-function", MangledName);
// Also store unmangled name for diagnostics
PublicFn->addFnAttr("clang-customizable-function-name", D->getName());
```

**Result:** Each function gets its **unique** override symbol:
- `namespace1::compute` → `__custom_override__ZN10namespace17computeEi`
- `namespace2::compute` → `__custom_override__ZN10namespace27computeEi` (**DIFFERENT!**)
- `compute(int)` → `__custom_override__Z7computei`
- `compute(double)` → `__custom_override__Z7computed` (**DIFFERENT!**)

## Changes Made

### 1. CodeGen (`clang/lib/CodeGen/CodeGenModule.cpp:6549-6555`)

```cpp
// Before:
PublicFn->addFnAttr("clang-customizable-function", D->getName());

// After:
std::string MangledName = getMangledName(GD).str();
PublicFn->addFnAttr("clang-customizable-function", MangledName);
PublicFn->addFnAttr("clang-customizable-function-name", D->getName());
```

### 2. LTO Pass (`llvm/lib/Transforms/IPO/CustomizableFunctions.cpp:48-61`)

```cpp
// Before:
StringRef LogicalName = Attr.getValueAsString();
std::string OverrideName = ("__custom_override_" + LogicalName).str();

// After:
StringRef MangledName = Attr.getValueAsString();
// Get pretty name for diagnostics
StringRef PrettyName = MangledName;
if (Wrapper.hasFnAttribute("clang-customizable-function-name")) {
  PrettyName = Wrapper.getFnAttribute("clang-customizable-function-name")
                   .getValueAsString();
}
std::string OverrideName = ("__custom_override_" + MangledName).str();
```

### 3. New Tests

**CodeGen tests** verify distinct manglings:
- `customizable-functions-namespace-disambiguation.cpp`
- `customizable-functions-overload-disambiguation.cpp`
- `customizable-functions-template-disambiguation.cpp`

**LTO tests** verify correct override behavior:
- `namespace-override.ll` - Only the intended namespace is overridden
- `overload-override.ll` - Only the intended overload is overridden

## Impact

### Before Fix
```cpp
namespace lib1 { custom int foo(int x); }
namespace lib2 { custom int foo(int x); }

int __custom_override_foo(int x) {
  return x * 10;
}
```
**Result:** Both `lib1::foo` and `lib2::foo` get overridden ❌

### After Fix
```cpp
namespace lib1 { custom int foo(int x); }
namespace lib2 { custom int foo(int x); }

// Override only lib1::foo
int __custom_override__ZN4lib13fooEi(int x) {
  return x * 10;
}
```
**Result:** Only `lib1::foo` is overridden ✅

## User Impact

### For Override Providers

**Before (broken):**
```cpp
custom int process(int x);

// Simple but ambiguous
int __custom_override_process(int x) { ... }
```

**After (correct):**
```cpp
custom int process(int x);

// Must use mangled name
int __custom_override__Z7processi(int x) { ... }
```

**How to get the mangled name:**
```bash
# Compile with -S -emit-llvm and check attributes:
clang++ -std=c++20 -fcustomizable-functions -S -emit-llvm test.cpp -o - | grep clang-customizable-function

# Output:
# attributes #1 = { "clang-customizable-function"="_Z7processi" ... }
```

**Or use c++filt:**
```bash
# Create symbol and demangle to verify:
echo "_Z7processi" | c++filt
# Output: process(int)
```

### Trade-offs

**Pros:**
- ✅ **Correctness**: No more silent miscompilation
- ✅ **Unambiguous**: Each function has unique override
- ✅ **Deterministic**: Mangling is standard ABI
- ✅ **Cross-language**: Works with any C++ ABI-compliant tool

**Cons:**
- ⚠️ **Less convenient**: Must use mangled names
- ⚠️ **ABI-tied**: Override name depends on mangling scheme
- ⚠️ **Readability**: Mangled names are cryptic

## Test Status

**New tests (pass):**
- ✅ `customizable-functions-namespace-disambiguation.cpp`
- ✅ `customizable-functions-overload-disambiguation.cpp`
- ✅ `customizable-functions-template-disambiguation.cpp`
- ✅ `namespace-override.ll`
- ✅ `overload-override.ll`

**Existing tests (need update):**
- ⚠️ 9 CodeGen tests expect old unmangled names
- ⚠️ Need to update CHECK patterns to expect mangled names

**Example update needed:**
```diff
-// CHECK: "clang-customizable-function"="mul"
+// CHECK: "clang-customizable-function"="_Z3mulii"
+// CHECK: "clang-customizable-function-name"="mul"
```

## Documentation for Users

### How to Provide Overrides

1. **Find the mangled name:**
   ```bash
   clang++ -std=c++20 -fcustomizable-functions -S -emit-llvm your_code.cpp -o - \
     | grep '"clang-customizable-function"='
   ```

2. **Create override with mangled name:**
   ```cpp
   // Override process(int) - mangled name: _Z7processi
   int __custom_override__Z7processi(int x) {
     return x * 100;
   }
   ```

3. **Compile and link:**
   ```bash
   clang++ -std=c++20 -fcustomizable-functions your_code.cpp override.cpp
   ```

### Multiple Overloads

```cpp
custom void log(const char* msg);     // Mangled: _Z3logPKc
custom void log(const std::string& s); // Mangled: _Z3logRKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE

// Provide both overrides:
void __custom_override__Z3logPKc(const char* msg) {
  // Custom C-string logging
}

void __custom_override__Z3logRKNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEE(const std::string& s) {
  // Custom std::string logging
}
```

### Cross-Namespace

```cpp
namespace db { custom void close(Connection* c); }    // Mangled: _ZN2db5closeEP10Connection
namespace net { custom void close(Connection* c); }   // Mangled: _ZN3net5closeEP10Connection

// Override only database close:
void __custom_override__ZN2db5closeEP10Connection(Connection* c) {
  // Database-specific cleanup
}
// Network close uses its default implementation
```

## Future Improvements

### Option 1: Tooling Support
Provide utility to generate override stubs:
```bash
$ clang-override-gen mycode.cpp process
Generated override stub:
int __custom_override__Z7processi(int x) {
  // TODO: Implement override
}
```

### Option 2: Pragma/Attribute
Allow specifying override target:
```cpp
[[override_for("process(int)")]]
int my_custom_process(int x) { ... }
```

### Option 3: Link-time Symbol Alias
Use linker features to create readable aliases:
```cpp
int custom_process(int x) { ... }
__attribute__((alias("__custom_override__Z7processi")))
```

## Summary

**What was wrong:** Override symbols used unmangled names, causing collisions

**What changed:** Override symbols now use mangled names for uniqueness

**Result:** Correct, unambiguous override resolution at the cost of convenience

This fix is **critical for correctness** and must be applied before the feature can be considered production-ready.
