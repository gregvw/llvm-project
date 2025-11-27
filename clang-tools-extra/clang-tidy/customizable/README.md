# Customizable Functions Clang-Tidy Module

This module provides clang-tidy checks for working with customizable functions,
a Clang language extension that enables link-time function override.

## Checks

- **suggest-custom**: Suggests adding the `custom` keyword to functions that
  appear to be customization points based on naming patterns.

## Requirements

- C++20 or later
- `-fcustomizable-functions` compiler flag enabled

## Usage

```bash
clang-tidy -checks='customizable-*' \
           --extra-arg=-fcustomizable-functions \
           --extra-arg=-std=c++20 \
           file.cpp
```

## See Also

- Clang Customizable Functions documentation
- WG21 papers: P1292R0, P2279R0, P2547R1
