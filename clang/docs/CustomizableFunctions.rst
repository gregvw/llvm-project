============================
Customizable Functions in C++
============================

.. contents::
   :local:

Introduction
============

Clang supports an experimental language extension that adds a ``custom`` function
specifier to C++. This specifier enables functions to be marked as "customizable",
allowing them to be replaced or extended at link time or runtime through backend
mechanisms.

The ``custom`` specifier is implemented as a **contextual keyword**, meaning it
only has special meaning in specific syntactic contexts (as a function specifier)
and can still be used as an identifier elsewhere in your code.

Enabling the Feature
====================

The customizable functions feature is enabled with the ``-fcustomizable-functions``
compiler flag:

.. code-block:: bash

   clang++ -fcustomizable-functions source.cpp

Grammar and Syntax
==================

The ``custom`` keyword is recognized as a function specifier, similar to ``inline``,
``virtual``, or ``explicit``:

.. code-block:: c++

   // Valid: custom on a free function
   custom void myFunction();

   custom int computeValue(int x) {
     return x * 2;
   }

   // Valid: custom with other specifiers (except inline)
   custom constexpr int getValue() { return 42; }
   custom noexcept void safeOperation();

Contextual Keyword Behavior
----------------------------

Since ``custom`` is a contextual keyword, it can still be used as an identifier
in non-specifier contexts:

.. code-block:: c++

   // OK: 'custom' used as a variable name
   int custom = 42;

   // OK: 'custom' used as a type name
   struct custom {
     int value;
   };
   custom x;

   // OK: 'custom' as function specifier
   custom void foo();

This backward compatibility ensures existing code using ``custom`` as an identifier
continues to compile without modification.

Restrictions
============

The ``custom`` specifier has several restrictions to maintain clear semantics:

Free Functions Only
-------------------

The ``custom`` specifier can only be applied to free functions, not member functions:

.. code-block:: c++

   // Valid
   custom void freeFunction();

   // Invalid: custom on member function
   class MyClass {
     custom void memberFunction();  // Error
   };

No Special Member Functions
----------------------------

Constructors and destructors cannot be marked ``custom``:

.. code-block:: c++

   class MyClass {
     custom MyClass();   // Error: cannot apply to constructor
     custom ~MyClass();  // Error: cannot apply to destructor
   };

Incompatible with inline
-------------------------

The ``custom`` and ``inline`` specifiers are mutually exclusive:

.. code-block:: c++

   custom inline void foo();  // Error: cannot combine custom with inline

Identifier Names Only
---------------------

The ``custom`` specifier can only be applied to functions with identifier names,
not operators, conversion functions, or user-defined literals:

.. code-block:: c++

   // Valid: identifier name
   custom int compute(int x);

   // Invalid: operator overload
   custom int operator+(int a, int b);  // Error

   // Invalid: conversion function
   custom operator bool();  // Error

   // Invalid: user-defined literal
   custom long double operator""_km(long double);  // Error

**Rationale**: The customization mechanism uses the function name in attributes
and override symbols. Non-identifier names cannot be represented in these
contexts without complex escaping, and operators already have Argument-Dependent
Lookup (ADL) for customization.

External Linkage Only
---------------------

The ``custom`` specifier requires functions to have external linkage:

.. code-block:: c++

   // Valid: external linkage
   custom int compute(int x);

   // Valid: external linkage with extern "C"
   extern "C" custom int process(int x);

   // Valid: external linkage in namespace
   namespace utils {
     custom int helper(int x);
   }

   // Invalid: static (internal linkage)
   static custom int foo(int x);  // Error

   // Invalid: anonymous namespace (internal linkage)
   namespace {
     custom int bar(int x);  // Error
   }

**Rationale**: The implementation generates wrapper functions with ``linkonce_odr``
linkage to enable link-time customization. Silently upgrading internal linkage
to external linkage would violate the programmer's intent and cause unexpected
ABI/visibility changes. The link-time override mechanism requires externally
visible symbols.

Semantic Meaning
================

A function declared with the ``custom`` specifier is marked in the AST and
propagated to LLVM IR as a function attribute. This enables backend passes
and code generation to recognize customizable functions and apply appropriate
transformations.

The exact behavior of customizable functions depends on the target backend
and linking strategy, but common use cases include:

- **Hot-patching**: Allowing function implementations to be replaced at runtime
- **Link-time customization**: Selecting different implementations during linking
- **Optimization hints**: Informing the optimizer about functions that may be replaced
- **Instrumentation points**: Marking functions for profiling or tracing

Implementation Details
======================

Language Option
---------------

The feature is controlled by the ``CustomizableFunctions`` language option,
which is enabled by the ``-fcustomizable-functions`` flag.

AST Representation
------------------

The ``custom`` specifier is represented in the Abstract Syntax Tree (AST) as:

- A bit flag in ``DeclSpec`` (``FS_custom_specified``)
- A bit flag in ``FunctionDeclBitfields`` (``IsCustom``)
- Accessor methods: ``FunctionDecl::isCustom()`` and ``FunctionDecl::setCustom()``

Override Resolution and Mangled Names
--------------------------------------

The customization mechanism uses **mangled names** for override resolution to
ensure correct disambiguation across namespaces, overloads, and template
instantiations:

.. code-block:: c++

   namespace lib1 {
     custom int compute(int x);  // Mangled as: _ZN4lib17computeEi
   }

   namespace lib2 {
     custom int compute(int x);  // Mangled as: _ZN4lib27computeEi
   }

   // Different overloads
   custom int add(int a, int b);     // Mangled as: _ZN3addEii
   custom double add(double a, double b);  // Mangled as: _ZN3addEdd

Each customizable function's wrapper is annotated with two attributes:

- ``clang-customizable-function``: Contains the **mangled name** for unique identification
- ``clang-customizable-function-name``: Contains the **pretty name** for diagnostics

At link time, the LLVM CustomizableFunctions pass looks for override symbols
with the naming convention ``__custom_override_<mangled_name>``. For example:

.. code-block:: c++

   // To override lib1::compute(int):
   extern "C" int __custom_override__ZN4lib17computeEi(int x) {
     return x * 3;  // Custom implementation
   }

This mangled-name approach prevents collisions that would occur with unqualified
names, ensuring that each function can be independently customized.

Parser Recognition
------------------

The parser recognizes ``custom`` contextually by:

1. Checking if the token is an identifier with spelling "custom"
2. Verifying the ``CustomizableFunctions`` language option is enabled
3. Only treating it as a specifier in declaration-specifier context

This approach ensures ``custom`` remains available as an identifier in other contexts.

Diagnostics
===========

The implementation provides clear error messages for invalid uses:

.. code-block:: c++

   class Foo {
     custom void method();
     // error: 'custom' can only be applied to free functions, not member functions

     custom Foo();
     // error: 'custom' cannot be applied to constructors

     custom ~Foo();
     // error: 'custom' cannot be applied to destructors
   };

   custom inline void bar();
   // error: 'custom' and 'inline' cannot be combined

   custom int operator+(int a, int b);
   // error: 'custom' can only be applied to functions with identifier names,
   //        not operators

   static custom int helper(int x);
   // error: 'custom' can only be applied to functions with external linkage

   namespace {
     custom int internal(int x);
     // error: 'custom' can only be applied to functions with external linkage
   }

Examples
========

Basic Usage
-----------

.. code-block:: c++

   // Declaration
   custom void initialize();

   // Definition
   custom void initialize() {
     // Implementation
   }

   // Function template
   template<typename T>
   custom T process(T value) {
     return value;
   }

Interaction with Other Features
--------------------------------

.. code-block:: c++

   // With constexpr (allowed)
   custom constexpr int compute() { return 42; }

   // With noexcept (allowed)
   custom void critical() noexcept;

   // With attributes (allowed)
   [[nodiscard]] custom int getValue();

   // With trailing return type (allowed)
   custom auto calculate(int x) -> int;

Checking for Support
====================

You can check for support of customizable functions at compile time:

.. code-block:: c++

   #ifdef __clang__
   #if __has_feature(customizable_functions)
     custom void optimizedPath();
   #else
     void optimizedPath();  // Fallback
   #endif
   #endif

Future Directions
=================

This is an experimental feature under active development. Future enhancements
may include:

- Support for member functions with explicit opt-in
- Integration with modules
- Standardized ABI for customization mechanisms
- Additional backend optimization strategies

References
==========

- Clang Language Extensions: :doc:`LanguageExtensions`
- Function Attributes: :doc:`AttributeReference`
- Clang AST: :doc:`ClangAST`
