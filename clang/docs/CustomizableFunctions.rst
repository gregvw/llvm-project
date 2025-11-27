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

Motivation
==========

The need for explicit customization points in C++ has been a long-standing
challenge. Traditional approaches using Argument-Dependent Lookup (ADL) are
subtle and error-prone, while library solutions like customization point objects
(CPOs) or ``tag_invoke`` add complexity and can impact compile times and
diagnostics.

**Key problems this feature addresses:**

- **ADL complexity**: Proper creation and use of ADL-based customization points
  requires expert knowledge and careful attention to detail

- **Library overhead**: CPO and ``tag_invoke`` approaches introduce template
  machinery and indirection that affects compile times and error messages

- **Lack of explicit opt-in**: No clear language mechanism to declare that a
  function is intended as a customization point

- **Interface clarity**: Hard to understand which functions are customizable
  without reading extensive documentation

This feature provides a simple, explicit syntax for declaring customizable
functions at the language level, with efficient link-time override resolution
that avoids the complexity of existing library-based approaches.

Enabling the Feature
====================

The customizable functions feature is enabled with the ``-fcustomizable-functions``
compiler flag:

.. code-block:: bash

   clang++ -fcustomizable-functions source.cpp

To enable override substitution at link time, you must also use LTO:

.. code-block:: bash

   clang++ -fcustomizable-functions -flto source.cpp override.cpp -o program

Alternatively, for the experimental Sema-level ADL-based override mechanism:

.. code-block:: bash

   clang++ -fcustomizable-functions -fcustomizable-functions-sema source.cpp

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
   custom int add(int a, int b);     // Mangled as: _Z3addii
   custom double add(double a, double b);  // Mangled as: _Z3adddd

Each customizable function's wrapper is annotated with two attributes:

- ``clang-customizable-function``: Contains the **mangled name** for unique identification
- ``clang-customizable-function-name``: Contains the **pretty name** for diagnostics

At link time (during LTO or ThinLTO), the LLVM CustomizableFunctions pass
looks for override symbols with the naming convention ``__custom_override_<mangled_name>``.
For example:

.. code-block:: c++

   // To override lib1::compute(int):
   extern "C" int __custom_override__ZN4lib17computeEi(int x) {
     return x * 3;  // Custom implementation
   }

This mangled-name approach prevents collisions that would occur with unqualified
names, ensuring that each function can be independently customized.

**Important requirements for override substitution:**

- **LTO/ThinLTO required**: Override substitution only happens during link-time
  optimization. Non-LTO builds will continue to call the ``.default`` implementation.
  Use ``-flto`` or ``-flto=thin`` to enable override resolution.

- **Exact signature match**: The override symbol must have the same signature and
  calling convention as the wrapper function. If the signature doesn't match, the
  pass will leave the wrapper calling the ``.default`` body and may emit a warning.

Experimental Sema-Level Override (ADL-based)
--------------------------------------------

An experimental alternative override mechanism is available via the
``-fcustomizable-functions-sema`` flag. When enabled, override resolution
happens at the call site during semantic analysis using Argument-Dependent
Lookup (ADL) instead of link-time substitution.

With this flag:

- The compiler looks for an ADL-visible function in the same namespace as
  the customizable function's arguments
- The override function should be named with a special pattern that Sema
  recognizes
- Override resolution happens at compile time, not link time
- No LTO is required for this mechanism

**This is highly experimental** and the naming convention and exact behavior
may change. The default link-time mechanism (described above) is the
recommended and stable approach.

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

Overriding a Customizable Function
-----------------------------------

Here's a complete example showing how to override a customizable function using LTO:

**library.cpp** (defines customizable function):

.. code-block:: c++

   // Library provides a default allocator
   custom void* allocate(size_t size) {
     return malloc(size);  // Default implementation
   }

   void library_function() {
     void* ptr = allocate(100);  // Calls customizable allocate
     // ... use ptr ...
   }

**override.cpp** (provides custom override):

.. code-block:: c++

   #include <cstddef>

   // Override the allocate function with a custom implementation
   // Use the mangled name: _Z8allocatem (for allocate(size_t))
   extern "C" void* __custom_override__Z8allocatem(size_t size) {
     // Custom pool allocator implementation
     return my_pool_allocator(size);
   }

**Building with LTO:**

.. code-block:: bash

   clang++ -fcustomizable-functions -flto -c library.cpp -o library.o
   clang++ -flto -c override.cpp -o override.o
   clang++ -flto library.o override.o -o program

At link time, the ``CustomizableFunctions`` pass will replace calls to
``allocate`` with calls to ``__custom_override__Z8allocatem``.

Finding the Mangled Name
-------------------------

To determine the mangled name for a customizable function, you can use one
of these approaches:

**Method 1: Check LLVM IR attributes**

.. code-block:: bash

   clang++ -fcustomizable-functions -S -emit-llvm library.cpp -o library.ll
   grep 'clang-customizable-function' library.ll

Output will show the attribute with the mangled name:

.. code-block:: llvm

   attributes #0 = { ... "clang-customizable-function"="_Z8allocatem" ... }

**Method 2: Use nm with demangling**

.. code-block:: bash

   clang++ -fcustomizable-functions -c library.cpp -o library.o
   nm library.o | grep -i allocate

This shows the raw mangled names. Use ``nm -C`` to see demangled names alongside.

Overload Example
----------------

When overloading customizable functions, each overload gets a unique mangled name:

.. code-block:: c++

   // library.cpp
   custom int add(int a, int b) {
     return a + b;
   }

   custom double add(double a, double b) {
     return a + b;
   }

.. code-block:: bash

   # Find the mangled names
   clang++ -fcustomizable-functions -S -emit-llvm library.cpp -o library.ll
   grep 'clang-customizable-function' library.ll
   # Shows: _Z3addii (int version) and _Z3adddd (double version)

.. code-block:: c++

   // override.cpp - override only the int version
   extern "C" int __custom_override__Z3addii(int a, int b) {
     return a + b + 1;  // Custom implementation for int
   }

   // The double version continues using the default implementation

Template Instantiation Example
-------------------------------

Template instantiations also get unique mangled names:

.. code-block:: c++

   // library.cpp
   template<typename T>
   custom T process(T value) {
     return value * 2;
   }

   // Explicit instantiations
   template int process<int>(int);
   template double process<double>(double);

.. code-block:: bash

   # Find mangled names for instantiations
   clang++ -fcustomizable-functions -S -emit-llvm library.cpp -o library.ll
   grep 'clang-customizable-function' library.ll
   # Shows: _Z7processIiET_S0_ (int) and _Z7processIdET_S0_ (double)

.. code-block:: c++

   // override.cpp - override just the int instantiation
   extern "C" int __custom_override__Z7processIiET_S0_(int value) {
     return value * 3;  // Different implementation for int
   }

Extern "C" Example
------------------

Functions with C linkage have simpler, unmangled names:

.. code-block:: c++

   // library.cpp
   extern "C" custom int compute(int x) {
     return x * 2;
   }

Since there's no C++ name mangling, the override symbol is straightforward:

.. code-block:: c++

   // override.cpp
   extern "C" int __custom_override_compute(int x) {
     return x * 3;  // Override implementation
   }

No need to look up mangled names—just use ``__custom_override_<function_name>``.

Sema-Level Override Example (Experimental)
-------------------------------------------

With ``-fcustomizable-functions-sema``, you can override functions using
ADL at the call site instead of link-time substitution:

.. code-block:: c++

   // library.cpp
   namespace lib {
     struct Point { int x, y; };

     custom void transform(Point& p) {
       p.x *= 2;
       p.y *= 2;
     }

     void use_transform() {
       Point p{1, 2};
       transform(p);  // ADL will find overrides in same namespace as Point
     }
   }

.. code-block:: c++

   // override.cpp
   namespace lib {
     // Sema finds this via ADL when compiling the call site
     void transform(Point& p) {
       // Custom implementation - triple instead of double
       p.x *= 3;
       p.y *= 3;
     }
   }

.. code-block:: bash

   # Compile with Sema override enabled
   clang++ -fcustomizable-functions -fcustomizable-functions-sema \
           library.cpp override.cpp -o program

The override resolution happens at compile time based on ADL visibility.
**Note**: This mechanism is highly experimental and subject to change.

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

Related Work
------------

This feature is inspired by and addresses problems identified in several WG21
proposals for language-level customization mechanisms:

**P1292R0: Customization Point Functions** (2018)
  Daveed Vandevoorde's proposal for explicit syntax to declare customization
  point functions using ``virtual`` at namespace scope with explicit ``override``
  specifiers. Identifies the complexity of ADL-based customization and proposes
  language support for hierarchical override chains.

  https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p1292r0.html

**P1665R0: Range Adaptors: Property Customisation Points** (2019)
  Discusses customization mechanisms for C++20 ranges, exploring how to allow
  users to customize behavior of range adaptors and standard library components.

  https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1665r0.pdf

**P2279R0: We need a language mechanism for customization points** (2021)
  Barry Revzin's analysis comparing existing customization strategies (virtual
  functions, template specialization, ADL, CPOs, ``tag_invoke``) and arguing
  that none comprehensively solve the problem. Advocates for dedicated language
  support similar to Rust traits.

  https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2021/p2279r0.html

**P2547R1: Language Support for Customisable Functions** (2022)
  Proposes a language mechanism for defining customizable namespace-scoped
  functions as an alternative to ``tag_invoke``, aiming to simplify library
  customization points while improving compile times and error messages.

  https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2022/p2547r1.html

Clang Documentation
-------------------

- Clang Language Extensions: :doc:`LanguageExtensions`
- Function Attributes: :doc:`AttributeReference`
- Clang AST: :doc:`ClangAST`
