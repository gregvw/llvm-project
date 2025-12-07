// Simplified custom function declarations (no std library dependencies)
// This demonstrates the C++29 `custom` keyword proposal

#pragma once

namespace serialization {

// Custom serialization function - writes to a buffer
template<typename T>
custom void serialize(const T& obj, char* buffer);

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

// Serialize Point to buffer
void serialize(const Point& p, char* buffer) {
    buffer[0] = 'P';
    __builtin_memcpy(buffer + 1, &p.x, sizeof(int));
    __builtin_memcpy(buffer + 5, &p.y, sizeof(int));
}

// Serialize Color to buffer
void serialize(const Color& c, char* buffer) {
    buffer[0] = 'C';
    buffer[1] = c.r;
    buffer[2] = c.g;
    buffer[3] = c.b;
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
