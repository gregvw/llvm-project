# Custom Keyword in Requires Clauses and Concepts

## Executive Summary

This document outlines the design and implementation plan for enabling the `custom` function specifier keyword to be used within C++ concepts and requires clauses. This will allow developers to create concepts that specifically require member functions to have the `custom` specifier.

## Background

The `custom` keyword is intended to be implemented as a function specifier (similar to `virtual`, `constexpr`, or `override`) that marks functions with special semantics. For the constraint system to be useful, we need to support two key capabilities:

1. **Basic compatibility**: Functions marked `custom` should work in standard concept definitions without any special handling
2. **Explicit checking**: Ability to define concepts that specifically require a function to have the `custom` specifier

### Example Use Cases

```cpp
// Basic case: custom functions should work in regular concepts
struct Foo {
  custom void serialize();
};

template<typename T>
concept Serializable = requires(T t) {
  { t.serialize() };  // Should work regardless of custom specifier
};

// Advanced case: concept that requires the custom specifier
template<typename T>
concept HasCustomSerialize = requires(T t) {
  { t.serialize() } custom;  // Only satisfied if serialize has custom specifier
};
```

## Current State

Based on exploration of the LLVM/Clang codebase, the `custom` keyword does not currently exist. The following components need to be implemented:

1. **Token definition** - The `custom` keyword token
2. **Function specifier storage** - AST representation in `FunctionDecl`
3. **Parser support** - Recognition and parsing of `custom` in function declarations
4. **Semantic analysis** - Validation of `custom` usage
5. **Requires clause integration** - Mechanism to check for `custom` in constraints

## Problem Analysis

### Problem 1: Basic Compatibility

**Issue**: Functions marked with `custom` must be usable in standard requires expressions without special syntax.

**Why it's important**: Developers shouldn't need to know or care whether a function is marked `custom` when writing general-purpose concepts.

**Current status**: Will work automatically once `custom` is implemented as a function specifier. Requires expressions evaluate whether function calls are well-formed, and most specifiers don't affect this evaluation.

**Solution required**: ✅ No special work needed - inherent to requires expression design.

### Problem 2: Explicit Custom Checking

**Issue**: Need a mechanism to create concepts that specifically require functions to have the `custom` specifier.

**Why it's important**: Some use cases may need to distinguish between custom and non-custom functions at compile time.

**Current status**: No mechanism exists. Must be implemented.

**Solution required**: ❌ Requires new implementation - this is the core problem.

## Approach Analysis

### Approach 1: Compound Requirement Extension (RECOMMENDED)

Model after the existing `noexcept` checking in compound requirements.

**Syntax:**
```cpp
template<typename T>
concept HasCustomSerialize = requires(T t) {
  { t.serialize() } custom;  // Check for custom specifier
};

// Can combine with other requirements
concept ComplexConcept = requires(T t) {
  { t.process() } noexcept custom -> std::same_as<int>;
};
```

**Advantages:**
- ✅ Natural syntax that mirrors existing `noexcept` pattern
- ✅ Integrates seamlessly with compound requirements
- ✅ Follows established precedent in Clang
- ✅ Can be combined with other requirement properties
- ✅ Straightforward implementation path
- ✅ Clear error messages ("requirement not satisfied: function must be custom")

**Disadvantages:**
- ⚠️ Adds keyword to requires expression grammar
- ⚠️ Moderate implementation complexity (4-5 files to modify)

**Implementation complexity**: Medium (3-5 days of work)

**Files to modify:**
1. `clang/lib/Parse/ParseExprCXX.cpp` - Parse `custom` in compound requirements
2. `clang/include/clang/AST/ExprConcepts.h` - Add `CustomLoc` and `SS_CustomNotMet` status
3. `clang/include/clang/Sema/Sema.h` - Update semantic action signatures
4. `clang/lib/Sema/SemaExprCXX.cpp` - Implement checking logic
5. `clang/lib/Sema/SemaConcept.cpp` - Add diagnostics
6. `clang/include/clang/Basic/DiagnosticSemaKinds.td` - Add diagnostic messages

### Approach 2: Type Trait Intrinsic

Add a builtin type trait like `__is_custom_function`.

**Syntax:**
```cpp
template<typename T>
concept HasCustomSerialize = requires(T t) {
  requires __is_custom_function(decltype(&T::serialize));
};
```

**Advantages:**
- ✅ Doesn't modify requires expression grammar
- ✅ Follows standard type trait pattern
- ✅ Could be useful in other contexts (static_assert, if constexpr, etc.)

**Disadvantages:**
- ❌ Verbose and unintuitive syntax
- ❌ Requires pointer-to-member types (awkward for overloaded functions)
- ❌ Poor error messages (just "constraint not satisfied")
- ❌ Limited to function type checking, not call expression checking
- ❌ Doesn't check the actual called function (only the type)

**Implementation complexity**: Low-Medium (2-3 days of work)

**Files to modify:**
1. `clang/include/clang/Basic/TokenKinds.def` - Add type trait token
2. `clang/lib/Sema/SemaTypeTraits.cpp` - Implement evaluation
3. `clang/include/clang/Basic/DiagnosticSemaKinds.td` - Add diagnostic messages

### Approach 3: Requires Expression Extension

Add a new requirement type specifically for checking function properties.

**Syntax:**
```cpp
template<typename T>
concept HasCustomSerialize = requires(T t) {
  custom t.serialize();  // New requirement type
};
```

**Advantages:**
- ✅ Clean, minimal syntax
- ✅ Extensible to other function properties

**Disadvantages:**
- ❌ Major departure from existing requires expression grammar
- ❌ High implementation complexity
- ❌ Would require new AST node type
- ❌ Less composable than compound requirements

**Implementation complexity**: High (1-2 weeks of work)

## Recommendation: Approach 1

**I recommend implementing Approach 1 (Compound Requirement Extension)** for the following reasons:

### Primary Rationale

1. **Precedent**: The `noexcept` checking in compound requirements provides a clear template to follow. The implementation pattern is well-established and proven.

2. **Consistency**: Keeps all requirement properties (return type, noexcept, custom) in one unified syntax.

3. **Composability**: Allows combining multiple requirements naturally:
   ```cpp
   { t.serialize() } noexcept custom -> std::convertible_to<std::string>
   ```

4. **Error Quality**: Can provide specific, actionable error messages:
   ```
   note: requirement not satisfied: expression must call a custom function
   ```

5. **Implementation Path**: Clear implementation path following existing patterns in the codebase.

### Secondary Benefits

- **Maintainability**: Future developers will easily understand the implementation by comparing to `noexcept` handling
- **Testability**: Can leverage existing test infrastructure for compound requirements
- **Documentation**: Easy to document as an extension of existing compound requirement syntax

### Trade-offs Accepted

- Minor grammar extension (acceptable given strong precedent)
- Moderate implementation complexity (mitigated by clear examples to follow)

## Implementation Plan

### Phase 1: Foundation (Prerequisites)

Before implementing requires clause support, the basic `custom` keyword must be implemented:

**1.1 Token Definition**
- File: `clang/include/clang/Basic/TokenKinds.def`
- Add: `CXX20_KEYWORD(custom, 0)`

**1.2 AST Storage**
- File: `clang/include/clang/AST/Decl.h`
- Add bit field to `FunctionDeclBits`: `unsigned HasCustomSpecifier : 1;`
- Add accessors: `bool hasCustomSpecifier()` and `void setCustomSpecifier(bool)`

**1.3 Parser Implementation**
- File: `clang/lib/Parse/ParseDecl.cpp` or `clang/lib/Parse/ParseDeclCXX.cpp`
- Implement parsing of `custom` in function declarators
- Handle `custom` in both free functions and member functions

**1.4 Semantic Analysis**
- File: `clang/lib/Sema/SemaDecl.cpp` or `clang/lib/Sema/SemaDeclCXX.cpp`
- Implement validation rules for `custom` specifier
- Add semantic checks (e.g., valid contexts for custom)

**1.5 Basic Testing**
- File: `clang/test/Parser/cxx-custom-specifier.cpp`
- File: `clang/test/SemaCXX/custom-specifier.cpp`
- Test parsing and basic semantic validation

**Estimated effort**: 2-3 days

### Phase 2: Requires Clause Integration

**2.1 Parse Custom in Compound Requirements**
- File: `clang/lib/Parse/ParseExprCXX.cpp`
- Location: Around line 3223 (after noexcept parsing)
- Implementation:
  ```cpp
  SourceLocation CustomLoc;
  if (Tok.is(tok::kw_custom)) {
    CustomLoc = ConsumeToken();
  }
  ```
- Update call to `ActOnCompoundRequirement` to pass `CustomLoc`

**2.2 Extend ExprRequirement AST**
- File: `clang/include/clang/AST/ExprConcepts.h`
- Add to `SatisfactionStatus` enum: `SS_CustomNotMet`
- Add field: `SourceLocation CustomLoc;`
- Add accessors: `bool hasCustomRequirement()`, `SourceLocation getCustomLoc()`
- Update constructor to accept `CustomLoc` parameter

**2.3 Update Semantic Action Signatures**
- File: `clang/include/clang/Sema/Sema.h`
- Modify `ActOnCompoundRequirement` overloads to accept `SourceLocation CustomLoc`
- Two overloads need updating (around line 14800)

**2.4 Implement Checking Logic**
- File: `clang/lib/Sema/SemaExprCXX.cpp`
- Location: In `BuildExprRequirement` function (around line 7930)
- Implement `hasCustomSpecifier(Expr *E)` helper function:
  - Handle `CXXMemberCallExpr` (member function calls)
  - Handle `DeclRefExpr` (free function calls)
  - Handle `CallExpr` (general function calls)
  - Strip implicit casts and references appropriately
- Add check after noexcept validation:
  ```cpp
  if (CustomLoc.isValid() && !hasCustomSpecifier(E)) {
    Status = concepts::ExprRequirement::SS_CustomNotMet;
  }
  ```

**2.5 Diagnostic Implementation**
- File: `clang/include/clang/Basic/DiagnosticSemaKinds.td`
- Add: `def note_expr_requirement_custom_not_met : Note<...>`
- File: `clang/lib/Sema/SemaConcept.cpp`
- Location: Around line 1748 (in requirement diagnostic handling)
- Add case for `SS_CustomNotMet` with appropriate diagnostic

**2.6 AST Serialization/Deserialization**
- File: `clang/lib/Serialization/ASTWriterStmt.cpp`
- File: `clang/lib/Serialization/ASTReaderStmt.cpp`
- Update serialization for `ExprRequirement` to include `CustomLoc`

**Estimated effort**: 3-4 days

### Phase 3: Testing and Documentation

**3.1 Parser Tests**
- File: `clang/test/Parser/cxx-concepts-custom.cpp`
- Test valid syntax: `{ expr } custom`
- Test combined syntax: `{ expr } noexcept custom`
- Test invalid syntax and error recovery

**3.2 Semantic Tests**
- File: `clang/test/SemaCXX/concepts-custom-requires.cpp`
- Test satisfied requirements (function has custom)
- Test unsatisfied requirements (function lacks custom)
- Test with overload resolution
- Test with template instantiation
- Test diagnostic messages

**3.3 Integration Tests**
- File: `clang/test/SemaCXX/concepts-custom-integration.cpp`
- Test complex concepts combining multiple requirements
- Test concept subsumption with custom requirements
- Test SFINAE behavior

**3.4 AST Tests**
- File: `clang/test/AST/ast-dump-concepts-custom.cpp`
- Verify AST structure is correct
- Test AST serialization/deserialization

**3.5 Documentation**
- Update Clang language extensions documentation
- Add examples to concepts documentation
- Update release notes

**Estimated effort**: 2-3 days

### Phase 4: Optional Enhancements

**4.1 Code Completion**
- Add code completion support for `custom` in compound requirements

**4.2 Fix-It Hints**
- Provide fix-it hints when `custom` requirement is not met

**4.3 Enhanced Diagnostics**
- Show why a particular function doesn't satisfy the custom requirement
- Include source location of function definition

**Estimated effort**: 1-2 days

## Total Implementation Estimate

- **Phase 1 (Foundation)**: 2-3 days
- **Phase 2 (Requires Clause Integration)**: 3-4 days
- **Phase 3 (Testing and Documentation)**: 2-3 days
- **Phase 4 (Optional Enhancements)**: 1-2 days

**Total**: 8-12 days of focused development work

## Risk Analysis

### Low Risk
- ✅ Following established patterns (noexcept checking)
- ✅ Clear implementation path
- ✅ No ABI changes
- ✅ Additive feature (doesn't break existing code)

### Medium Risk
- ⚠️ Need to handle overloaded functions correctly
- ⚠️ Need to handle template instantiation correctly
- ⚠️ Need to handle dependent expressions correctly

### Mitigation Strategies
- Follow the exact pattern used for `noexcept` checking
- Comprehensive test coverage including edge cases
- Code review focusing on template and overload handling

## Alternative Considered: Hybrid Approach

If Approach 1 proves too complex or has unforeseen issues, we could implement both Approach 1 and Approach 2:
- Approach 1 for the primary use case (better syntax, better errors)
- Approach 2 as a fallback for edge cases or metaprogramming needs

This would provide maximum flexibility at the cost of maintaining two mechanisms.

## Conclusion

Implementing custom keyword support in requires clauses via Approach 1 (Compound Requirement Extension) is the recommended path forward. It provides:

1. **Natural syntax** that fits well with existing C++ concepts
2. **Clear implementation path** following established patterns
3. **Quality diagnostics** for users
4. **Reasonable implementation cost** (8-12 days)

The implementation should proceed in phases, starting with the foundational custom keyword implementation, then adding requires clause integration, and finally comprehensive testing and documentation.

## Questions for Discussion

1. Should `custom` be allowed to combine with `noexcept` in compound requirements?
   - Recommendation: **Yes** - no reason to restrict this

2. Should the syntax be `{ expr } custom` or `{ expr } is custom` or `{ expr } requires custom`?
   - Recommendation: **`{ expr } custom`** - matches noexcept pattern

3. Should we also implement the type trait (Approach 2) for completeness?
   - Recommendation: **Not initially** - add only if users request it

4. What should happen with overloaded functions?
   - Recommendation: Check the **actually called** function after overload resolution

5. Should templates be instantiated to check the custom specifier?
   - Recommendation: **Yes** - consistent with other constraint checking

## References

- Clang requires expression implementation: `clang/lib/Parse/ParseExprCXX.cpp:3134`
- Noexcept checking precedent: `clang/lib/Sema/SemaExprCXX.cpp:7928`
- ExprRequirement AST: `clang/include/clang/AST/ExprConcepts.h:282`
- Compound requirement parsing: `clang/lib/Parse/ParseExprCXX.cpp:3192`
