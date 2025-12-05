# Identifier-Only Restriction for Custom Functions

## Problem

CodeGen assumes `custom` function names are simple identifiers when attaching the `"clang-customizable-function"` attribute. It calls `FunctionDecl::getName()`, which **asserts for non-identifiers** (operators, conversion functions, UDLs), causing an Internal Compiler Error (ICE) during CodeGen.

### Before Fix - Would ICE

```cpp
// This would crash the compiler during CodeGen!
custom int operator+(int a, int b) {
    return a + b + 1;
}

custom long double operator""_km(long double x) {
    return x * 1000.0;
}
```

**Error:**
```
Assertion failed: getNameKind() == Identifier && "getName() called on non-identifier"
```

## Why Operators Can't Be Custom (For Now)

1. **CodeGen Crashes**: `getName()` asserts on non-identifiers (`clang/lib/CodeGen/CodeGenModule.cpp:6550`)
2. **Override Naming**: The LTO pass uses `__custom_override_<identifier>` convention, which can't represent operators
3. **Already Customizable**: ADL already applies to operators, so there's less need for `custom`
4. **Experimental Sema**: The override resolution already filters on `Name.isIdentifier()`

## The Fix

Added Sema diagnostic to reject `custom` on non-identifier function names **before** reaching CodeGen.

### Files Modified

**1. `clang/include/clang/Basic/DiagnosticSemaKinds.td`**
```tablegen
def err_custom_requires_identifier : Error<
  "'custom' can only be applied to functions with identifier names, not "
  "%select{operators|conversion functions|user-defined literals}0">;
```

**2. `clang/lib/Sema/SemaDecl.cpp` (around line 10157)**
```cpp
} else if (!NewFD->getDeclName().isIdentifier()) {
  // Check that the function name is a simple identifier
  DeclarationName::NameKind Kind = NewFD->getDeclName().getNameKind();
  unsigned DiagSel;
  if (Kind == DeclarationName::CXXOperatorName)
    DiagSel = 0; // operators
  else if (Kind == DeclarationName::CXXConversionFunctionName)
    DiagSel = 1; // conversion functions
  else // DeclarationName::CXXLiteralOperatorName
    DiagSel = 2; // user-defined literals
  Diag(D.getDeclSpec().getCustomSpecLoc(),
       diag::err_custom_requires_identifier) << DiagSel;
  NewFD->setInvalidDecl();
}
```

**3. `clang/test/SemaCXX/customizable-functions-identifier-only.cpp` (NEW)**

Test file verifying diagnostics for operators and UDLs.

## After Fix - Clean Error

```bash
$ clang++ -std=c++20 -fcustomizable-functions test.cpp
test.cpp:1:1: error: 'custom' can only be applied to functions with identifier names, not operators
custom int operator+(int a, int b) {
^
1 error generated.
```

No more ICE - just a clear diagnostic!

## What Still Works

```cpp
// Regular functions with identifier names - ✓ Works perfectly
custom int add(int a, int b);
custom double multiply(double x, double y);

// Templates - ✓ Works
template<typename T>
custom T max(T a, T b);

// Namespaced functions - ✓ Works
namespace math {
  custom int compute(int x);
}
```

## Test Results

```bash
$ llvm-lit clang/test/SemaCXX/customizable-functions-identifier-only.cpp
PASS: Clang :: SemaCXX/customizable-functions-identifier-only.cpp

# All existing tests still pass
$ ./custom-functions-dev.sh test all
All customizable functions tests: PASS (21 tests)
```

## Future Considerations

### Possible Future Extensions

If we want to support operators in the future, we would need to:

1. **Change CodeGen**: Use `DeclarationName::getAsString()` instead of `getName()`
2. **Change Override Convention**: Support mangled names in override lookup
3. **Update LTO Pass**: Handle operator names in override resolution

### Why Not Do This Now?

- ADL already provides customization for operators
- The feature is experimental - start simple
- Can always extend later without breaking changes
- Focus on the common case (identifier functions)

## Impact

### Before
- ❌ Compiler crashes (ICE) on `custom` operators/UDLs
- ❌ Confusing assertion failure
- ❌ No way for users to know what's wrong

### After
- ✅ Clear diagnostic error message
- ✅ Indicates exactly what's not supported
- ✅ Prevents ICE in CodeGen
- ✅ Aligns Sema checking with CodeGen/LTO capabilities

## Summary

**What was wrong:** CodeGen called `getName()` on potentially non-identifier function names, causing assertion failure

**What we changed:** Added Sema diagnostic to reject `custom` on operators, conversion functions, and UDLs

**Result:** Clean error message instead of ICE, preventing invalid code from reaching CodeGen

This is a **critical safety fix** that prevents compiler crashes while the feature is experimental.
