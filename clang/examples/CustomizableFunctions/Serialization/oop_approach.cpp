/**
 * Traditional OOP Serialization Approach
 *
 * This demonstrates the PROBLEMATIC inheritance-based approach to serialization.
 *
 * Problems demonstrated:
 * 1. INTRUSIVE - Must modify all types to inherit from Serializable
 * 2. TIGHT COUPLING - Types depend on serialization library
 * 3. LIMITED - Cannot serialize primitives, std types, or third-party types
 * 4. RUNTIME OVERHEAD - Virtual function calls
 * 5. OWNERSHIP ISSUES - Need virtual destructor, heap allocation
 */

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <sstream>

// ============================================================================
// SERIALIZATION LIBRARY
// ============================================================================

/**
 * Base class that all serializable types must inherit from.
 * PROBLEM: This is intrusive - requires modifying every type.
 */
class Serializable {
public:
    virtual ~Serializable() = default;  // PROBLEM: Need virtual destructor
    virtual std::string toJSON() const = 0;  // PROBLEM: Virtual function overhead
};

// ============================================================================
// USER-DEFINED TYPES (Must be modified to inherit from Serializable)
// ============================================================================

/**
 * PROBLEM: We must modify our Person class to inherit from Serializable.
 * This couples our domain model to the serialization library.
 */
class Person : public Serializable {
public:
    std::string name;
    int age;

    Person(std::string n, int a) : name(std::move(n)), age(a) {}

    // PROBLEM: Must implement toJSON for every class
    std::string toJSON() const override {
        return "{\"name\": \"" + name + "\", \"age\": " + std::to_string(age) + "}";
    }
};

/**
 * PROBLEM: Another class that must be modified to be serializable.
 */
class Point : public Serializable {
public:
    double x, y;

    Point(double x_, double y_) : x(x_), y(y_) {}

    std::string toJSON() const override {
        return "{\"x\": " + std::to_string(x) + ", \"y\": " + std::to_string(y) + "}";
    }
};

// ============================================================================
// THIRD-PARTY LIBRARY (We don't control this code)
// ============================================================================

namespace third_party {
    /**
     * PROBLEM: We cannot modify this class to inherit from Serializable!
     * This is from a third-party library we don't control.
     */
    class Product {
    public:
        std::string sku;
        double price;

        Product(std::string s, double p) : sku(std::move(s)), price(p) {}

        // Note: Does NOT inherit from Serializable
        // SOLUTION? We'd need to create a wrapper class:
        // class SerializableProduct : public Product, public Serializable { ... }
        // But this is clunky and doesn't work for types with no public constructor!
    };
}

// ============================================================================
// WORKAROUNDS FOR LIMITATIONS
// ============================================================================

/**
 * PROBLEM: std::string doesn't inherit from Serializable.
 * We must create wrapper functions for common types.
 */
namespace serialize_helpers {
    std::string serializeString(const std::string& s) {
        return "\"" + s + "\"";
    }

    std::string serializeInt(int value) {
        return std::to_string(value);
    }

    std::string serializeDouble(double value) {
        return std::to_string(value);
    }

    /**
     * PROBLEM: Can't easily serialize std::vector<T> generically.
     * Each type needs its own function or we need complex template machinery.
     */
    template<typename T>
    std::string serializeVector(const std::vector<T>& vec) {
        // PROBLEM: This doesn't work! T might not inherit from Serializable
        // We'd need to specialize for each type or use enable_if tricks
        std::string result = "[";
        for (size_t i = 0; i < vec.size(); ++i) {
            // Can't call vec[i].toJSON() - what if T is int?
            // Must handle each type separately!
            result += "???";  // What to put here?
            if (i < vec.size() - 1) result += ", ";
        }
        result += "]";
        return result;
    }
}

// ============================================================================
// GENERIC ALGORITHMS (Limited effectiveness)
// ============================================================================

/**
 * PROBLEM: This only works for types inheriting from Serializable.
 * Can't use with primitives, std types, or third-party types.
 */
template<typename Container>
std::string serializeContainer(const Container& items) {
    // PROBLEM: Requires heap allocation and virtual dispatch
    std::string result = "[";
    bool first = true;
    for (const auto& item : items) {
        if (!first) result += ", ";
        first = false;

        // PROBLEM: Must use dynamic_cast or assume Serializable interface
        // This won't compile for std::vector<int>, for example!
        // result += item.toJSON();  // Only works if item IS-A Serializable
    }
    result += "]";
    return result;
}

// ============================================================================
// USAGE DEMONSTRATION
// ============================================================================

int main() {
    std::cout << "=== OOP Approach to Serialization ===" << std::endl;
    std::cout << "Demonstrating the problems with inheritance-based design\n" << std::endl;

    // ✅ WORKS: Types that inherit from Serializable
    Person person("Alice", 30);
    std::cout << "Person: " << person.toJSON() << std::endl;

    Point point(10.5, 20.3);
    std::cout << "Point: " << point.toJSON() << std::endl;

    // ❌ PROBLEM 1: Can't serialize primitives
    int count = 42;
    // std::cout << count.toJSON();  // ERROR: int doesn't have toJSON()
    std::cout << "Int (workaround): " << serialize_helpers::serializeInt(count) << std::endl;

    // ❌ PROBLEM 2: Can't serialize std::string directly
    std::string message = "Hello";
    // std::cout << message.toJSON();  // ERROR: std::string doesn't inherit from Serializable
    std::cout << "String (workaround): " << serialize_helpers::serializeString(message) << std::endl;

    // ❌ PROBLEM 3: Can't serialize third-party types
    third_party::Product product("SKU-123", 99.99);
    // std::cout << product.toJSON();  // ERROR: Product doesn't inherit from Serializable
    std::cout << "Product: CANNOT SERIALIZE (third-party type)" << std::endl;

    // ❌ PROBLEM 4: Polymorphic containers require heap allocation and virtual dispatch
    std::vector<std::unique_ptr<Serializable>> mixedItems;
    mixedItems.push_back(std::make_unique<Person>("Bob", 25));
    mixedItems.push_back(std::make_unique<Point>(5.0, 10.0));

    std::cout << "\nPolymorphic container (with overhead): [" << std::endl;
    for (const auto& item : mixedItems) {
        std::cout << "  " << item->toJSON() << std::endl;  // Virtual dispatch overhead!
    }
    std::cout << "]" << std::endl;

    // ❌ PROBLEM 5: Can't serialize std::vector<int>
    std::vector<int> numbers = {1, 2, 3, 4, 5};
    // std::cout << serializeContainer(numbers);  // ERROR: int doesn't inherit from Serializable
    std::cout << "\nVector<int>: CANNOT USE GENERIC ALGORITHM" << std::endl;

    std::cout << "\n=== Summary of Problems ===" << std::endl;
    std::cout << "❌ Intrusive: Must modify all types to inherit from Serializable" << std::endl;
    std::cout << "❌ Tight coupling: Types depend on serialization library" << std::endl;
    std::cout << "❌ Limited: Can't serialize primitives, std types, third-party types" << std::endl;
    std::cout << "❌ Runtime overhead: Virtual function calls and v-table lookups" << std::endl;
    std::cout << "❌ Requires heap allocation for polymorphic collections" << std::endl;

    return 0;
}
