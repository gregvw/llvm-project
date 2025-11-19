# Custom Function Specifier Implementation Summary

## Overview

This document summarizes the implementation of the `custom` function specifier for C++, which provides a contextual keyword for marking functions as customizable for backend optimization and replacement strategies.

## Feature Description

The `custom` specifier is a **contextual keyword** that:
- Acts as a function specifier (like `inline`, `virtual`, `explicit`)
- Only has special meaning in declaration-specifier context
- Remains available as an identifier elsewhere for backward compatibility
- Is enabled via the `-fcustomizable-functions` compiler flag

## Implementation Components

### 1. Language Option

**File**: `clang/include/clang/Basic/LangOptions.def`
- Added `CustomizableFunctions` language option (line 504)
- Enabled by `-fcustomizable-functions` flag

### 2. DeclSpec Changes

**File**: `clang/include/clang/Sema/DeclSpec.h`
- Added `FS_custom_specified` bit field (line 372)
- Added `FS_customLoc` source location (line 416)
- Added `isCustomSpecified()` getter (line 636)
- Added `setFunctionSpecCustom()` setter declaration (line 788)
- Updated `ClearFunctionSpecs()` to clear custom flag (lines 651-652)

**File**: `clang/lib/Sema/DeclSpec.cpp`
- Implemented `setFunctionSpecCustom()` method (lines 1081-1094)
- Handles duplicate specifier warnings

### 3. AST Representation

**File**: `clang/include/clang/AST/DeclBase.h`
- Added `IsCustom` bit to `FunctionDeclBitfields` (line 1835)
- Updated `NumFunctionDeclBits` from 32 to 33 (line 1839)

**File**: `clang/include/clang/AST/Decl.h`
- Added `isCustom()` getter (line 2925)
- Added `setCustom()` setter (line 2928)

### 4. Parser Implementation

**File**: `clang/lib/Parse/ParseDecl.cpp`
- Added contextual keyword recognition in `ParseDeclarationSpecifiers` (lines 3753-3759)
- Checks for:
  - C++ mode
  - `CustomizableFunctions` language option enabled
  - Identifier token with spelling "custom"
- Only recognizes `custom` in declaration-specifier context

### 5. Semantic Validation

**File**: `clang/lib/Sema/SemaDecl.cpp`

**Member Function Checks** (lines 10116-10132):
- Rejects `custom` on constructors
- Rejects `custom` on destructors
- Rejects `custom` on member functions

**Inline Compatibility Check** (lines 10090-10097):
- Rejects combination of `custom` and `inline`

**Flag Setting** (lines 10149-10154):
- Sets `isCustom()` flag on valid free function declarations

### 6. Diagnostics

**File**: `clang/include/clang/Basic/DiagnosticSemaKinds.td`
- `err_custom_non_function` (line 2173)
- `err_custom_on_special_member` (lines 2175-2176)
- `err_custom_on_member_function` (lines 2177-2178)
- `err_custom_with_inline` (lines 2179-2180)

### 7. Documentation

**File**: `clang/docs/CustomizableFunctions.rst`
- Comprehensive documentation of the feature
- Syntax examples
- Restriction explanations
- Use cases and future directions

**File**: `clang/docs/ReleaseNotes.rst`
- Added feature announcement in "C++ Language Changes" section (lines 189-197)

**File**: `clang/docs/index.rst`
- Added `CustomizableFunctions` to documentation tree (line 22)

**File**: `clang/docs/LanguageExtensions.rst`
- Added `CustomizableFunctions` to language extensions toctree (line 19)

## Design Decisions

### Contextual Keyword Approach

We chose to make `custom` a contextual keyword rather than a hard keyword:

**Pros**:
- Preserves backward compatibility with existing code using `custom` as identifier
- Follows modern C++ design patterns (similar to `final`, `override`)
- Allows gradual adoption without breaking existing codebases

**Cons**:
- Slightly more complex parser implementation
- Potential ambiguity in edge cases (mitigated by context-sensitive parsing)

### Free Functions Only

The initial implementation restricts `custom` to free functions:

**Rationale**:
- Simpler semantics and implementation
- Member functions have complex interactions with virtual dispatch
- Constructors/destructors have special initialization/cleanup semantics
- Can be extended to member functions in future if needed

### Mutual Exclusion with inline

The `custom` and `inline` specifiers are mutually exclusive:

**Rationale**:
- `inline` suggests the function body should be inlined at call sites
- `custom` suggests the function may be replaced or modified externally
- These concepts are fundamentally incompatible
- Clear error prevents confusing optimization behavior

## Usage Example

```cpp
// Enable the feature
// clang++ -fcustomizable-functions example.cpp

// Valid: custom free function
custom void processData(int* data, size_t size);

// Valid: with constexpr
custom constexpr int computeValue() { return 42; }

// Invalid: on member function
struct MyClass {
  custom void method();  // Error: only free functions allowed
};

// Invalid: with inline
custom inline void foo();  // Error: mutually exclusive

// Valid: custom can still be used as identifier
int custom = 10;  // OK
struct custom { int x; };  // OK
```

## Testing

**Test File**: `test_custom.cpp`
- Tests valid usage on free functions
- Tests rejection on member functions
- Tests rejection on constructors/destructors
- Tests rejection of `custom inline` combination

## Future Enhancements

Potential future work includes:

1. **Member Function Support**: Allow `custom` on member functions with explicit opt-in
2. **Module Integration**: Ensure correct behavior with C++20 modules
3. **ABI Considerations**: Define standardized ABI for customization mechanisms
4. **Backend Integration**: Connect to LLVM IR attributes and optimization passes
5. **Feature Detection**: Add `__has_feature(customizable_functions)` support

## Integration Points

### CodeGen
The `isCustom()` flag on `FunctionDecl` should be propagated to:
- LLVM IR function attributes
- Object file annotations
- Debug information

### Linker
The linker should recognize customizable functions and:
- Allow multiple definitions (with appropriate ODR handling)
- Support symbol replacement strategies
- Maintain metadata for runtime patching

## References

- C++ Function Specifiers: [ISO C++ Standard §9.2.3]
- Contextual Keywords: [ISO C++ Standard §5.11]
- Clang AST: https://clang.llvm.org/docs/IntroductionToTheClangAST.html
- Clang Parser: https://clang.llvm.org/docs/HowToSetupToolingForLLVM.html

## Commit Information

**Branch**: `claude/add-custom-specifier-01RpdbysckXoep87ka238LBB`
**Commit Hash**: 058677009
**Date**: 2025-11-19

### Files Modified
1. `clang/include/clang/Basic/LangOptions.def`
2. `clang/include/clang/Sema/DeclSpec.h`
3. `clang/lib/Sema/DeclSpec.cpp`
4. `clang/include/clang/AST/DeclBase.h`
5. `clang/include/clang/AST/Decl.h`
6. `clang/lib/Parse/ParseDecl.cpp`
7. `clang/lib/Sema/SemaDecl.cpp`
8. `clang/include/clang/Basic/DiagnosticSemaKinds.td`

### Documentation Added
1. `clang/docs/CustomizableFunctions.rst`
2. `clang/docs/ReleaseNotes.rst` (updated)
3. `clang/docs/index.rst` (updated)
4. `clang/docs/LanguageExtensions.rst` (updated)

## Build Instructions

```bash
# Configure CMake
cmake -S llvm -B build -G Ninja \
  -DLLVM_ENABLE_PROJECTS=clang \
  -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build

# Test
./build/bin/clang++ -fcustomizable-functions test_custom.cpp
```

## Contact

For questions or issues related to this implementation, please refer to:
- GitHub Issues: https://github.com/gregvw/llvm-project/issues
- Clang Development List: cfe-dev@lists.llvm.org
