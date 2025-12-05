#!/bin/bash
# Cleanup script to remove top-level files before PR submission
# Run from llvm-project root directory

set -e

echo "=========================================="
echo "Customizable Functions: PR Cleanup"
echo "=========================================="
echo ""

# Check we're in the right directory
if [ ! -d "clang" ] || [ ! -d "llvm" ]; then
    echo "❌ Error: Run this script from llvm-project root directory"
    exit 1
fi

echo "This script will DELETE the following top-level files:"
echo "  - CUSTOMIZABLE_FUNCTIONS_VS_TAG_INVOKE.md"
echo "  - SIDE_BY_SIDE_COMPARISON.md"
echo "  - REORGANIZATION_SUMMARY.md"
echo "  - PR_PREPARATION_GUIDE.md"
echo "  - cleanup-for-pr.sh (this script)"
echo "  - examples/ (entire directory)"
echo ""

read -p "Continue? (y/N) " -n 1 -r
echo
if [[ ! $REPLY =~ ^[Yy]$ ]]; then
    echo "Cancelled."
    exit 0
fi

echo ""
echo "🗑️  Removing top-level documentation files..."
rm -f CUSTOMIZABLE_FUNCTIONS_VS_TAG_INVOKE.md
rm -f SIDE_BY_SIDE_COMPARISON.md
rm -f REORGANIZATION_SUMMARY.md
rm -f PR_PREPARATION_GUIDE.md

echo "🗑️  Removing top-level examples directory..."
rm -rf examples/

echo "🗑️  Removing this cleanup script..."
rm -f cleanup-for-pr.sh

echo ""
echo "✅ Cleanup complete!"
echo ""
echo "Files kept in proper locations:"
echo "  📚 Documentation: clang/docs/CustomizableFunctions.rst"
echo "  💡 Examples: clang/examples/CustomizableFunctions/"
echo "  🧪 Tests: clang/test/CodeGenCXX/customizable-functions-*.cpp"
echo ""
echo "Next steps:"
echo "  1. Review changes: git status"
echo "  2. Run tests: ninja check-clang"
echo "  3. Format code: git diff --name-only | grep -E '\.(cpp|h)$' | xargs clang-format -i"
echo "  4. Commit and create PR"
