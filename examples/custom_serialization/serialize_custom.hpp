// Example: Custom function declarations for serialization
// This demonstrates the C++29 `custom` keyword proposal

#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <sstream>

namespace serialization {

// Custom serialization function - the customization point
template<typename T>
custom void serialize(const T& obj, std::ostream& os);

// Custom clone function
template<typename T>
custom T clone(const T& x);

} // namespace serialization

// Test types
namespace test_types {

struct Point {
    int x, y;

    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }
};

struct Color {
    unsigned char r, g, b;

    bool operator==(const Color& other) const {
        return r == other.r && g == other.g && b == other.b;
    }
};

} // namespace test_types

// User implementations via ADL
namespace test_types {

// Serialize Point
void serialize(const Point& p, std::ostream& os) {
    os << "Point(" << p.x << ", " << p.y << ")";
}

// Serialize Color
void serialize(const Color& c, std::ostream& os) {
    os << "Color("
       << static_cast<int>(c.r) << ", "
       << static_cast<int>(c.g) << ", "
       << static_cast<int>(c.b) << ")";
}

// Clone Point
Point clone(const Point& p) {
    return Point{p.x, p.y};
}

// Clone Color
Color clone(const Color& c) {
    return Color{c.r, c.g, c.b};
}

} // namespace test_types
