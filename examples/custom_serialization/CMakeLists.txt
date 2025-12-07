# Regression test: Custom keyword vs TInCuP CPO equivalence
#
# This CMake file demonstrates that C++29 'custom' keyword produces
# semantically identical behavior to C++20 tag_invoke CPO patterns.

cmake_minimum_required(VERSION 3.20)
project(CustomTInCuPRegression LANGUAGES CXX)

# Find TInCuP headers
# Try local path first, then fall back to FetchContent
set(TINCUP_INCLUDE_DIR "" CACHE PATH "Path to TInCuP include directory")

if(NOT TINCUP_INCLUDE_DIR)
  # Try common locations
  set(POSSIBLE_PATHS
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../TInCuP/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../../TInCuP/include"
  )

  foreach(PATH ${POSSIBLE_PATHS})
    if(EXISTS "${PATH}/tincup/tincup.hpp")
      set(TINCUP_INCLUDE_DIR "${PATH}")
      message(STATUS "Found TInCuP at: ${TINCUP_INCLUDE_DIR}")
      break()
    endif()
  endforeach()
endif()

if(NOT TINCUP_INCLUDE_DIR OR NOT EXISTS "${TINCUP_INCLUDE_DIR}/tincup/tincup.hpp")
  message(STATUS "TInCuP not found locally, fetching from GitHub...")
  include(FetchContent)
  FetchContent_Declare(
    TInCuP
    GIT_REPOSITORY https://github.com/sandialabs/TInCuP.git
    GIT_TAG main
    GIT_SHALLOW TRUE
  )
  FetchContent_Populate(TInCuP)
  set(TINCUP_INCLUDE_DIR "${tincup_SOURCE_DIR}/include")
endif()

if(NOT EXISTS "${TINCUP_INCLUDE_DIR}/tincup/tincup.hpp")
  message(FATAL_ERROR "TInCuP headers not found. Set TINCUP_INCLUDE_DIR manually.")
endif()

message(STATUS "Using TInCuP headers from: ${TINCUP_INCLUDE_DIR}")

# Test 1: Custom keyword version (requires modified clang)
add_executable(test_custom_serialization
  test_custom_simple.cpp
)

set_target_properties(test_custom_serialization PROPERTIES
  CXX_STANDARD 20
  CXX_STANDARD_REQUIRED ON
)

target_compile_options(test_custom_serialization PRIVATE
  -fcustomizable-functions
  -nostdlib++
)

# Test 2: TInCuP version (standard C++20)
add_executable(test_tincup_serialization
  test_tincup_real.cpp
)

target_include_directories(test_tincup_serialization PRIVATE
  ${TINCUP_INCLUDE_DIR}
)

target_compile_options(test_tincup_serialization PRIVATE
  -nostdlib++
)

set_target_properties(test_tincup_serialization PROPERTIES
  CXX_STANDARD 20
  CXX_STANDARD_REQUIRED ON
)

# Create tests
enable_testing()

add_test(NAME RunCustomVersion
  COMMAND test_custom_serialization
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

add_test(NAME RunTInCuPVersion
  COMMAND test_tincup_serialization
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# Test that compares outputs
add_test(NAME CustomTInCuPEquivalence
  COMMAND ${CMAKE_COMMAND}
    -DCUSTOM_EXE=$<TARGET_FILE:test_custom_serialization>
    -DTINCUP_EXE=$<TARGET_FILE:test_tincup_serialization>
    -P ${CMAKE_CURRENT_SOURCE_DIR}/run_equivalence_test.cmake
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# Install test binaries (optional)
install(TARGETS test_custom_serialization test_tincup_serialization
  RUNTIME DESTINATION bin/examples
)

# Configuration summary
message(STATUS "")
message(STATUS "Custom vs TInCuP Regression Test Configuration:")
message(STATUS "  C++ Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "  C++ Standard: C++20")
message(STATUS "  TInCuP Headers: ${TINCUP_INCLUDE_DIR}")
message(STATUS "  Custom Test: test_custom_serialization")
message(STATUS "  TInCuP Test: test_tincup_serialization")
message(STATUS "")
