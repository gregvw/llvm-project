# Custom Function Specifier Implementation Summary

## Overview

This document summarizes the implementation of the `custom` function specifier for C++, which provides a contextual keyword for marking functions as customizable for backend optimization and replacement strategies.

## Feature Description

The `custom` specifier is a **contextual keyword** that:
- Acts as a function specifier (like `inline`, `virtual`, `explicit`)
- Only has special meaning in declaration-specifier context
- Remains available as an identifier elsewhere for backward compatibility
- Is enabled via the `-fcustomizable-functions` compiler flag

## Current Status

**Phase 1 (Frontend & CodeGen): ✅ COMPLETE**

The implementation has successfully completed the first phase:

1. ✅ **Parser**: Contextual keyword recognition in declaration-specifier context
2. ✅ **AST**: `isCustom()` flag on `FunctionDecl`
3. ✅ **Sema**: Validation (free functions only, no inline, error diagnostics)
4. ✅ **CodeGen**: Canonical IR representation with two-function pattern
5. ✅ **Tests**: 10 comprehensive test files (9 CodeGen + 1 Sema)
6. ✅ **Documentation**: Full feature documentation and release notes
7. ✅ **Tooling**: Development script for building and testing

**Phase 2 (LTO Pass): ✅ COMPLETE**

The LTO pass discovers and replaces customizable functions at link time.

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

### 8. CodeGen Implementation

**Status**: ✅ Complete (Phase 1 - Canonical IR Representation)

#### Overview

The CodeGen implementation establishes a **stable canonical IR representation** for customizable functions. This two-function pattern provides the contract between Clang's frontend and future LLVM optimization passes.

#### Canonical IR Structure

For each `custom` function, CodeGen emits **two LLVM functions**:

1. **Public Interface** (`@foo`):
   - Linkage: `linkonce_odr` (for ODR deduplication across TUs)
   - Visibility: Default (externally visible)
   - Body: Tail-calls the default implementation
   - Attribute: `"clang-customizable-function"="<name>"` for LTO pass discovery
   - Metadata: Tagged in `!clang.customizable` module metadata

2. **Default Implementation** (`@foo.default`):
   - Linkage: `internal` (hidden from linker, per-TU scope)
   - Visibility: Hidden
   - Body: Contains the actual function implementation
   - Metadata: Tagged in `!clang.custom.default` module metadata

#### Example IR Output

For this C++ code:
```cpp
custom int add(int a, int b) {
  return a + b;
}
```

Clang generates:
```llvm
; Public interface (customization point)
define linkonce_odr i32 @_Z3addii(i32 noundef %a, i32 noundef %b) #0 {
entry:
  %call = tail call i32 @_Z3addii.default(i32 noundef %a, i32 noundef %b)
  ret i32 %call
}

; Default implementation
define internal i32 @_Z3addii.default(i32 noundef %a, i32 noundef %b) {
entry:
  %add = add nsw i32 %b, %a
  ret i32 %add
}

attributes #0 = { "clang-customizable-function"="add" }

!clang.customizable = !{!0}
!clang.custom.default = !{!1}
!0 = !{ptr @_Z3addii}
!1 = !{ptr @_Z3addii.default}
```

#### Implementation Details

**File**: `clang/include/clang/CodeGen/CodeGenModule.h`
- Added `EmitCustomizableFunctionDefinition` method declaration (lines 1896-1898)

**File**: `clang/lib/CodeGen/CodeGenModule.cpp`

**Detection Logic** (lines 6451-6455 in `EmitGlobalFunctionDefinition`):
```cpp
// Handle customizable functions specially
if (D->isCustom()) {
  EmitCustomizableFunctionDefinition(GD, FI, Ty);
  return;
}
```

**Emission Logic** (lines 6518-6594 in `EmitCustomizableFunctionDefinition`):

**Step 1**: Create default implementation function
- Construct mangled name with `.default` suffix
- Use `internal` linkage for per-TU scope
- Copy all original attributes and calling conventions

**Step 2**: Emit function body into default implementation
- Reuse existing `CodeGenFunction` machinery
- Generates complete implementation IR

**Step 3**: Create public interface function
- Use `linkonce_odr` linkage for ODR compliance
- Add `"clang-customizable-function"` attribute with unmangled name
- Mark as `uwtable` for exception handling compatibility

**Step 4**: Generate wrapper body
- Create basic block
- Forward all parameters to default implementation
- Use `tail call` for zero-overhead forwarding
- Return result (or void for void functions)

**Step 5**: Add module metadata
- Tag public function in `!clang.customizable` list
- Tag default function in `!clang.custom.default` list
- Enables future pass discovery and validation

#### Design Rationale

**Why LinkOnceODR for public interface?**
- Allows same function defined in multiple TUs (header-only libraries)
- Linker deduplicates to single definition per ODR
- Future LTO pass can replace/customize at link time

**Why Internal for default implementation?**
- Prevents linker visibility and symbol conflicts
- Each TU keeps its own default copy
- LTO pass can inline or eliminate as needed

**Why tail call?**
- Zero runtime overhead for wrapper indirection
- Optimizes to direct call in non-customized case
- Maintains performance parity with non-custom functions

**Why function attribute for discovery?**
- LTO pass can quickly identify customizable functions
- Carries unmangled name for diagnostics
- No need to parse complex metadata graphs

#### Compatibility with C++ Features

The CodeGen implementation correctly handles:

- **Templates**: Each instantiation gets independent wrapper+default pair
- **Overloads**: Each overload treated as separate customizable function
- **Namespaces**: Name mangling preserves namespace scope
- **Constexpr**: Constexpr evaluation happens on default implementation
- **Noexcept**: Exception specifications copied to both functions
- **Trailing return types**: Modern C++ syntax fully supported
- **Complex arguments**: References, structs, templates work correctly

### 9. LTO Pass Implementation ✨ NEW

**File**: `llvm/lib/Transforms/IPO/CustomizableFunctions.cpp`
- Implemented `CustomizableFunctionsPass` module pass
- Discovers functions with `"clang-customizable-function"` attribute
- Finds override functions matching `__custom_override_<name>`
- Validates signature compatibility
- Replaces wrapper body with tail call to override
- Propagates attributes from override to call site

**File**: `llvm/lib/Passes/PassBuilderPipelines.cpp`
- Added pass to `buildLTODefaultPipeline` (Full LTO)
- Added pass to `buildThinLTODefaultPipeline` (Thin LTO)

**File**: `clang/lib/Driver/ToolChains/Clang.cpp`
- Updates driver to pass `-fcustomizable-functions` to cc1

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

### Comprehensive Test Suite

The implementation includes **10 test files** covering all aspects of the feature:

#### CodeGen Tests (9 files in `clang/test/CodeGenCXX/`)

These tests verify the canonical IR representation using FileCheck:

1. **`customizable-functions-basic.cpp`**
   - Tests basic `int` return type
   - Verifies wrapper + default function structure
   - Checks `linkonce_odr` and `internal` linkage
   - Validates `"clang-customizable-function"` attribute
   - Confirms tail call optimization
   - Validates module metadata

2. **`customizable-functions-void.cpp`**
   - Tests `void` return type
   - Ensures proper handling of no-return-value case

3. **`customizable-functions-constexpr.cpp`**
   - Tests `custom constexpr` combination
   - Verifies constexpr doesn't conflict with custom
   - Constexpr's implicit inline is allowed with custom

4. **`customizable-functions-noexcept.cpp`**
   - Tests `custom` with `noexcept` specifier
   - Verifies exception specifications propagate correctly

5. **`customizable-functions-template.cpp`**
   - Tests function templates: `template<class T> custom T id(T x)`
   - Verifies each instantiation gets own wrapper+default pair
   - Checks mangling for template instantiations

6. **`customizable-functions-overload.cpp`**
   - Tests function overloading with `custom`
   - Verifies each overload treated independently
   - Checks distinct mangled names

7. **`customizable-functions-namespace.cpp`**
   - Tests `custom` in namespace scope
   - Verifies correct name mangling with namespace prefix

8. **`customizable-functions-trailing-return.cpp`**
   - Tests modern C++: `custom auto get() -> int`
   - Verifies trailing return type syntax works

9. **`customizable-functions-complex-args.cpp`**
   - Tests reference parameters: `Point& p1, Point& p2`
   - Tests struct arguments
   - Verifies proper parameter forwarding in wrapper

#### Semantic Tests (1 file in `clang/test/SemaCXX/`)

10. **`customizable-functions-errors.cpp`**
    - Tests `custom inline` rejection (mutually exclusive)
    - Tests `custom` on member functions (rejected)
    - Tests `custom` on constructors (rejected)
    - Tests `custom` on destructors (rejected)
    - Verifies all error diagnostics work correctly

### Test Execution

A development script `custom-functions-dev.sh` provides convenient test execution:

```bash
# Run all tests
./custom-functions-dev.sh test all

# Run only CodeGen tests
./custom-functions-dev.sh test codegen

# Run only Sema tests
./custom-functions-dev.sh test sema

# Run specific test by pattern
./custom-functions-dev.sh test template
```

### Test Coverage Matrix

| Feature | CodeGen Test | Sema Test | Status |
|---------|--------------|-----------|--------|
| Basic int return | ✅ basic.cpp | - | ✅ |
| Void return | ✅ void.cpp | - | ✅ |
| With constexpr | ✅ constexpr.cpp | - | ✅ |
| With noexcept | ✅ noexcept.cpp | - | ✅ |
| Templates | ✅ template.cpp | - | ✅ |
| Overloads | ✅ overload.cpp | - | ✅ |
| Namespaces | ✅ namespace.cpp | - | ✅ |
| Trailing return | ✅ trailing-return.cpp | - | ✅ |
| Complex args | ✅ complex-args.cpp | - | ✅ |
| Error: inline | - | ✅ errors.cpp | ✅ |
| Error: member | - | ✅ errors.cpp | ✅ |
| Error: ctor | - | ✅ errors.cpp | ✅ |
| Error: dtor | - | ✅ errors.cpp | ✅ |

## Implementation Status

### Phase 1: Frontend & CodeGen ✅ COMPLETE

- ✅ Contextual keyword parsing
- ✅ AST representation (`isCustom()` flag)
- ✅ Semantic validation (free functions only, no inline)
- ✅ Diagnostic messages
- ✅ Documentation
- ✅ **CodeGen: Canonical IR representation**
- ✅ **Comprehensive test suite (10 files)**

### Phase 2: LTO Pass ✅ COMPLETE

**Goal**: Implement `CustomizableFunctionsPass` in LLVM

**Tasks**:
1. ✅ Create new ModulePass in `llvm/lib/Transforms/IPO/`
2. ✅ Discover customizable functions via `"clang-customizable-function"` attribute
3. ✅ Find override functions (same signature, different name)
4. ✅ Validate signature compatibility
5. ✅ Replace calls from `@foo` → `@foo.override`
6. ✅ Add pass to default LTO pipeline
7. ✅ End-to-end testing with override functions

**IR Contract** (already established by CodeGen):
- Public interface: `linkonce_odr` with attribute
- Default impl: `internal` with `.default` suffix
- Module metadata for discovery

### Phase 2b: Hardening ✅ COMPLETE

**Goal**: Make existing behavior bullet-proof with comprehensive edge-case handling.

**Tasks**:
1. ✅ **Signature mismatch behavior**: When override signature doesn't match wrapper, keep calling `.default`
   - Test: `llvm/test/Transforms/CustomizableFunctions/signature-mismatch.ll`
2. ✅ **Multiple wrappers, same logical name**: Only rewrite wrappers with matching function type
   - Test: `llvm/test/Transforms/CustomizableFunctions/multiple-wrappers.ll`
3. ✅ **No-op on missing override**: Pass leaves body unchanged when no override exists
   - Test: `llvm/test/Transforms/CustomizableFunctions/no-override.ll`
4. ✅ **Pipeline placement tests**: Verify pass runs in both LTO and ThinLTO, before inliner
   - Test: `llvm/test/Other/customizable-functions-pipeline.ll`
5. ✅ **No-flag behavior test**: Verify `custom` is treated as identifier without `-fcustomizable-functions`
   - Test: `clang/test/SemaCXX/customizable-functions-no-flag.cpp`
6. ✅ **Feature detection**: Added `__has_feature(customizable_functions)` and `__has_extension(customizable_functions)`
   - Test: `clang/test/Preprocessor/has_feature_customizable_functions.cpp`
7. ✅ **AST tooling**: Added `custom` to `-ast-dump` and `-ast-print` output
   - Test: `clang/test/AST/customizable-functions-ast-dump.cpp`

### Phase 3 (Experimental): Sema-Level Override Hooks ✅ COMPLETE

**Goal**: Introduce infrastructure for Sema-level override resolution, gated behind an experimental flag.

**Tasks**:
1. ✅ **Add `-fcustomizable-functions-sema` flag**: Gates experimental Sema-level override resolution
2. ✅ **Introduce `TryResolveCustomOverride` helper**: Single choke point for custom function resolution
3. ✅ **Implement `_override` naming convention**: For testing, looks for `<name>_override` in same context
4. ✅ **Hook into `BuildResolvedCallExpr`**: Intercepts calls to custom functions

**Usage**:
```cpp
// With -fcustomizable-functions-sema, calls to foo() are redirected to foo_override()
custom int foo(int x) { return x + 1; }
int foo_override(int x) { return x + 100; }
```

**Design Notes**:
- Completely independent of LTO override mechanism
- Only activates with `-fcustomizable-functions-sema` flag
- Uses standard overload resolution (silent, no diagnostics on mismatch)
- Falls back to original function if no viable override found

### Phase 4: Advanced Features 📋 FUTURE

Potential enhancements for later:

1. **Tag Invoke Support**: Per-type customization via ADL
   - `custom<T>` syntax for type-specific behavior
   - Integration with C++20 concepts
   - CPO (Customization Point Object) pattern

2. **Member Function Support**: Allow `custom` on member functions
   - Interaction with virtual dispatch
   - Explicit opt-in to avoid confusion

3. **Module Integration**: Ensure correct behavior with C++20 modules
   - Module boundary semantics
   - Import/export of customizable functions

4. **ABI Standardization**: Define stable ABI for customization
   - Cross-compiler compatibility
   - Version compatibility

5. **Richer Override Naming**: ADL-based lookup, namespace-qualified names, etc.

## Integration Points

### CodeGen ✅ IMPLEMENTED
The `isCustom()` flag on `FunctionDecl` is now propagated to:
- ✅ LLVM IR function attributes (`"clang-customizable-function"="<name>"`)
- ✅ Module metadata (`!clang.customizable`, `!clang.custom.default`)
- ✅ Two-function IR structure (public interface + default implementation)

See Section 8 for complete CodeGen implementation details.

### LTO Pass ✅ IMPLEMENTED
The CustomizableFunctionsPass:
- Discovers customizable functions via `"clang-customizable-function"` attribute
- Finds override functions matching `__custom_override_<name>`
- Validates signature compatibility before rewriting
- Replaces wrapper body with tail call to override

### Linker
The linker (via LTO) will:
- Deduplicate `linkonce_odr` public interfaces across TUs
- Allow override functions to replace default implementations
- Support whole-program optimization of customizable functions

## References

- C++ Function Specifiers: [ISO C++ Standard §9.2.3]
- Contextual Keywords: [ISO C++ Standard §5.11]
- Clang AST: https://clang.llvm.org/docs/IntroductionToTheClangAST.html
- Clang Parser: https://clang.llvm.org/docs/HowToSetupToolingForLLVM.html

## Commit Information

**Branch**: `claude/add-custom-specifier-01RpdbysckXoep87ka238LBB`
**Latest Commit**: ef0ba45fa (Implement canonical IR representation for custom functions)
**Date**: 2025-11-19

### Files Modified

#### Frontend Implementation
1. `clang/include/clang/Basic/LangOptions.def` - Added CustomizableFunctions option
2. `clang/include/clang/Sema/DeclSpec.h` - Added FS_custom_specified
3. `clang/lib/Sema/DeclSpec.cpp` - Implemented setFunctionSpecCustom()
4. `clang/include/clang/AST/DeclBase.h` - Added IsCustom bit
5. `clang/include/clang/AST/Decl.h` - Added isCustom()/setCustom()
6. `clang/lib/Parse/ParseDecl.cpp` - Contextual keyword parsing
7. `clang/lib/Sema/SemaDecl.cpp` - Semantic validation
8. `clang/include/clang/Basic/DiagnosticSemaKinds.td` - Error diagnostics
9. `clang/lib/Driver/ToolChains/Clang.cpp` ✨ NEW - Driver flag handling

#### CodeGen Implementation
10. `clang/include/clang/CodeGen/CodeGenModule.h` (lines 1896-1898) - Added EmitCustomizableFunctionDefinition
11. `clang/lib/CodeGen/CodeGenModule.cpp` (lines 6451-6455, 6518-6594) - Canonical IR emission

#### LTO Pass Implementation ✨ NEW
12. `llvm/lib/Transforms/IPO/CustomizableFunctions.cpp` - The LTO pass
13. `llvm/lib/Passes/PassBuilderPipelines.cpp` - Pipeline registration

#### Phase 2b Hardening ✨ NEW
14. `clang/include/clang/Basic/Features.def` - Added customizable_functions extension
15. `clang/lib/AST/TextNodeDumper.cpp` - Added custom to -ast-dump
16. `clang/lib/AST/DeclPrinter.cpp` - Added custom to -ast-print

#### Phase 3 Sema Override ✨ NEW
17. `clang/include/clang/Basic/LangOptions.def` - Added CustomizableFunctionsSema option
18. `clang/include/clang/Options/Options.td` - Added -fcustomizable-functions-sema flag
19. `clang/include/clang/Sema/Sema.h` - Added TryResolveCustomOverride declaration
20. `clang/lib/Sema/SemaOverload.cpp` - TryResolveCustomOverride implementation
21. `clang/lib/Sema/SemaExpr.cpp` - Hooked into BuildResolvedCallExpr

### Documentation Added
1. `clang/docs/CustomizableFunctions.rst` - Feature documentation
2. `clang/docs/ReleaseNotes.rst` (updated) - Release notes
3. `clang/docs/index.rst` (updated) - Documentation index
4. `clang/docs/LanguageExtensions.rst` (updated) - Extensions list

### Test Files Added ✨ NEW

#### CodeGen Tests (9 files)
1. `clang/test/CodeGenCXX/customizable-functions-basic.cpp`
2. `clang/test/CodeGenCXX/customizable-functions-void.cpp`
3. `clang/test/CodeGenCXX/customizable-functions-constexpr.cpp`
4. `clang/test/CodeGenCXX/customizable-functions-noexcept.cpp`
5. `clang/test/CodeGenCXX/customizable-functions-template.cpp`
6. `clang/test/CodeGenCXX/customizable-functions-overload.cpp`
7. `clang/test/CodeGenCXX/customizable-functions-namespace.cpp`
8. `clang/test/CodeGenCXX/customizable-functions-trailing-return.cpp`
9. `clang/test/CodeGenCXX/customizable-functions-complex-args.cpp`

#### Semantic Tests (1 file)
10. `clang/test/SemaCXX/customizable-functions-errors.cpp`

#### LLVM Transform Tests ✨ NEW
11. `llvm/test/Transforms/CustomizableFunctions/basic-override.ll`
12. `llvm/test/Transforms/CustomizableFunctions/attributes.ll`
13. `llvm/test/Transforms/CustomizableFunctions/no-override.ll`
14. `llvm/test/Transforms/CustomizableFunctions/signature-mismatch.ll` (Phase 2b)
15. `llvm/test/Transforms/CustomizableFunctions/multiple-wrappers.ll` (Phase 2b)

#### Integration Tests ✨ NEW
16. `llvm/test/Other/customizable-functions-pipeline.ll` (Pipeline verification)
17. `clang/test/Driver/customizable-functions.cpp` (Driver flag verification)

#### Phase 2b Hardening Tests ✨ NEW
18. `clang/test/SemaCXX/customizable-functions-no-flag.cpp` (No-flag behavior)
19. `clang/test/Preprocessor/has_feature_customizable_functions.cpp` (Feature detection)
20. `clang/test/AST/customizable-functions-ast-dump.cpp` (AST tooling)

#### Phase 3 Sema Override Tests ✨ NEW
21. `clang/test/CodeGenCXX/customizable-functions-sema-override.cpp` (Sema-level override)

### Development Tools
- `custom-functions-dev.sh` - Build and test automation script

## Build Instructions

### Quick Start (Using Development Script)

```bash
# Configure for debug build
./custom-functions-dev.sh configure debug

# Build clang
./custom-functions-dev.sh build

# Run all tests
./custom-functions-dev.sh test all

# Run only CodeGen tests
./custom-functions-dev.sh test codegen

# Get help
./custom-functions-dev.sh help
```

### Manual Build

```bash
# Configure CMake
cmake -S llvm -B build-custom-functions -G Ninja \
  -DLLVM_ENABLE_PROJECTS=clang \
  -DLLVM_TARGETS_TO_BUILD=X86 \
  -DCMAKE_BUILD_TYPE=Debug \
  -DLLVM_ENABLE_ASSERTIONS=ON

# Build
cmake --build build-custom-functions -j$(nproc)

# Test with clang
./build-custom-functions/bin/clang++ -fcustomizable-functions \
  -std=c++20 -emit-llvm -S -o test.ll test.cpp

# Run lit tests
./build-custom-functions/bin/llvm-lit -v \
  clang/test/CodeGenCXX/customizable-functions-*.cpp \
  clang/test/SemaCXX/customizable-functions-errors.cpp
```

## Contact

For questions or issues related to this implementation, please refer to:
- GitHub Issues: https://github.com/gregvw/llvm-project/issues
- Clang Development List: cfe-dev@lists.llvm.org
