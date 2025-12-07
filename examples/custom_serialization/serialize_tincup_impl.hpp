// TInCuP CPO implementation for serialization
// This is the C++20-compatible version of serialize_custom.hpp

#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <sstream>

// Note: In a real implementation, you would include the actual TInCuP library
// For this demo, we'll use a simplified inline implementation

namespace tincup {
    // Simplified tag_invoke detection
    template<typename CPO, typename... Args>
    concept tag_invocable = requires(CPO cpo, Args&&... args) {
        tag_invoke(cpo, std::forward<Args>(args)...);
    };

    template<typename CPO, typename... Args>
    using tag_invoke_result_t = decltype(tag_invoke(std::declval<CPO>(), std::declval<Args>()...));

    // Base class for CPOs
    template<typename Derived>
    struct cpo_base {
        template<typename... Args>
        requires tag_invocable<Derived, Args...>
        constexpr auto operator()(Args&&... args) const
            noexcept(noexcept(tag_invoke(std::declval<Derived>(), std::forward<Args>(args)...)))
            -> tag_invoke_result_t<Derived, Args...>
        {
            return tag_invoke(Derived{}, std::forward<Args>(args)...);
        }
    };
}

namespace serialization {

// CPO for serialize
inline constexpr struct serialize_ftor final : tincup::cpo_base<serialize_ftor> {
    using tincup::cpo_base<serialize_ftor>::operator();
} serialize;

// CPO for clone
inline constexpr struct clone_ftor final : tincup::cpo_base<clone_ftor> {
    using tincup::cpo_base<clone_ftor>::operator();
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
void tag_invoke(serialization::serialize_ftor, const Point& p, std::ostream& os) {
    os << "Point(" << p.x << ", " << p.y << ")";
}

Point tag_invoke(serialization::clone_ftor, const Point& p) {
    return Point{p.x, p.y};
}

// Tag_invoke implementations for Color
void tag_invoke(serialization::serialize_ftor, const Color& c, std::ostream& os) {
    os << "Color("
       << static_cast<int>(c.r) << ", "
       << static_cast<int>(c.g) << ", "
       << static_cast<int>(c.b) << ")";
}

Color tag_invoke(serialization::clone_ftor, const Color& c) {
    return Color{c.r, c.g, c.b};
}

} // namespace test_types
