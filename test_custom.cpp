// Test file for custom function specifier

// Valid: custom on free function
custom void validCustomFunction() {
}

// Invalid: custom on member function
struct MyClass {
  custom void memberFunction();  // Should error
  custom MyClass();              // Should error (constructor)
  custom ~MyClass();             // Should error (destructor)
};

// Invalid: custom with inline
custom inline void invalidCombo() {  // Should error
}

// Valid: custom without inline
custom void anotherValidCustom() {
}

int main() {
  validCustomFunction();
  return 0;
}
