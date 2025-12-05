// RUN: %clang_cc1 -std=c++20 -fcustomizable-functions -Wdocumentation -fsyntax-only -verify %s
// expected-no-diagnostics

namespace lib {

/// @custom
/// A customization point for serialization.
/// @customizationpoint
/// Users can provide their own implementations via ADL.
template<typename T>
custom void serialize(const T& obj);

/// @brief Customizable clone operation
/// @customizable
/// This function can be customized for user types by providing
/// an overload in the associated namespace.
/// @tparam T The type to clone
/// @param x The object to clone
/// @returns A copy of the object
template<typename T>
custom T clone(const T& x) {
  return T(x);
}

}

namespace user {

struct MyType {
  int value;
};

/// User-provided customization for serialize.
/// @custom
/// This overload is found via ADL when serialize::serialize(MyType) is called.
void serialize(const MyType& obj) {
  // Implementation
}

/// @brief Custom clone implementation for MyType
/// @customizationpoint
/// Optimized clone using memcpy
MyType clone(const MyType& x) {
  return MyType{x.value};
}

}
