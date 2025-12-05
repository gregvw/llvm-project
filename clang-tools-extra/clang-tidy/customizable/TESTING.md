# Testing the Customizable Functions Clang-Tidy Module

This guide explains how to test the customizable functions clang-tidy checks.

## Prerequisites

1. Build configuration must include `clang-tools-extra`:
   ```bash
   cmake -DLLVM_ENABLE_PROJECTS="clang;clang-tools-extra" path/to/llvm
   ```

2. Build clang-tidy and test tools:
   ```bash
   ninja clang-tidy FileCheck llvm-lit
   ```

## Running Tests

### 1. Run All Clang-Tidy Tests (Quick Check)

```bash
ninja check-clang-tools
```

### 2. Run Only Customizable Module Tests

```bash
bin/llvm-lit -v ../clang-tools-extra/test/clang-tidy/checkers/customizable/
```

Expected output:
```
PASS: Clang Tools :: clang-tidy/checkers/customizable/suggest-custom.cpp
Testing Time: X.XXs
  Passed: 1
```

### 3. Manual Testing with Real Files

Create a test file (`test.cpp`):
```cpp
void allocate(int size) {  // Should warn
}

void process_hook() {  // Should warn
}

custom void already_marked() {  // Should NOT warn
}

void regular_function() {  // Should NOT warn
}
```

Run clang-tidy:
```bash
bin/clang-tidy -checks='customizable-suggest-custom' \
               --extra-arg=-fcustomizable-functions \
               --extra-arg=-std=c++20 \
               test.cpp
```

Expected output:
```
test.cpp:1:6: warning: function 'allocate' appears to be a customization point;
              consider marking it 'custom' [customizable-suggest-custom]
void allocate(int size) {
     ^
     custom
test.cpp:1:6: note: customizable functions enable link-time override via LTO

test.cpp:5:6: warning: function 'process_hook' appears to be a customization point;
              consider marking it 'custom' [customizable-suggest-custom]
```

### 4. Test with Automatic Fixes

```bash
bin/clang-tidy -checks='customizable-suggest-custom' \
               --extra-arg=-fcustomizable-functions \
               --extra-arg=-std=c++20 \
               --fix \
               test.cpp
```

This will automatically add `custom` keyword to matching functions.

### 5. List Available Checks

```bash
bin/clang-tidy -list-checks | grep customizable
```

Should output:
```
customizable-suggest-custom
```

### 6. Get Check Documentation

```bash
bin/clang-tidy -checks='customizable-suggest-custom' \
               --explain-config
```

## Debugging Failed Tests

If tests fail, check:

1. **Compilation errors**: Ensure `-fcustomizable-functions` is enabled
2. **Wrong line numbers**: Update CHECK comments in test file
3. **Missing warnings**: Verify function matches naming patterns
4. **Extra warnings**: Check if pattern matching is too broad

View detailed test output:
```bash
bin/llvm-lit -v -a ../clang-tools-extra/test/clang-tidy/checkers/customizable/
```

## Test File Format

Test files use FileCheck directives:

```cpp
// RUN: %check_clang_tidy -std=c++20 -fcustomizable-functions %s customizable-suggest-custom %t

void allocate(int size) {
  // CHECK-MESSAGES: :[[@LINE-1]]:6: warning: function 'allocate' appears to be
  // CHECK-FIXES: custom void allocate(int size) {
}
```

Key directives:
- `RUN:` - Command to execute
- `CHECK-MESSAGES:` - Expected warning message
- `CHECK-FIXES:` - Expected code after fix is applied
- `[[@LINE-N]]` - Refers to line N lines above

## Integration Testing

Test with a real project:

```bash
# Generate compile_commands.json
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ...

# Run clang-tidy on project
bin/run-clang-tidy -checks='customizable-suggest-custom' \
                    -extra-arg=-fcustomizable-functions \
                    -p /path/to/build
```

## Performance Testing

For large codebases:

```bash
time bin/clang-tidy -checks='customizable-suggest-custom' \
                     --extra-arg=-fcustomizable-functions \
                     --extra-arg=-std=c++20 \
                     large_file.cpp
```

## See Also

- `../test/clang-tidy/checkers/customizable/suggest-custom.cpp` - Test file
- `../docs/clang-tidy/checks/customizable/suggest-custom.rst` - Documentation
- Clang-tidy developer guide: https://clang.llvm.org/extra/clang-tidy/Contributing.html
