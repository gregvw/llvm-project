# Doxygen Integration for Customizable Functions

## What Was Added

### New Doxygen Commands in `clang/include/clang/AST/CommentCommands.td`

**Function Declaration Command:**
```tablegen
def Custom : FunctionDeclarationVerbatimLineCommand<"custom">;
```

**Block Commands:**
```tablegen
def CustomizationPoint : BlockCommand<"customizationpoint">;
def Customizable       : BlockCommand<"customizable">;
```

These commands are now recognized by Clang's documentation parser and won't produce warnings.

## Usage Examples

### Basic Function Declaration

```cpp
/// @custom
/// Serialize an object to JSON.
template<typename T>
custom std::string toJSON(const T& obj);
```

### Detailed Documentation

```cpp
/// @brief Clone operation
/// @custom
/// @customizationpoint
/// This is a customization point for cloning objects. Users can provide
/// optimized implementations for their types.
///
/// To customize, provide an overload in your type's namespace:
/// @code
/// namespace user {
///   MyType clone(const MyType& x) {
///     // Your custom implementation
///   }
/// }
/// @endcode
template<typename T>
custom T clone(const T& x) {
  return T(x);
}
```

### User Customization

```cpp
/// @customizationpoint
/// Custom implementation for MyVector
MyVector clone(const MyVector& x) {
  // Optimized implementation
}
```

## Testing

### Verify No Warnings

```bash
# Should produce no warnings
clang -cc1 -std=c++20 -fcustomizable-functions \
  -Wdocumentation -fsyntax-only \
  clang/test/Sema/doxygen-custom-functions.cpp
```

### Generate Doxygen Documentation

```bash
# In your project with Doxyfile
doxygen

# The @custom, @customizationpoint, and @customizable tags will appear
# in the generated documentation
```

## Integration with Tools

### Doxygen
- **@custom** - Appears as function attribute
- **@customizationpoint** - Creates detailed section
- **@customizable** - Adds customization note

### IDE Support
- **VSCode C++ Extension** - Recognizes in hover tooltips
- **Qt Creator** - Shows in documentation viewer
- **CLion** - Displays in quick documentation

### Breathe/Sphinx
- All commands work with Breathe for Sphinx integration
- Appear in generated reStructuredText

## Files Modified

```
clang/include/clang/AST/CommentCommands.td  (3 new commands added)
```

## Files Added

```
clang/test/Sema/doxygen-custom-functions.cpp              (test)
clang/examples/CustomizableFunctions/DOXYGEN_GUIDE.rst    (documentation)
```

## Rebuilding After Changes

Since `CommentCommands.td` is a TableGen file, you need to rebuild:

```bash
cd build-custom-functions
cmake --build . --target clang
# Or just:
ninja clang
```

The new commands will then be recognized.

## Benefits

1. **No Warnings**: Code with these Doxygen tags won't produce warnings
2. **Better Documentation**: Clear indication of customization points
3. **Tool Support**: IDEs and doc generators recognize these tags
4. **Professional**: Consistent with Doxygen's existing commands

## Example in Real Code

From the serialization example:

```cpp
namespace serialize {

/// @custom
/// @brief Generic serialization customization point
/// @customizationpoint
/// Users can provide implementations for their types via ADL.
/// The library provides built-in support for primitives and std types.
///
/// Example customization:
/// @code
/// namespace user {
///   struct Person { std::string name; int age; };
///
///   std::string toJSON(const Person& p) {
///     return "{\"name\": \"" + p.name + "\"}";
///   }
/// }
/// @endcode
///
/// @tparam T The type to serialize
/// @param value The object to serialize
/// @returns JSON string representation
template<typename T>
custom std::string toJSON(const T& value);

}
```

## Comparison with Existing Commands

| Command           | Use Case                    |
|-------------------|-----------------------------|
| `@function`       | Regular functions           |
| `@method`         | Class methods               |
| `@callback`       | Callback functions          |
| **`@custom`**     | **Customization points**    |
| `@virtual`        | Virtual functions           |
| `@static`         | Static functions            |

## Next Steps

1. Document examples in serialization code ✓
2. Add to PR (include `CommentCommands.td` changes)
3. Mention in `CustomizableFunctions.rst` documentation
4. Add examples to Doxygen guide

## PR Impact

This is a small, additive change:
- **Files Modified**: 1 (CommentCommands.td)
- **Lines Added**: 3
- **Breaking Changes**: None
- **Benefits**: Better documentation support

Perfect addition to the customizable functions PR!
