// Test program using TInCuP CPO version
#include "serialize_tincup_impl.hpp"
#include <sstream>
#include <iostream>

int main() {
    using namespace test_types;
    using namespace serialization;

    // Test Point serialization
    Point p1{10, 20};
    std::ostringstream oss1;
    serialize(p1, oss1);
    std::cout << "Point serialize: " << oss1.str() << std::endl;

    // Test Point clone
    Point p2 = clone(p1);
    std::ostringstream oss2;
    serialize(p2, oss2);
    std::cout << "Point clone: " << oss2.str() << std::endl;
    std::cout << "Point clone equals original: " << (p1 == p2 ? "true" : "false") << std::endl;

    // Test Color serialization
    Color c1{255, 128, 64};
    std::ostringstream oss3;
    serialize(c1, oss3);
    std::cout << "Color serialize: " << oss3.str() << std::endl;

    // Test Color clone
    Color c2 = clone(c1);
    std::ostringstream oss4;
    serialize(c2, oss4);
    std::cout << "Color clone: " << oss4.str() << std::endl;
    std::cout << "Color clone equals original: " << (c1 == c2 ? "true" : "false") << std::endl;

    return 0;
}
