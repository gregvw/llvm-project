#!/bin/bash

# Build script for custom functions demo

CLANG="./build-custom-functions/bin/clang++"

echo "=== Building Custom Functions Demo ==="
echo

# Build old approach (doesn't need custom functions)
echo "1. Building old_approach (tag_invoke)..."
$CLANG -std=c++20 -O2 old_approach.cpp -o old_approach
if [ $? -eq 0 ]; then
    echo "   ✓ Built successfully"
else
    echo "   ✗ Build failed"
    exit 1
fi
echo

# Build new approach with default implementation
echo "2. Building new_approach (custom functions, default impl)..."
$CLANG -std=c++20 -O2 -fcustomizable-functions new_approach.cpp -o new_approach_default
if [ $? -eq 0 ]; then
    echo "   ✓ Built successfully"
else
    echo "   ✗ Build failed"
    exit 1
fi
echo

# Check for .custom directive in assembly
echo "3. Verifying .custom directives in assembly..."
$CLANG -std=c++20 -O2 -S -fcustomizable-functions new_approach.cpp -o new_approach.s
if grep -q "\.custom.*inner_product" new_approach.s && grep -q "\.custom.*axpy" new_approach.s; then
    echo "   ✓ Found .custom directives for inner_product and axpy"
else
    echo "   ✗ .custom directives not found"
fi
echo

# Build with optimized implementation (separate compilation + linking)
echo "4. Building new_approach with optimized implementation..."
echo "   a. Compiling main file..."
$CLANG -std=c++20 -O2 -fcustomizable-functions -c new_approach.cpp -o new_approach_main.o
echo "   b. Compiling optimized implementations..."
$CLANG -std=c++20 -O2 -fcustomizable-functions -c optimized_impl.cpp -o optimized_impl.o
echo "   c. Linking..."
$CLANG new_approach_main.o optimized_impl.o -o new_approach_optimized
if [ $? -eq 0 ]; then
    echo "   ✓ Built successfully"
else
    echo "   ✗ Build failed"
    exit 1
fi
echo

echo "=== Running Tests ==="
echo

echo "1. Old approach (tag_invoke):"
./old_approach
echo

echo "2. New approach (default implementation):"
./new_approach_default
echo

echo "3. New approach (optimized implementation):"
./new_approach_optimized
echo

echo "=== Comparison ==="
echo "Old approach: Complex template machinery, compile-time customization"
echo "New approach: Simple functions, link-time customization"
echo "              - Simpler code"
echo "              - Faster compilation"
echo "              - Same runtime performance"
echo "              - Can swap implementations at link time"
