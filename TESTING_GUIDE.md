# Quick Testing Guide for Customizable Functions

This guide shows how to quickly verify the implementation works correctly.

## Prerequisites

Ensure you have the custom functions branch built:
```bash
git checkout claude/implement-customizable-functions-017PytX2DYHa66AXVdLaeae5
cmake --build build-custom-functions --target clang llc FileCheck count not
```

---

## Test 1: Basic Functionality

Create a simple test file:

```bash
cat > /tmp/test_custom.cpp << 'EOF'
#include <iostream>

custom double square(double x) {
  return x * x;
}

double cube(double x) {
  return x * x * x;
}

int main() {
  std::cout << "square(5) = " << square(5.0) << std::endl;
  std::cout << "cube(5) = " << cube(5.0) << std::endl;
  return 0;
}
EOF
```

Compile and check assembly:
```bash
# Compile to assembly
./build-custom-functions/bin/clang++ -std=c++20 -fcustomizable-functions \
  -S /tmp/test_custom.cpp -o /tmp/test_custom.s

# Check for .custom directive (should find it for square, not for cube)
echo "=== Checking for .custom directives ==="
grep -n "\.custom" /tmp/test_custom.s
```

**Expected output**:
```
(some line number): .custom _Z6squared
```

Note: Only `square` should have `.custom`, not `cube`.

---

## Test 2: LLVM IR Generation

Check LLVM IR output:

```bash
# Generate LLVM IR
./build-custom-functions/bin/clang++ -std=c++20 -fcustomizable-functions \
  -S -emit-llvm /tmp/test_custom.cpp -o /tmp/test_custom.ll

# Check for custom attribute
echo "=== Checking LLVM IR for custom attribute ==="
grep -A 2 "define.*square" /tmp/test_custom.ll
```

**Expected output**:
```llvm
; Function Attrs: custom ...
define ... @_Z6squared(double ...) #0 {
  ...
}
```

The function should have `custom` in the attributes comment.

---

## Test 3: Without Flag

Verify that without `-fcustomizable-functions`, the keyword doesn't work:

```bash
# Try to compile without the flag (should error)
./build-custom-functions/bin/clang++ -std=c++20 \
  /tmp/test_custom.cpp -o /tmp/test_custom 2>&1 | head -5
```

**Expected output**:
```
error: unknown type name 'custom'
```

---

## Test 4: Run Automated Tests

### LLVM IR Backend Test

```bash
cd build-custom-functions

# Run the LLVM IR test
bin/llc -mtriple=x86_64 -o - ../llvm/test/CodeGen/X86/custom-function.ll | \
  bin/FileCheck ../llvm/test/CodeGen/X86/custom-function.ll

# Check exit status
echo "Test result: $?"
```

**Expected**: Exit status 0 (success), no output.

### Clang Frontend Test (with flag)

```bash
cd build-custom-functions

# Test with -fcustomizable-functions
bin/clang -cc1 -triple x86_64 -emit-llvm -fcustomizable-functions \
  -o - ../clang/test/CodeGen/x86_64-custom-function.c | \
  bin/FileCheck ../clang/test/CodeGen/x86_64-custom-function.c \
  --check-prefix=CUSTOM

echo "With flag test result: $?"
```

**Expected**: Exit status 0 (success), no output.

### Clang Frontend Test (without flag)

```bash
cd build-custom-functions

# Test without -fcustomizable-functions
bin/clang -cc1 -triple x86_64 -emit-llvm \
  -o - ../clang/test/CodeGen/x86_64-custom-function.c | \
  bin/FileCheck ../clang/test/CodeGen/x86_64-custom-function.c \
  --check-prefix=NO-CUSTOM

echo "Without flag test result: $?"
```

**Expected**: Exit status 0 (success), no output.

---

## Test 5: Real-World Example (Demo)

Build and run the demo comparing tag_invoke vs custom functions:

```bash
cd custom-functions-demo

# Build the demo
./build.sh
```

**Expected output**:
```
=== Building Custom Functions Demo ===

1. Building old_approach (tag_invoke)...
   ✓ Built successfully

2. Building new_approach (custom functions, default impl)...
   ✓ Built successfully

3. Verifying .custom directives in assembly...
   ✓ Found .custom directives for inner_product and axpy

4. Building new_approach with optimized implementation...
   a. Compiling main file...
   b. Compiling optimized implementations...
   c. Linking...
   ✓ Built successfully

=== Running Tests ===

1. Old approach (tag_invoke):
Inner product: 32
After AXPY: [6, 9, 12]

2. New approach (default implementation):
Inner product: 32
After AXPY: [6, 9, 12]

3. New approach (optimized implementation):
[Using OPTIMIZED inner_product]
Inner product: 32
[Using OPTIMIZED axpy]
After AXPY: [6, 9, 12]
```

Note: The optimized version should print the "[Using OPTIMIZED ...]" messages.

---

## Test 6: Verify Assembly Directive Format

Check that the directive format matches expectations:

```bash
cat > /tmp/directive_test.cpp << 'EOF'
custom void foo() {}
custom int bar(int x) { return x; }
void baz() {}
EOF

./build-custom-functions/bin/clang++ -std=c++20 -fcustomizable-functions \
  -S /tmp/directive_test.cpp -o /tmp/directive_test.s

# Show the directives in context
echo "=== Assembly output around .custom directives ==="
grep -B 2 -A 2 "\.custom" /tmp/directive_test.s
```

**Expected format**:
```asm
.globl  _Z3foov
.type   _Z3foov,@function
.custom _Z3foov
_Z3foov:
```

The `.custom` directive should appear:
1. After `.globl` and `.type`
2. Before the function label
3. For `foo` and `bar`, but NOT for `baz`

---

## Test 7: Error Cases

Test that invalid usage produces appropriate errors:

```bash
# Test 1: Can't use custom keyword without flag
cat > /tmp/error_test1.cpp << 'EOF'
custom void foo() {}
EOF

echo "=== Test 1: Using custom without -fcustomizable-functions ==="
./build-custom-functions/bin/clang++ -std=c++20 /tmp/error_test1.cpp 2>&1 | grep error
```

**Expected**: Error about unknown type name 'custom'

```bash
# Test 2: Duplicate custom specifier (should be caught by parser)
cat > /tmp/error_test2.cpp << 'EOF'
custom custom void foo() {}
EOF

echo "=== Test 2: Duplicate custom specifier ==="
./build-custom-functions/bin/clang++ -std=c++20 -fcustomizable-functions \
  /tmp/error_test2.cpp 2>&1 | grep error
```

**Expected**: Error about duplicate declaration specifier

---

## Summary Checklist

Run through this checklist to verify everything works:

- [ ] Basic compilation with `custom` keyword succeeds
- [ ] `.custom` directive appears in assembly output
- [ ] `custom` attribute appears in LLVM IR
- [ ] Without `-fcustomizable-functions` flag, `custom` keyword causes error
- [ ] LLVM IR backend test passes
- [ ] Clang frontend test with flag passes
- [ ] Clang frontend test without flag passes
- [ ] Demo builds and runs successfully
- [ ] Optimized implementation in demo is actually used
- [ ] `.custom` directive has correct format and placement
- [ ] Error cases produce appropriate diagnostics

If all items pass ✓, the implementation is working correctly!

---

## Troubleshooting

### "unknown type name 'custom'" error

**Problem**: Using `custom` keyword but getting error.

**Solution**: Add `-fcustomizable-functions` flag to enable the keyword.

### No .custom directive in assembly

**Problem**: Function has `custom` keyword but no `.custom` directive in assembly.

**Possible causes**:
1. Function was inlined (make it `__attribute__((noinline))`)
2. Clang wasn't built with the changes (rebuild clang)
3. Using wrong clang binary (check path)

### Tests fail with "FileCheck" not found

**Problem**: Running tests but FileCheck tool missing.

**Solution**:
```bash
cmake --build build-custom-functions --target FileCheck count not
```

### Demo build fails

**Problem**: `./build.sh` fails in custom-functions-demo.

**Check**:
1. Is clang built? `ls build-custom-functions/bin/clang++`
2. Is build.sh executable? `chmod +x custom-functions-demo/build.sh`
3. Are you in the llvm-project root directory?
