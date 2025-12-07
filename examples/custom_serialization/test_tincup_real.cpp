// Test program using real TInCuP library
#include <tincup/tincup.hpp>

// Simple output functions (no std library)
extern "C" {
    long write(int fd, const void* buf, unsigned long count);
    void exit(int status);
}

void print_str(const char* str) {
    const char* p = str;
    while (*p) ++p;
    write(1, str, p - str);
}

void print_int(int n) {
    if (n < 0) {
        print_str("-");
        n = -n;
    }
    if (n >= 10) print_int(n / 10);
    char c = '0' + (n % 10);
    write(1, &c, 1);
}

// Define CPOs using real TInCuP
namespace serialization {

// Serialize CPO
inline constexpr struct serialize_ftor final : tincup::cpo_base<serialize_ftor> {
    TINCUP_CPO_TAG("serialize")
    inline static constexpr bool is_variadic = false;
    using tincup::cpo_base<serialize_ftor>::operator();

    template<typename T>
    constexpr auto operator()(T&& obj, char* buffer) const
        noexcept(tincup::nothrow_invocable_c<serialize_ftor, T, char*>)
        -> tincup::invocable_t<serialize_ftor, T, char*> {
        return tincup::tag_invoke_cpo(*this, static_cast<T&&>(obj), buffer);
    }
} serialize;

// Clone CPO
inline constexpr struct clone_ftor final : tincup::cpo_base<clone_ftor> {
    TINCUP_CPO_TAG("clone")
    inline static constexpr bool is_variadic = false;
    using tincup::cpo_base<clone_ftor>::operator();

    template<typename T>
    constexpr auto operator()(T&& x) const
        noexcept(tincup::nothrow_invocable_c<clone_ftor, T>)
        -> tincup::invocable_t<clone_ftor, T> {
        return tincup::tag_invoke_cpo(*this, static_cast<T&&>(x));
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

int main() {
    using namespace test_types;
    using namespace serialization;

    char buffer[16];

    // Test Point serialization
    Point p1{10, 20};
    serialize(p1, buffer);
    print_str("Point serialize: buffer[0]=");
    print_int(buffer[0]);
    print_str("\n");

    // Test Point clone
    Point p2 = clone(p1);
    print_str("Point clone: x=");
    print_int(p2.x);
    print_str(", y=");
    print_int(p2.y);
    print_str("\n");
    print_str("Point clone equals original: ");
    print_str(p1 == p2 ? "true" : "false");
    print_str("\n");

    // Test Color serialization
    Color c1{255, 128, 64};
    serialize(c1, buffer);
    print_str("Color serialize: buffer[0]=");
    print_int(buffer[0]);
    print_str("\n");

    // Test Color clone
    Color c2 = clone(c1);
    print_str("Color clone: r=");
    print_int(c2.r);
    print_str(", g=");
    print_int(c2.g);
    print_str(", b=");
    print_int(c2.b);
    print_str("\n");
    print_str("Color clone equals original: ");
    print_str(c1 == c2 ? "true" : "false");
    print_str("\n");

    return 0;
}
