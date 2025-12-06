# PR Preparation Guide: Customizable Functions

## Current State ✅

### Files in Correct Locations

**Tests** (✅ Already correct):
```
clang/test/CodeGenCXX/
├── customizable-functions-basic.cpp
├── customizable-functions-complex-args.cpp
├── customizable-functions-constexpr.cpp
├── customizable-functions-declaration-only.cpp
├── customizable-functions-errors.cpp (Sema test)
├── customizable-functions-extern-c.cpp
├── customizable-functions-namespace.cpp
├── customizable-functions-noexcept.cpp
├── customizable-functions-overload.cpp
├── customizable-functions-rvf-comparison.cpp
├── customizable-functions-sema-override.cpp
├── customizable-functions-template.cpp
├── customizable-functions-trailing-return.cpp
├── customizable-functions-unaffected.cpp
└── customizable-functions-void.cpp
```

**Documentation** (✅ Already exists):
```
clang/docs/CustomizableFunctions.rst
```

**Examples** (✅ Just created):
```
clang/examples/CustomizableFunctions/
├── CMakeLists.txt
└── Serialization/
    ├── CMakeLists.txt
    ├── README.rst
    ├── oop_approach.cpp
    └── custom_approach.cpp
```

### Files to DELETE Before PR 🗑️

**Top-level pollution** (delete these):
```bash
# Delete these files:
rm CUSTOMIZABLE_FUNCTIONS_VS_TAG_INVOKE.md
rm SIDE_BY_SIDE_COMPARISON.md
rm REORGANIZATION_SUMMARY.md  # (this file too, once done)
rm PR_PREPARATION_GUIDE.md    # (this file, once read)
rm -rf examples/               # entire directory
```

## Recommended Actions Before PR

### 1. Clean Up Top-Level Files

```bash
cd /home/greg/Projects/llvm-project

# Remove top-level documentation (content preserved in proper locations)
rm CUSTOMIZABLE_FUNCTIONS_VS_TAG_INVOKE.md
rm SIDE_BY_SIDE_COMPARISON.md

# Remove top-level examples (moved to clang/examples/)
rm -rf examples/

# Remove these temporary guide files
rm REORGANIZATION_SUMMARY.md
rm PR_PREPARATION_GUIDE.md
```

### 2. Enhance Documentation (Optional)

The existing `clang/docs/CustomizableFunctions.rst` could be enhanced with:

- Section on comparison with tag_invoke pattern
- Section on practical use cases (serialization example)
- Performance characteristics
- Best practices for ADL usage

**But this is optional** - the current documentation may be sufficient for the initial PR.

### 3. Verify Examples Build

```bash
cd build-custom-functions
cmake ..
cmake --build . --target oop-serialization-demo
cmake --build . --target custom-serialization-demo

# Test they work
./bin/oop-serialization-demo
./bin/custom-serialization-demo
```

### 4. Run All Tests

```bash
cd build-custom-functions
ninja check-clang
# Or just the custom functions tests:
bin/llvm-lit -v ../clang/test/CodeGenCXX/customizable-functions*.cpp
```

### 5. Format Code

```bash
# Format all modified files
git diff --name-only | grep -E '\.(cpp|h)$' | xargs clang-format -i
```

## PR Structure Recommendations

### Commit Organization

Consider splitting into logical commits:

1. **Core implementation** (if not already committed)
   - Parser changes
   - Sema changes
   - CodeGen changes

2. **Tests**
   - All test files in `clang/test/`

3. **Documentation**
   - `clang/docs/CustomizableFunctions.rst`

4. **Examples** (optional, could be separate PR)
   - `clang/examples/CustomizableFunctions/`

### Commit Message Template

```
[Clang] Add customizable functions language extension

This patch introduces the 'custom' keyword as an experimental language
extension for defining non-intrusive customization points in C++.

The feature addresses the same problems as the tag_invoke pattern
(WG21 P1895R0, P2279R0) but with built-in compiler support.

Key features:
- 'custom' keyword marks functions as customization points
- ADL-based override resolution with -fcustomizable-functions-sema
- Zero runtime overhead (direct function calls)
- Works with any type (primitives, std types, third-party)

Tests: clang/test/CodeGenCXX/customizable-functions-*.cpp
Docs: clang/docs/CustomizableFunctions.rst
Examples: clang/examples/CustomizableFunctions/

Differential Revision: https://reviews.llvm.org/DXXXXX
```

## File Organization Summary

### ✅ Keep These (Correct Locations)

```
clang/
├── docs/
│   └── CustomizableFunctions.rst               ✅ Keep
├── examples/
│   └── CustomizableFunctions/                  ✅ Keep
│       ├── CMakeLists.txt
│       └── Serialization/
│           ├── CMakeLists.txt
│           ├── README.rst
│           ├── oop_approach.cpp
│           └── custom_approach.cpp
└── test/
    └── CodeGenCXX/
        └── customizable-functions-*.cpp        ✅ Keep
```

### 🗑️ Delete These (Top-Level Pollution)

```
llvm-project/
├── CUSTOMIZABLE_FUNCTIONS_VS_TAG_INVOKE.md    🗑️ Delete
├── SIDE_BY_SIDE_COMPARISON.md                 🗑️ Delete
├── REORGANIZATION_SUMMARY.md                  🗑️ Delete
├── PR_PREPARATION_GUIDE.md                    🗑️ Delete
└── examples/                                   🗑️ Delete (entire dir)
```

## Quick Cleanup Script

```bash
#!/bin/bash
# Run from llvm-project root

# Remove top-level documentation
rm -f CUSTOMIZABLE_FUNCTIONS_VS_TAG_INVOKE.md
rm -f SIDE_BY_SIDE_COMPARISON.md
rm -f REORGANIZATION_SUMMARY.md
rm -f PR_PREPARATION_GUIDE.md

# Remove top-level examples (content moved to clang/examples/)
rm -rf examples/

echo "✅ Cleanup complete!"
echo ""
echo "Remaining customizable functions files:"
echo "  Documentation: clang/docs/CustomizableFunctions.rst"
echo "  Examples: clang/examples/CustomizableFunctions/"
echo "  Tests: clang/test/CodeGenCXX/customizable-functions-*.cpp"
```

## Final Checklist

Before submitting PR:

- [ ] Delete all top-level MD files
- [ ] Delete top-level examples/ directory
- [ ] Verify tests pass: `ninja check-clang`
- [ ] Verify examples build: `ninja oop-serialization-demo custom-serialization-demo`
- [ ] Run clang-format on modified files
- [ ] Update clang/docs/index.rst if needed (add CustomizableFunctions.rst to TOC)
- [ ] Write clear commit message
- [ ] Review LLVM coding standards
- [ ] Test on multiple platforms if possible

## LLVM Review Process

1. Create Phabricator account if needed
2. Use `arc diff` to upload patch
3. Add reviewers (look for Clang frontend experts)
4. Respond to review comments
5. Update patch as needed
6. Wait for approval

## Reference

- LLVM Contributing: https://llvm.org/docs/Contributing.html
- Clang Developer Docs: https://clang.llvm.org/docs/
- Code Review: https://llvm.org/docs/CodeReview.html
- Phabricator Guide: https://llvm.org/docs/Phabricator.html
