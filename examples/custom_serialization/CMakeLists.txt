# Regression test: Custom keyword vs TInCuP CPO equivalence
#
# This CMake file demonstrates that C++29 'custom' keyword produces
# semantically identical behavior to C++20 tag_invoke CPO patterns.
#
# Key point: Custom version uses modified clang, TInCuP version uses
# standard C++20 compiler - proving compatibility!

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

# Find standard C++20 compiler for TInCuP version
# Try g++ first, then system clang++, then fall back to CMake's compiler
find_program(SYSTEM_CXX_COMPILER
  NAMES g++-13 g++-12 g++-11 g++ clang++-18 clang++-17 clang++-16 clang++
  PATHS /usr/bin /usr/local/bin
  DOC "Standard C++20 compiler for TInCuP version"
)

if(NOT SYSTEM_CXX_COMPILER)
  set(SYSTEM_CXX_COMPILER ${CMAKE_CXX_COMPILER})
  message(STATUS "Using CMake's default compiler for TInCuP version")
else()
  message(STATUS "Found system C++ compiler: ${SYSTEM_CXX_COMPILER}")
endif()

# Verify it supports C++20
execute_process(
  COMMAND ${SYSTEM_CXX_COMPILER} -std=c++20 -xc++ -E -
  INPUT_FILE /dev/null
  OUTPUT_QUIET
  ERROR_QUIET
  RESULT_VARIABLE CXX20_RESULT
)

if(NOT CXX20_RESULT EQUAL 0)
  message(FATAL_ERROR "${SYSTEM_CXX_COMPILER} does not support C++20")
endif()

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

# Test 2: TInCuP version (standard C++20 - uses system compiler)
# This is compiled separately to avoid using modified clang
add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/test_tincup_serialization
  COMMAND ${SYSTEM_CXX_COMPILER}
    -std=c++20
    -nostdlib++
    -I${TINCUP_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/test_tincup_real.cpp
    -o ${CMAKE_CURRENT_BINARY_DIR}/test_tincup_serialization
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/test_tincup_real.cpp
  COMMENT "Building TInCuP version with ${SYSTEM_CXX_COMPILER}"
  VERBATIM
)

add_custom_target(test_tincup_serialization_target ALL
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/test_tincup_serialization
)

# Create tests
enable_testing()

add_test(NAME RunCustomVersion
  COMMAND test_custom_serialization
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

add_test(NAME RunTInCuPVersion
  COMMAND ${CMAKE_CURRENT_BINARY_DIR}/test_tincup_serialization
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# Test that compares outputs
add_test(NAME CustomTInCuPEquivalence
  COMMAND ${CMAKE_COMMAND}
    -DCUSTOM_EXE=$<TARGET_FILE:test_custom_serialization>
    -DTINCUP_EXE=${CMAKE_CURRENT_BINARY_DIR}/test_tincup_serialization
    -P ${CMAKE_CURRENT_SOURCE_DIR}/run_equivalence_test.cmake
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# Make sure TInCuP binary is built before running tests
add_dependencies(test_custom_serialization test_tincup_serialization_target)

# Install test binaries (optional)
install(PROGRAMS
  $<TARGET_FILE:test_custom_serialization>
  ${CMAKE_CURRENT_BINARY_DIR}/test_tincup_serialization
  DESTINATION bin/examples
)

# Configuration summary
message(STATUS "")
message(STATUS "Custom vs TInCuP Regression Test Configuration:")
message(STATUS "  Custom compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "  TInCuP compiler: ${SYSTEM_CXX_COMPILER}")
message(STATUS "  C++ Standard: C++20")
message(STATUS "  TInCuP Headers: ${TINCUP_INCLUDE_DIR}")
message(STATUS "  Custom Test: test_custom_serialization")
message(STATUS "  TInCuP Test: test_tincup_serialization")
message(STATUS "")
message(STATUS "This demonstrates that:")
message(STATUS "  - Custom version requires modified clang")
message(STATUS "  - TInCuP version works with ANY C++20 compiler")
message(STATUS "  - Both produce identical runtime behavior")
message(STATUS "")
