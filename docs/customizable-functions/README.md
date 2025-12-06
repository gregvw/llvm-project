# Customizable Functions - Development Documentation

This directory contains development documentation for the `custom` function specifier feature for C++.

## Main Documentation

- **[IMPLEMENTATION_SUMMARY.md](IMPLEMENTATION_SUMMARY.md)**: Comprehensive overview of the entire implementation, including architecture, design decisions, and current status.
- **[CUSTOM_KEYWORD_REQUIRES_DESIGN.md](CUSTOM_KEYWORD_REQUIRES_DESIGN.md)**: Design document for enabling `custom` keyword in requires clauses and concepts.

## Implementation Notes

- **[AMBIGUOUS_OVERRIDE_BUG.md](AMBIGUOUS_OVERRIDE_BUG.md)**: Notes on fixing ambiguous override issues.
- **[DRIVER_FLAG_FIX.md](DRIVER_FLAG_FIX.md)**: Details on fixing driver flag handling.
- **[IDENTIFIER_ONLY_FIX.md](IDENTIFIER_ONLY_FIX.md)**: Notes on handling `custom` as an identifier.
- **[MANGLED_NAME_FIX.md](MANGLED_NAME_FIX.md)**: Information about mangled name handling.
- **[DOXYGEN_INTEGRATION_SUMMARY.md](DOXYGEN_INTEGRATION_SUMMARY.md)**: Integration with Doxygen documentation.
- **[REORGANIZATION_SUMMARY.md](REORGANIZATION_SUMMARY.md)**: Summary of code reorganization efforts.

## Comparison and Planning

- **[CUSTOMIZABLE_FUNCTIONS_VS_TAG_INVOKE.md](CUSTOMIZABLE_FUNCTIONS_VS_TAG_INVOKE.md)**: Comparison with tag_invoke pattern.
- **[SIDE_BY_SIDE_COMPARISON.md](SIDE_BY_SIDE_COMPARISON.md)**: Side-by-side feature comparisons.
- **[PR_PREPARATION_GUIDE.md](PR_PREPARATION_GUIDE.md)**: Guide for preparing pull requests.

## Getting Started

For an overview of the customizable functions feature, start with [IMPLEMENTATION_SUMMARY.md](IMPLEMENTATION_SUMMARY.md).

For information on the requires clause support, see [CUSTOM_KEYWORD_REQUIRES_DESIGN.md](CUSTOM_KEYWORD_REQUIRES_DESIGN.md).

## Testing

Run the test suite with:
```bash
./custom-functions-dev.sh test all       # Run all tests
./custom-functions-dev.sh test custom    # Run custom requires clause tests only
```

See the project root's `custom-functions-dev.sh` for all available test commands.
