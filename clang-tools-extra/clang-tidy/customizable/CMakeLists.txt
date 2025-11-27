if(TARGET clang-tidy-headers)
  add_custom_target(clang-tidy-customizable-headers)
  set_target_properties(clang-tidy-customizable-headers PROPERTIES FOLDER "Clang Tools Extra/clang-tidy/Checks")
  add_dependencies(clang-tidy-headers clang-tidy-customizable-headers)
endif()

set(LLVM_LINK_COMPONENTS
  Support
  )

add_clang_library(clangTidyCustomizableModule
  CustomizableTidyModule.cpp
  SuggestCustomCheck.cpp

  LINK_LIBS
  clangTidy
  clangTidyUtils

  DEPENDS
  omp_gen
  ClangDriverOptions
  )

clang_target_link_libraries(clangTidyCustomizableModule
  PRIVATE
  clangAST
  clangASTMatchers
  clangBasic
  clangLex
  )

target_include_directories(clangTidyCustomizableModule SYSTEM
  PRIVATE
  ${CLANG_INCLUDE_DIRS}
  )
