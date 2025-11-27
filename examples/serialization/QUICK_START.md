# Quick Start: Serialization with Custom Functions

A practical guide to using `custom` functions for serialization.

## Step 1: Define Your Customization Point

```cpp
namespace serialize {
    // This is your customization point - a 'custom' function template
    template<typename T>
    custom std::string toJSON(const T& value) {
        // You can provide a default implementation, or static_assert
        static_assert(sizeof(T) == 0, "No serialization defined for this type");
        return "";
    }
}
```

## Step 2: Provide Built-in Type Overloads

```cpp
namespace serialize {
    // Overloads for common types
    inline std::string toJSON(int value) {
        return std::to_string(value);
    }

    inline std::string toJSON(double value) {
        return std::to_string(value);
    }

    inline std::string toJSON(bool value) {
        return value ? "true" : "false";
    }
}
```

## Step 3: Add std Library Type Support

```cpp
namespace std {
    // Add support for std::string
    inline std::string toJSON(const std::string& value) {
        return "\"" + value + "\"";
    }

    // Generic vector support - works with ANY element type!
    template<typename T>
    std::string toJSON(const std::vector<T>& vec) {
        using serialize::toJSON;  // Important for ADL!

        std::string result = "[";
        for (size_t i = 0; i < vec.size(); ++i) {
            result += toJSON(vec[i]);  // Recursive - finds right overload
            if (i < vec.size() - 1) result += ", ";
        }
        result += "]";
        return result;
    }
}
```

## Step 4: Make Your Types Serializable

```cpp
// Your domain type - stays clean, no inheritance!
struct Person {
    std::string name;
    int age;
};

// Provide serialization externally (non-intrusive)
inline std::string toJSON(const Person& p) {
    using serialize::toJSON;  // Import the namespace

    return "{\"name\": " + toJSON(p.name) +
           ", \"age\": " + toJSON(p.age) + "}";
}
```

## Step 5: Use It!

```cpp
int main() {
    using serialize::toJSON;

    // Primitives
    int x = 42;
    std::cout << toJSON(x);  // "42"

    // Strings
    std::string msg = "Hello";
    std::cout << toJSON(msg);  // "\"Hello\""

    // User types
    Person p{"Alice", 30};
    std::cout << toJSON(p);  // {"name": "Alice", "age": 30}

    // Vectors work automatically!
    std::vector<int> nums = {1, 2, 3};
    std::cout << toJSON(nums);  // [1, 2, 3]

    std::vector<Person> people = {{"Bob", 25}, {"Carol", 35}};
    std::cout << toJSON(people);  // [{"name": "Bob", ...}, ...]

    // Nested structures work!
    std::vector<std::vector<int>> matrix = {{1, 2}, {3, 4}};
    std::cout << toJSON(matrix);  // [[1, 2], [3, 4]]
}
```

## Step 6: Write Generic Algorithms

```cpp
template<typename Container>
std::string serializeAll(const Container& items) {
    using serialize::toJSON;

    std::string result = "[";
    bool first = true;
    for (const auto& item : items) {
        if (!first) result += ", ";
        first = false;
        result += toJSON(item);  // Works with ANY type!
    }
    result += "]";
    return result;
}

// Use with anything:
serializeAll(std::vector<int>{1, 2, 3});
serializeAll(std::vector<Person>{ ... });
serializeAll(std::vector<std::string>{ ... });
```

## Advanced: Third-Party Types

```cpp
// Third-party library (you don't control)
namespace third_party {
    class Product {
    public:
        std::string sku;
        double price;
    };

    // Just add an overload in their namespace!
    inline std::string toJSON(const Product& p) {
        using serialize::toJSON;
        return "{\"sku\": " + toJSON(p.sku) +
               ", \"price\": " + toJSON(p.price) + "}";
    }
}

// Now it works:
third_party::Product product{"ABC-123", 99.99};
std::cout << serialize::toJSON(product);  // ADL finds it!
```

## Key Points

### ✅ DO

1. **Use `using serialize::toJSON;`** in your overloads to enable ADL
2. **Provide overloads in the same namespace** as your types
3. **Keep types clean** - no inheritance, no member functions
4. **Write generic algorithms** - they work with all types

### ❌ DON'T

1. **Don't use qualified calls in generic code** - breaks ADL
   ```cpp
   // Bad:
   serialize::toJSON(item);  // Disables ADL!

   // Good:
   using serialize::toJSON;
   toJSON(item);  // Enables ADL!
   ```

2. **Don't modify types** - keep them as pure data classes

3. **Don't use inheritance** - that's the old OOP way

## Building

```bash
# With custom clang build
/path/to/custom-clang++ -std=c++20 -fcustomizable-functions \
  -fcustomizable-functions-sema your_code.cpp -o your_app
```

## Common Patterns

### JSON Object Builder

```cpp
namespace json {
    inline std::string object(const std::string& content) {
        return "{" + content + "}";
    }

    inline std::string keyValue(const std::string& key, const std::string& value) {
        return "\"" + key + "\": " + value;
    }
}

// Use in your overloads:
std::string toJSON(const Person& p) {
    using serialize::toJSON;
    return json::object(
        json::keyValue("name", toJSON(p.name)) + ", " +
        json::keyValue("age", toJSON(p.age))
    );
}
```

### Array Builder

```cpp
namespace json {
    inline std::string array(const std::vector<std::string>& items) {
        std::string result = "[";
        for (size_t i = 0; i < items.size(); ++i) {
            result += items[i];
            if (i < items.size() - 1) result += ", ";
        }
        result += "]";
        return result;
    }
}
```

### Generic Container Serialization

```cpp
template<typename Container>
std::string toJSONContainer(const Container& items) {
    using serialize::toJSON;

    std::vector<std::string> serialized;
    for (const auto& item : items) {
        serialized.push_back(toJSON(item));
    }
    return json::array(serialized);
}
```

## Troubleshooting

### Error: "No serialization defined for this type"

**Solution**: Provide a `toJSON` overload for that type in its namespace.

### Error: Ambiguous call to `toJSON`

**Solution**: Make sure you're using `using serialize::toJSON;` before the call.

### Generic algorithm doesn't compile

**Solution**: Don't use qualified calls (`serialize::toJSON`). Use unqualified calls with `using`.

```cpp
// Wrong:
for (const auto& item : items) {
    result += serialize::toJSON(item);  // Qualified - breaks ADL
}

// Right:
using serialize::toJSON;
for (const auto& item : items) {
    result += toJSON(item);  // Unqualified - enables ADL
}
```

## Next Steps

1. See `custom_approach.cpp` for a complete working example
2. See `COMPARISON.md` for detailed comparison with OOP
3. See `DEMO_OUTPUT.md` for actual runtime output

## Summary

**Custom functions make serialization:**
- ✅ Non-intrusive (no type modifications)
- ✅ Universal (works with all types)
- ✅ Zero-overhead (direct calls)
- ✅ Composable (generic algorithms work)
- ✅ Clean (simple syntax)

**That's it!** You now have a powerful, non-intrusive serialization system in less than 50 lines of code.
