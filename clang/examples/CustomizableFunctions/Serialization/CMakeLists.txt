# Customizable Functions Serialization Example

# OOP approach - standard C++20
add_executable(oop-serialization-demo
  oop_approach.cpp
)

set_target_properties(oop-serialization-demo PROPERTIES
  CXX_STANDARD 20
  CXX_STANDARD_REQUIRED ON
)

# Custom functions approach - requires -fcustomizable-functions
add_executable(custom-serialization-demo
  custom_approach.cpp
)

set_target_properties(custom-serialization-demo PROPERTIES
  CXX_STANDARD 20
  CXX_STANDARD_REQUIRED ON
)

# Add the custom functions flags
target_compile_options(custom-serialization-demo PRIVATE
  -fcustomizable-functions
  -fcustomizable-functions-sema
)
