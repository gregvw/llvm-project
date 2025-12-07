# CMake script to run both executables and compare outputs
# This is called by CTest to verify equivalence

# Check that required variables are set
if(NOT DEFINED CUSTOM_EXE OR NOT DEFINED TINCUP_EXE)
  message(FATAL_ERROR "CUSTOM_EXE and TINCUP_EXE must be defined")
endif()

# Run custom version
execute_process(
  COMMAND ${CUSTOM_EXE}
  OUTPUT_VARIABLE CUSTOM_OUTPUT
  RESULT_VARIABLE CUSTOM_RESULT
  ERROR_VARIABLE CUSTOM_ERROR
)

if(NOT CUSTOM_RESULT EQUAL 0)
  message(FATAL_ERROR "Custom version failed with code ${CUSTOM_RESULT}\nError: ${CUSTOM_ERROR}")
endif()

# Run TInCuP version
execute_process(
  COMMAND ${TINCUP_EXE}
  OUTPUT_VARIABLE TINCUP_OUTPUT
  RESULT_VARIABLE TINCUP_RESULT
  ERROR_VARIABLE TINCUP_ERROR
)

if(NOT TINCUP_RESULT EQUAL 0)
  message(FATAL_ERROR "TInCuP version failed with code ${TINCUP_RESULT}\nError: ${TINCUP_ERROR}")
endif()

# Compare outputs
if(CUSTOM_OUTPUT STREQUAL TINCUP_OUTPUT)
  message(STATUS "✓ SUCCESS: Custom and TInCuP outputs are identical!")
  message(STATUS "")
  message(STATUS "Output:")
  message(STATUS "${CUSTOM_OUTPUT}")
else()
  message(FATAL_ERROR "✗ FAILURE: Outputs differ!\n\nCustom:\n${CUSTOM_OUTPUT}\n\nTInCuP:\n${TINCUP_OUTPUT}")
endif()
