# Custom Functions Demo

This directory demonstrates replacing TInCUp's `tag_invoke` customization points with the new `custom` function attribute approach.

## Example: Simplified Vector Operations

We'll recreate a subset of the RealVectorFramework's vector operations:
- `inner_product()` - compute dot product of two vectors
- `axpy()` - compute y = a*x + y operation

## Files

- `old_approach.cpp` - Using TInCUp tag_invoke (conceptual)
- `new_approach.cpp` - Using custom functions with -fcustomizable-functions
- `test.cpp` - Test both approaches
- `build.sh` - Build script
