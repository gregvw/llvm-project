#!/bin/bash
# Regression test: Verify custom and TInCuP versions produce identical results
# Simplified version without std library dependencies

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo "========================================="
echo "Custom vs TInCuP Equivalence Test"
echo "(Simplified - No std library)"
echo "========================================="
echo ""

# Paths
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/../../build-custom-functions"
CUSTOM_CLANG="${BUILD_DIR}/bin/clang++"
SYSTEM_CXX="g++"

# Output files
CUSTOM_BIN="${SCRIPT_DIR}/test_custom_simple"
TINCUP_BIN="${SCRIPT_DIR}/test_tincup_simple"
CUSTOM_OUTPUT="${SCRIPT_DIR}/output_custom_simple.txt"
TINCUP_OUTPUT="${SCRIPT_DIR}/output_tincup_simple.txt"

# Clean up previous builds
rm -f "${CUSTOM_BIN}" "${TINCUP_BIN}" "${CUSTOM_OUTPUT}" "${TINCUP_OUTPUT}"

echo "Step 1: Compile custom version with modified clang..."
if [ ! -f "${CUSTOM_CLANG}" ]; then
    echo -e "${RED}Error: Modified clang not found at ${CUSTOM_CLANG}${NC}"
    echo "Please build the project first: ./custom-functions-dev.sh build"
    exit 1
fi

"${CUSTOM_CLANG}" -std=c++20 -fcustomizable-functions -nostdlib++ \
    -o "${CUSTOM_BIN}" \
    "${SCRIPT_DIR}/test_custom_simple.cpp"

if [ $? -eq 0 ]; then
    echo -e "${GREEN}✓ Custom version compiled successfully${NC}"
else
    echo -e "${RED}✗ Custom version compilation failed${NC}"
    exit 1
fi

echo ""
echo "Step 2: Compile TInCuP version with standard C++20 (using ${SYSTEM_CXX})..."
"${SYSTEM_CXX}" -std=c++20 -nostdlib++ \
    -o "${TINCUP_BIN}" \
    "${SCRIPT_DIR}/test_tincup_simple.cpp"

if [ $? -eq 0 ]; then
    echo -e "${GREEN}✓ TInCuP version compiled successfully${NC}"
else
    echo -e "${RED}✗ TInCuP version compilation failed${NC}"
    exit 1
fi

echo ""
echo "Step 3: Run custom version..."
"${CUSTOM_BIN}" > "${CUSTOM_OUTPUT}"
echo -e "${GREEN}✓ Custom version executed${NC}"
cat "${CUSTOM_OUTPUT}"

echo ""
echo "Step 4: Run TInCuP version..."
"${TINCUP_BIN}" > "${TINCUP_OUTPUT}"
echo -e "${GREEN}✓ TInCuP version executed${NC}"
cat "${TINCUP_OUTPUT}"

echo ""
echo "Step 5: Compare outputs..."
if diff -q "${CUSTOM_OUTPUT}" "${TINCUP_OUTPUT}" > /dev/null; then
    echo -e "${GREEN}=========================================${NC}"
    echo -e "${GREEN}✓ SUCCESS: Outputs are identical!${NC}"
    echo -e "${GREEN}=========================================${NC}"
    echo ""
    echo "This proves that custom and TInCuP implementations"
    echo "produce semantically equivalent behavior."
    echo ""
    exit 0
else
    echo -e "${RED}=========================================${NC}"
    echo -e "${RED}✗ FAILURE: Outputs differ${NC}"
    echo -e "${RED}=========================================${NC}"
    echo ""
    echo "Diff:"
    diff "${CUSTOM_OUTPUT}" "${TINCUP_OUTPUT}"
    echo ""
    exit 1
fi
