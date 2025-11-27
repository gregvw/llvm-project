===========================================
Customizable Functions: Serialization Example
===========================================

This example demonstrates the practical advantages of ``custom`` functions
over traditional OOP inheritance for cross-cutting concerns like serialization.

Overview
========

This directory contains two implementations of JSON serialization:

1. **oop_approach.cpp** - Traditional inheritance-based approach (problematic)
2. **custom_approach.cpp** - Custom functions approach (recommended)

The Problem
===========

Traditional OOP serialization requires:

- Modifying all types to inherit from ``Serializable``
- Tight coupling between domain model and serialization library
- Cannot serialize primitives, std types, or third-party types
- Virtual function overhead on every call
- Heap allocation for polymorphic collections

The Solution
============

Custom functions provide:

- **Non-intrusive**: No modification to existing types
- **Universal**: Works with primitives, std types, third-party types
- **Zero overhead**: Direct calls, fully inlinable
- **Generic algorithms**: Single implementation for all types

Building
========

OOP Approach
------------

.. code-block:: bash

   clang++ -std=c++20 oop_approach.cpp -o oop_demo
   ./oop_demo

Custom Functions Approach
-------------------------

.. code-block:: bash

   clang++ -std=c++20 -fcustomizable-functions \\
     -fcustomizable-functions-sema custom_approach.cpp -o custom_demo
   ./custom_demo

Key Differences
===============

What Works
----------

==================  ==============  ===============
Feature             OOP             Custom
==================  ==============  ===============
User types          ✅ Works        ✅ Works
Primitives          ⚠️  Workaround  ✅ Native
std::string         ⚠️  Workaround  ✅ Native
Third-party types   ❌ Fails        ✅ Works
std::vector<int>    ❌ Fails        ✅ Works
Nested structures   ❌ Fails        ✅ Works
Generic algorithms  ❌ Fails        ✅ Works
==================  ==============  ===============

Performance
-----------

============  ===============================  =============================
Operation     OOP                              Custom
============  ===============================  =============================
Function call Virtual dispatch (~5-10 cycles)  Direct call (0 cycles, inlined)
Memory        20% overhead (v-table pointers)  0% overhead
============  ===============================  =============================

Example Usage
=============

Custom Functions Approach
--------------------------

.. code-block:: cpp

   // Define customization point
   namespace serialize {
     template<typename T>
     custom std::string toJSON(const T& value);
   }

   // Your clean data class (no inheritance!)
   struct Person {
     std::string name;
     int age;
   };

   // Provide serialization externally
   std::string toJSON(const Person& p) {
     using serialize::toJSON;
     return "{\"name\": " + toJSON(p.name) +
            ", \"age\": " + toJSON(p.age) + "}";
   }

   // Use it
   Person p{"Alice", 30};
   std::cout << serialize::toJSON(p);
   // Output: {"name": "Alice", "age": 30}

See Also
========

- ``clang/docs/CustomizableFunctions.rst`` - Full documentation
- ``clang/docs/LanguageExtensions.rst`` - Language extensions overview
