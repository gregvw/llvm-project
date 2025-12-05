/**
 * Custom Functions Serialization Approach
 *
 * This demonstrates the CLEAN approach using 'custom' functions.
 *
 * Advantages demonstrated:
 * 1. NON-INTRUSIVE - No modification to existing types
 * 2. DECOUPLED - Types have zero knowledge of serialization
 * 3. UNIVERSAL - Works with primitives, std types, and third-party types
 * 4. ZERO OVERHEAD - Compile-time dispatch, direct function calls
 * 5. NATURAL - Clean function call syntax
 *
 * Build with:
 *   clang++ -std=c++20 -fcustomizable-functions -fcustomizable-functions-sema \
 *           custom_approach.cpp -o custom_demo
 */

#include <iostream>
#include <string>
#include <vector>
#include <sstream>

// ============================================================================
// SERIALIZATION LIBRARY (using 'custom' functions)
// ============================================================================

namespace serialize {

/// @custom
/// @brief Generic JSON serialization customization point
/// @customizationpoint
/// This function provides a non-intrusive way to serialize any C++ type to JSON.
/// Users can customize serialization for their types by providing an overload
/// in their type's namespace, which will be found via ADL (Argument Dependent Lookup).
///
/// Example customization:
/// @code
/// namespace user {
///   struct Person { std::string name; int age; };
///
///   // This overload is found via ADL
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
custom std::string toJSON(const T& value) {
    // Default implementation - could provide a generic fallback
    // For this example, we require explicit overloads
    static_assert(sizeof(T) == 0, "No serialization defined for this type");
    return "";
}

// Helper functions for JSON formatting
namespace json {
    inline std::string object(const std::string& content) {
        return "{" + content + "}";
    }

    inline std::string keyValue(const std::string& key, const std::string& value) {
        return "\"" + key + "\": " + value;
    }

    inline std::string quote(const std::string& s) {
        return "\"" + s + "\"";
    }

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

// ============================================================================
// LIBRARY-PROVIDED OVERLOADS for common types
// ============================================================================

// ADVANTAGE: Can provide serialization for primitive types!
inline std::string toJSON(int value) {
    return std::to_string(value);
}

inline std::string toJSON(double value) {
    return std::to_string(value);
}

inline std::string toJSON(bool value) {
    return value ? "true" : "false";
}

} // namespace serialize

// ============================================================================
// STD NAMESPACE OVERLOADS (Non-intrusive customization)
// ============================================================================

namespace std {
    // ADVANTAGE: Can provide serialization for std::string without modifying it!
    inline std::string toJSON(const std::string& value) {
        return serialize::json::quote(value);
    }

    // ADVANTAGE: Generic vector serialization works with ANY element type!
    template<typename T>
    std::string toJSON(const std::vector<T>& vec) {
        using serialize::toJSON;
        std::vector<std::string> items;
        for (const auto& item : vec) {
            // Recursively serialize each item - ADL will find the right overload
            items.push_back(toJSON(item));
        }
        return serialize::json::array(items);
    }
}

// ============================================================================
// USER-DEFINED TYPES (No modification needed, no inheritance)
// ============================================================================

/**
 * ADVANTAGE: Person is a pure data class - no inheritance, no coupling!
 * It knows nothing about serialization.
 */
struct Person {
    std::string name;
    int age;

    Person(std::string n, int a) : name(std::move(n)), age(a) {}
    // No toJSON() method needed!
    // No inheritance from Serializable!
};

/**
 * ADVANTAGE: We provide serialization externally, non-intrusively.
 * This overload is found via ADL when serialize::toJSON is called.
 */
inline std::string toJSON(const Person& p) {
    using serialize::toJSON;
    return serialize::json::object(
        serialize::json::keyValue("name", toJSON(p.name)) + ", " +
        serialize::json::keyValue("age", toJSON(p.age))
    );
}

/**
 * ADVANTAGE: Another clean data class with external serialization.
 */
struct Point {
    double x, y;

    Point(double x_, double y_) : x(x_), y(y_) {}
    // No toJSON() method!
    // No inheritance!
};

inline std::string toJSON(const Point& p) {
    using serialize::toJSON;
    return serialize::json::object(
        serialize::json::keyValue("x", toJSON(p.x)) + ", " +
        serialize::json::keyValue("y", toJSON(p.y))
    );
}

// ============================================================================
// THIRD-PARTY LIBRARY (We don't control this code)
// ============================================================================

namespace third_party {
    /**
     * ADVANTAGE: This third-party class is completely unmodified.
     * No inheritance, no knowledge of our serialization system.
     */
    class Product {
    public:
        std::string sku;
        double price;

        Product(std::string s, double p) : sku(std::move(s)), price(p) {}
    };

    /**
     * ADVANTAGE: We can serialize third-party types non-intrusively!
     * Just provide an overload in the third-party namespace (or via ADL).
     */
    inline std::string toJSON(const Product& prod) {
        using serialize::toJSON;
        return serialize::json::object(
            serialize::json::keyValue("sku", toJSON(prod.sku)) + ", " +
            serialize::json::keyValue("price", toJSON(prod.price))
        );
    }
}

// ============================================================================
// GENERIC ALGORITHMS (Work with ANY type that has toJSON)
// ============================================================================

/**
 * ADVANTAGE: This generic algorithm works with ANY type!
 * - Primitives (int, double)
 * - std types (string, vector)
 * - User types (Person, Point)
 * - Third-party types (Product)
 */
template<typename Container>
std::string serializeContainer(const Container& items) {
    using serialize::toJSON;
    std::vector<std::string> serialized;
    for (const auto& item : items) {
        // ADVANTAGE: Direct function call, no virtual dispatch!
        // ADVANTAGE: Works with ANY type that has toJSON defined
        serialized.push_back(toJSON(item));
    }
    return serialize::json::array(serialized);
}

// ============================================================================
// USAGE DEMONSTRATION
// ============================================================================

int main() {
    using namespace serialize;

    std::cout << "=== Custom Functions Approach to Serialization ===" << std::endl;
    std::cout << "Demonstrating clean, non-intrusive design\n" << std::endl;

    // ✅ WORKS: User-defined types (no modification needed!)
    Person person("Alice", 30);
    std::cout << "Person: " << toJSON(person) << std::endl;

    Point point(10.5, 20.3);
    std::cout << "Point: " << toJSON(point) << std::endl;

    // ✅ WORKS: Primitives (seamlessly!)
    int count = 42;
    std::cout << "Int: " << toJSON(count) << std::endl;

    double pi = 3.14159;
    std::cout << "Double: " << toJSON(pi) << std::endl;

    // ✅ WORKS: std::string (no wrapper needed!)
    std::string message = "Hello, World!";
    std::cout << "String: " << toJSON(message) << std::endl;

    // ✅ WORKS: Third-party types (non-intrusive!)
    third_party::Product product("SKU-123", 99.99);
    std::cout << "Product: " << toJSON(product) << std::endl;

    // ✅ WORKS: Nested structures automatically!
    std::vector<int> numbers = {1, 2, 3, 4, 5};
    std::cout << "Vector<int>: " << toJSON(numbers) << std::endl;

    std::vector<std::string> words = {"hello", "world", "from", "custom"};
    std::cout << "Vector<string>: " << toJSON(words) << std::endl;

    std::vector<Person> people = {
        Person("Bob", 25),
        Person("Carol", 35),
        Person("Dave", 40)
    };
    std::cout << "Vector<Person>: " << toJSON(people) << std::endl;

    // ✅ WORKS: Generic algorithm works with ANY type!
    std::vector<Point> points = {Point(0, 0), Point(1, 1), Point(2, 4)};
    std::cout << "Generic container (Points): " << serializeContainer(points) << std::endl;

    std::vector<third_party::Product> products = {
        third_party::Product("A-100", 19.99),
        third_party::Product("B-200", 29.99)
    };
    std::cout << "Generic container (Products): " << serializeContainer(products) << std::endl;

    // ✅ WORKS: Complex nesting
    std::vector<std::vector<int>> matrix = {{1, 2}, {3, 4}, {5, 6}};
    std::cout << "Nested vector: " << toJSON(matrix) << std::endl;

    std::cout << "\n=== Summary of Advantages ===" << std::endl;
    std::cout << "✅ Non-intrusive: No modification to existing types" << std::endl;
    std::cout << "✅ Decoupled: Types have zero knowledge of serialization" << std::endl;
    std::cout << "✅ Universal: Works with primitives, std types, third-party types" << std::endl;
    std::cout << "✅ Zero overhead: Compile-time dispatch, direct function calls" << std::endl;
    std::cout << "✅ Composable: Automatic support for nested structures" << std::endl;
    std::cout << "✅ Natural: Clean function call syntax" << std::endl;

    return 0;
}
