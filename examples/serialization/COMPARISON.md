# Detailed Comparison: OOP vs Custom Functions for Serialization

## The Challenge

Build a serialization system that can convert C++ objects to JSON, supporting:
- User-defined types (Person, Point)
- Standard library types (std::string, std::vector)
- Primitive types (int, double, bool)
- Third-party library types (types you don't control)

---

## Approach 1: Traditional OOP (Inheritance)

### Design

```cpp
class Serializable {
public:
    virtual ~Serializable() = default;
    virtual std::string toJSON() const = 0;
};

class Person : public Serializable {
    std::string name;
    int age;
public:
    std::string toJSON() const override {
        return "{\"name\": \"" + name + "\", \"age\": " + std::to_string(age) + "}";
    }
};
```

### Problems

#### 1. **INTRUSIVE** - Must modify all types

```cpp
// Before (clean data class)
struct Person {
    std::string name;
    int age;
};

// After (coupled to serialization library)
class Person : public Serializable {  // 😞 Must inherit!
    std::string name;
    int age;
public:
    std::string toJSON() const override { ... }  // 😞 Must implement!
};
```

**Impact**: Every type in your codebase must be modified. This violates the Open-Closed Principle and makes refactoring difficult.

#### 2. **TIGHT COUPLING** - Creates unnecessary dependencies

```cpp
#include "serialization_library.hpp"  // 😞 Every data class needs this!

class Person : public Serializable {  // 😞 Domain model depends on serialization
    // ...
};
```

**Impact**: Your domain model (Person, Point) becomes coupled to your serialization library. Can't use Person in code that doesn't include the serialization library.

#### 3. **LIMITED APPLICABILITY** - Can't work with many types

```cpp
// ❌ IMPOSSIBLE: int doesn't inherit from Serializable
int count = 42;
// count.toJSON();  // ERROR!

// ❌ IMPOSSIBLE: std::string doesn't inherit from Serializable
std::string message = "hello";
// message.toJSON();  // ERROR!

// ❌ IMPOSSIBLE: Third-party types can't be modified
third_party::Product product("SKU-123", 99.99);
// product.toJSON();  // ERROR!

// Workaround: Wrapper functions (clunky!)
std::string serializeInt(int value) { return std::to_string(value); }
std::string serializeString(const std::string& s) { return "\"" + s + "\""; }
// Need a wrapper for EVERY non-inheriting type! 😞
```

**Impact**: Core types (primitives, standard library, third-party) can't use your serialization system. Need manual wrapper functions for each type.

#### 4. **RUNTIME OVERHEAD** - Virtual function dispatch

```cpp
std::vector<std::unique_ptr<Serializable>> items;
items.push_back(std::make_unique<Person>("Alice", 30));
items.push_back(std::make_unique<Point>(10, 20));

for (const auto& item : items) {
    std::cout << item->toJSON();  // 😞 Virtual function call!
                                   // 😞 V-table lookup overhead!
                                   // 😞 Requires heap allocation!
}
```

**Impact**: Every serialization call requires:
- V-table lookup (runtime cost)
- Indirect function call (prevents inlining)
- Heap allocation for polymorphic storage
- Cache misses from pointer chasing

#### 5. **GENERIC ALGORITHMS DON'T WORK**

```cpp
template<typename Container>
std::string serializeContainer(const Container& items) {
    for (const auto& item : items) {
        result += item.toJSON();  // ❌ Only works if item IS-A Serializable
    }
}

std::vector<int> numbers = {1, 2, 3};
serializeContainer(numbers);  // ❌ ERROR! int doesn't have toJSON()
```

**Impact**: Can't write generic algorithms that work with all types. Must specialize for each type category.

### Summary of OOP Problems

| Problem | Impact |
|---------|--------|
| Intrusive | Must modify every type's source code |
| Tight coupling | Domain model depends on serialization library |
| Limited applicability | Can't serialize primitives, std types, third-party types |
| Runtime overhead | Virtual dispatch, v-table lookups, heap allocation |
| Poor composability | Generic algorithms don't work with all types |

---

## Approach 2: Custom Functions (Non-Intrusive)

### Design

```cpp
namespace serialize {
    template<typename T>
    custom std::string toJSON(const T& value) {
        // Default implementation or compile error
    }
}

// User provides overloads in their namespace (found via ADL)
struct Person {
    std::string name;
    int age;
    // No inheritance! No toJSON method!
};

std::string toJSON(const Person& p) {
    return "{\"name\": \"" + p.name + "\", \"age\": " + std::to_string(p.age) + "}";
}
```

### Advantages

#### 1. **NON-INTRUSIVE** - Zero modification to types

```cpp
// Your data class stays clean!
struct Person {
    std::string name;
    int age;
    // No inheritance!
    // No methods!
    // Just pure data!
};

// Serialization is provided externally
std::string toJSON(const Person& p) {
    // Implementation
}

// Usage
Person p("Alice", 30);
auto json = serialize::toJSON(p);  // ✅ Works via ADL!
```

**Impact**: Types remain pure data classes. No coupling, no modifications needed.

#### 2. **DECOUPLED** - Complete separation of concerns

```cpp
// person.hpp - NO dependency on serialization!
struct Person {
    std::string name;
    int age;
};

// person_json.cpp - Serialization is separate
#include "serialize.hpp"
#include "person.hpp"

std::string toJSON(const Person& p) {
    return serialize::json::object(...);
}
```

**Impact**: Domain model has zero knowledge of serialization. Can use Person in serialization-unaware code.

#### 3. **UNIVERSAL** - Works with ANY type

```cpp
// ✅ Primitives work seamlessly
int count = 42;
auto json = serialize::toJSON(count);  // "42"

// ✅ std types work seamlessly
std::string msg = "hello";
auto json = serialize::toJSON(msg);  // "\"hello\""

// ✅ Third-party types work non-intrusively
third_party::Product product("SKU", 99.99);
auto json = serialize::toJSON(product);  // Works!

// ✅ Nested structures work automatically
std::vector<int> nums = {1, 2, 3};
auto json = serialize::toJSON(nums);  // "[1, 2, 3]"

std::vector<Person> people = {...};
auto json = serialize::toJSON(people);  // Works recursively!
```

**Impact**: Single unified interface works with all types - no special cases or wrapper functions needed.

#### 4. **ZERO OVERHEAD** - Compile-time dispatch

```cpp
std::vector<Person> people = {
    Person("Alice", 30),
    Person("Bob", 25)
};

for (const auto& person : people) {
    std::cout << serialize::toJSON(person);  // ✅ Direct function call!
                                              // ✅ Can be inlined!
                                              // ✅ No v-table lookup!
                                              // ✅ No heap allocation needed!
}
```

**Generated assembly**: Direct function call, fully inlinable!

```asm
; Direct call - no indirection
call    _Z6toJSONRK6Person

; Compare to virtual dispatch:
; mov     rax, qword ptr [rdi]        ; Load v-table
; call    qword ptr [rax + 8]         ; Indirect call through v-table
```

**Impact**:
- Zero runtime overhead
- Compiler can inline calls
- Better cache locality
- No heap allocation required

#### 5. **GENERIC ALGORITHMS WORK** - True generic programming

```cpp
template<typename Container>
std::string serializeContainer(const Container& items) {
    std::vector<std::string> results;
    for (const auto& item : items) {
        results.push_back(serialize::toJSON(item));  // ✅ Works with ANY type!
    }
    return json::array(results);
}

// ✅ Works with primitives
std::vector<int> numbers = {1, 2, 3};
serializeContainer(numbers);

// ✅ Works with user types
std::vector<Person> people = {...};
serializeContainer(people);

// ✅ Works with third-party types
std::vector<third_party::Product> products = {...};
serializeContainer(products);
```

**Impact**: Write generic algorithms once, works with all types. True generic programming.

### Summary of Custom Functions Advantages

| Advantage | Benefit |
|-----------|---------|
| Non-intrusive | No modification to existing types |
| Decoupled | Complete separation of concerns |
| Universal | Works with primitives, std types, third-party types |
| Zero overhead | Direct calls, inlinable, no virtual dispatch |
| Composable | Generic algorithms work with all types |
| Natural syntax | Clean function calls |

---

## Side-by-Side Code Comparison

### Serializing a User Type

**OOP Approach (18 lines):**
```cpp
#include "serializable.hpp"  // Dependency!

class Person : public Serializable {  // Inheritance required
private:
    std::string name;
    int age;
public:
    Person(std::string n, int a) : name(n), age(a) {}

    virtual ~Person() = default;  // Need virtual destructor

    std::string toJSON() const override {  // Must implement
        return "{\"name\": \"" + name +
               "\", \"age\": " + std::to_string(age) + "}";
    }
};
```

**Custom Functions Approach (10 lines):**
```cpp
struct Person {  // Clean data class, no dependencies!
    std::string name;
    int age;
};

std::string toJSON(const Person& p) {  // External implementation
    return serialize::json::object(
        serialize::json::keyValue("name", serialize::toJSON(p.name)) + ", " +
        serialize::json::keyValue("age", serialize::toJSON(p.age))
    );
}
```

### Serializing a Vector of Ints

**OOP Approach:**
```cpp
// ❌ DOESN'T WORK - int doesn't inherit from Serializable
// Must write special-case code:

std::string serializeIntVector(const std::vector<int>& vec) {
    std::string result = "[";
    for (size_t i = 0; i < vec.size(); ++i) {
        result += std::to_string(vec[i]);  // Manual handling
        if (i < vec.size() - 1) result += ", ";
    }
    result += "]";
    return result;
}

std::vector<int> numbers = {1, 2, 3};
auto json = serializeIntVector(numbers);  // Special function needed
```

**Custom Functions Approach:**
```cpp
// ✅ JUST WORKS - generic implementation handles it

std::vector<int> numbers = {1, 2, 3};
auto json = serialize::toJSON(numbers);  // Generic function works!
// Output: "[1, 2, 3]"
```

### Serializing Third-Party Types

**OOP Approach:**
```cpp
// Third-party type (can't modify)
namespace third_party {
    class Product {
        std::string sku;
        double price;
    };
}

// ❌ PROBLEM: Can't make it inherit from Serializable
// Workaround: Create wrapper class
class SerializableProduct : public Product, public Serializable {
    // ❌ Doesn't work if Product has private/protected constructor
    // ❌ Doesn't work if Product is final
    // ❌ Creates object slicing issues
};
```

**Custom Functions Approach:**
```cpp
// Third-party type (can't modify)
namespace third_party {
    class Product {
        std::string sku;
        double price;
    };

    // ✅ SOLUTION: Just provide an overload
    std::string toJSON(const Product& p) {
        return serialize::json::object(
            serialize::json::keyValue("sku", serialize::toJSON(p.sku)) + ", " +
            serialize::json::keyValue("price", serialize::toJSON(p.price))
        );
    }
}

// Usage
third_party::Product p("SKU-123", 99.99);
auto json = serialize::toJSON(p);  // ✅ Works via ADL!
```

---

## Performance Comparison

### Virtual Dispatch Overhead (OOP)

```cpp
// OOP: Virtual function call
Serializable* obj = new Person("Alice", 30);
obj->toJSON();  // Virtual dispatch

// Generated assembly:
// mov     rax, qword ptr [rdi]        ; Load v-table pointer
// call    qword ptr [rax + 8]         ; Indirect call
```

**Cost**: ~5-10 cycles for v-table lookup + prevents inlining

### Direct Dispatch (Custom Functions)

```cpp
// Custom: Direct function call
Person p("Alice", 30);
serialize::toJSON(p);  // Direct call

// Generated assembly:
// call    _Z6toJSONRK6Person           ; Direct call (can be inlined)
```

**Cost**: 0 cycles (inlined by optimizer)

### Benchmark Results

| Operation | OOP (virtual) | Custom (direct) | Speedup |
|-----------|---------------|-----------------|---------|
| Serialize 1M ints | 45ms | 12ms | 3.75x |
| Serialize 100K structs | 230ms | 45ms | 5.1x |
| Generic algorithm | N/A (doesn't work) | 38ms | ∞ |

*Note: Benchmarks are illustrative. Actual performance depends on compiler, optimization level, and data.*

---

## Real-World Use Cases

### Use Case 1: Logging Framework

**OOP Problems:**
- Can't log primitives directly
- Can't log std::string without wrapper
- Must modify all classes to inherit from Loggable
- Runtime overhead on every log call

**Custom Functions Benefits:**
```cpp
namespace logging {
    template<typename T>
    custom std::string format(const T& value);
}

// Works with EVERYTHING:
log(42);                      // Primitives
log("Hello");                 // Literals
log(myVector);               // Containers
log(thirdPartyObject);       // Third-party types
log(myCustomStruct);         // Your types
```

### Use Case 2: Configuration System

**OOP Problems:**
- All config classes must inherit from Configurable
- Can't configure third-party types
- Can't have config for primitive types

**Custom Functions Benefits:**
```cpp
namespace config {
    template<typename T>
    custom void load(T& value, const Config& cfg);

    template<typename T>
    custom void save(const T& value, Config& cfg);
}

// Works non-intrusively with any type!
```

### Use Case 3: Network Serialization

**OOP Problems:**
- Network library forces inheritance on all types
- Can't send primitives, std types
- Tight coupling between domain and network layers

**Custom Functions Benefits:**
```cpp
namespace network {
    template<typename T>
    custom void serialize(const T& obj, Buffer& buf);

    template<typename T>
    custom void deserialize(T& obj, const Buffer& buf);
}

// Clean separation: domain model knows nothing about networking
```

---

## Conclusion

### OOP Approach (Problematic)

✅ **When to use:**
- Never for cross-cutting concerns like serialization
- Only when true IS-A relationships exist
- When runtime polymorphism is genuinely needed

❌ **Problems:**
- Intrusive (modifies all types)
- Tight coupling (creates dependencies)
- Limited (can't work with many types)
- Runtime overhead (virtual dispatch)
- Poor composability (generic algorithms don't work)

### Custom Functions Approach (Recommended)

✅ **When to use:**
- Serialization (JSON, XML, binary, etc.)
- Logging and formatting
- Configuration systems
- Network protocols
- Any cross-cutting concern
- When working with types you don't control

✅ **Advantages:**
- Non-intrusive (zero modification)
- Decoupled (no dependencies)
- Universal (works with all types)
- Zero overhead (direct dispatch)
- Composable (generic algorithms work)
- Natural (clean syntax)

### The Verdict

For serialization and similar cross-cutting concerns, **custom functions are clearly superior**. They solve all the problems inherent in the OOP approach while providing better performance and cleaner code.

The `custom` keyword makes this pattern accessible without requiring complex library infrastructure (like tag_invoke), bringing the power of non-intrusive customization points directly into the language.
