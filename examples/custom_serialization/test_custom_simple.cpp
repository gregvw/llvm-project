// Test program using custom keyword version (no std library)
#include "serialize_custom_simple.hpp"

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
