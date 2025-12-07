# Building with CMake

This directory supports two build methods:

## Method 1: Quick Test (Bash Script)

For a quick proof-of-concept without CMake:

```bash
./run_simple_test.sh
```

This uses pre-built clang and compiles on-the-fly with g++.

## Method 2: CMake Build (Recommended)

For a proper production build with real TInCuP library:

### Prerequisites

1. **Modified LLVM/Clang** built with customizable functions support:
   ```bash
   cd ../..
   ./custom-functions-dev.sh build
   ```

2. **CMake 3.20+** and a standard C++20 compiler (for TInCuP version)

### Build Steps

```bash
# Create build directory
mkdir build
cd build

# Configure with CMake
# Use the modified clang we just built
CC=../../build-custom-functions/bin/clang \
CXX=../../build-custom-functions/bin/clang++ \
cmake ..

# Build both test executables
cmake --build .

# Run the equivalence test
ctest --verbose
```

### Expected Output

```
Test project /home/greg/Projects/llvm-project/examples/custom_serialization/build
    Start 1: RunCustomVersion
1/3 Test #1: RunCustomVersion .................   Passed    0.00 sec
    Start 2: RunTInCuPVersion
2/3 Test #2: RunTInCuPVersion .................   Passed    0.00 sec
    Start 3: CustomTInCuPEquivalence
3/3 Test #3: CustomTInCuPEquivalence ..........   Passed    0.00 sec

100% tests passed, 0 tests failed out of 3
```

### CMake Options

#### TInCuP Source

**Local Development (Default):**
```bash
cmake -DUSE_LOCAL_TINCUP=ON ..
```
Uses TInCuP from `../../../TInCuP` (adjacent to llvm-project).

**Fetch from GitHub:**
```bash
cmake -DUSE_LOCAL_TINCUP=OFF ..
```
Downloads TInCuP from https://github.com/sandialabs/TInCuP.git

### Targets

- `test_custom_serialization` - Compiled with `-fcustomizable-functions`
- `test_tincup_serialization` - Compiled with real TInCuP library

### CTest Integration

The CMake build creates three tests:

1. **RunCustomVersion** - Executes custom keyword version
2. **RunTInCuPVersion** - Executes TInCuP CPO version
3. **CustomTInCuPEquivalence** - Compares outputs byte-for-byte

Run all tests:
```bash
ctest
```

Run specific test:
```bash
ctest -R Equivalence --verbose
```

### Installation

Install test binaries (optional):
```bash
cmake --build . --target install
```

Binaries will be installed to `${CMAKE_INSTALL_PREFIX}/bin/examples/`.

## Troubleshooting

### "Local TInCuP not found"

Ensure TInCuP is cloned adjacent to llvm-project:
```bash
cd /home/greg/Projects
git clone https://github.com/sandialabs/TInCuP.git
```

Or use `-DUSE_LOCAL_TINCUP=OFF` to fetch automatically.

### "undefined reference to tag_invoke"

Ensure you're using the real TInCuP library:
```bash
# Check that tincup target exists
cmake --build . --target help | grep tincup
```

### Tests fail with different outputs

This indicates a bug in the implementation. Check:
1. Both executables compile without errors
2. Run each manually to see individual outputs
3. Compare outputs character-by-character

## Integration with LLVM Build

To integrate with the main LLVM CMakeLists.txt, add to `llvm-project/examples/CMakeLists.txt`:

```cmake
add_subdirectory(custom_serialization)
```

Then build with:
```bash
cd llvm-project/build-custom-functions
cmake --build . --target test_custom_serialization test_tincup_serialization
```
