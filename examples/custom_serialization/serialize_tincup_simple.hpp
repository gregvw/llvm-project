// TInCuP CPO implementation for serialization (ultra-simplified)
// This is the C++20-compatible version

#pragma once

namespace serialization {

// Forward declarations for tag_invoke overloads (will be defined later via ADL)
template<typename T>
void tag_invoke(auto, const T&, char*);

template<typename T>
T tag_invoke(auto, const T&);

// CPO for serialize
inline constexpr struct serialize_ftor final {
    template<typename T>
    void operator()(const T& obj, char* buffer) const {
        tag_invoke(*this, obj, buffer);
    }
} serialize;

// CPO for clone
inline constexpr struct clone_ftor final {
    template<typename T>
    T operator()(const T& x) const {
        return tag_invoke(*this, x);
    }
} clone;

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

// Tag_invoke implementations for Point
void tag_invoke(serialization::serialize_ftor, const Point& p, char* buffer) {
    buffer[0] = 'P';
    __builtin_memcpy(buffer + 1, &p.x, sizeof(int));
    __builtin_memcpy(buffer + 5, &p.y, sizeof(int));
}

Point tag_invoke(serialization::clone_ftor, const Point& p) {
    return Point{p.x, p.y};
}

// Tag_invoke implementations for Color
void tag_invoke(serialization::serialize_ftor, const Color& c, char* buffer) {
    buffer[0] = 'C';
    buffer[1] = c.r;
    buffer[2] = c.g;
    buffer[3] = c.b;
}

Color tag_invoke(serialization::clone_ftor, const Color& c) {
    return Color{c.r, c.g, c.b};
}

} // namespace test_types
