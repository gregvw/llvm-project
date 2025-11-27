====================================================
Documenting Customizable Functions with Doxygen
====================================================

Overview
========

Clang now recognizes special Doxygen commands for documenting customizable
functions. These help generate better API documentation and make the
customization points clear to users.

New Doxygen Commands
=====================

Function Declaration Commands
------------------------------

``@custom``
  Marks a function as a customization point (verbatim line command).
  Use this to declare that a function can be customized.

Block Commands
--------------

``@customizationpoint``
  Provides detailed explanation of how this customization point works.
  Can contain multiple paragraphs describing the customization mechanism.

``@customizable``
  Documents that a function can be customized.
  Use for general descriptions of customizability.

Usage Examples
==============

Basic Customization Point
--------------------------

.. code-block:: cpp

   /// @custom
   /// Serialize an object to JSON.
   /// @customizationpoint
   /// Users can provide their own serialization by defining an overload
   /// in the same namespace as their type. The overload will be found
   /// via ADL (Argument Dependent Lookup).
   /// @tparam T The type to serialize
   /// @param obj The object to serialize
   /// @returns JSON string representation
   template<typename T>
   custom std::string toJSON(const T& obj);

Detailed Documentation
-----------------------

.. code-block:: cpp

   /// @brief Clone operation
   /// @custom
   /// @customizable
   /// This is a customization point for cloning objects. Users can provide
   /// optimized implementations for their types.
   ///
   /// ## Default Behavior
   /// The default implementation uses the copy constructor.
   ///
   /// ## Customization
   /// To customize, provide an overload in your type's namespace:
   /// @code
   /// namespace user {
   ///   struct MyType { /* ... */ };
   ///
   ///   MyType clone(const MyType& x) {
   ///     // Your custom implementation
   ///     return MyType(x);
   ///   }
   /// }
   /// @endcode
   ///
   /// @tparam T The type to clone
   /// @param x The object to clone
   /// @returns A copy of the object
   template<typename T>
   custom T clone(const T& x) {
     return T(x);
   }

User Customization Documentation
---------------------------------

.. code-block:: cpp

   namespace user {
     struct MyVector {
       double* data;
       size_t len;
     };

     /// @brief Custom clone implementation for MyVector
     /// @customizationpoint
     /// Optimized implementation using memcpy for better performance.
     /// This overload is found via ADL when lib::clone(MyVector) is called.
     /// @param x The vector to clone
     /// @returns A new vector with copied data
     MyVector clone(const MyVector& x) {
       MyVector result(x.len);
       memcpy(result.data, x.data, x.len * sizeof(double));
       return result;
     }
   }

Full Example with All Tags
---------------------------

.. code-block:: cpp

   namespace serialize {

     /// @custom
     /// @brief Customization point for object serialization
     /// @customizationpoint
     /// This function provides a customization point for serializing objects
     /// to JSON format. The customization is discovered via ADL (Argument
     /// Dependent Lookup), allowing users to provide implementations for
     /// their types without modifying this library.
     ///
     /// ## Usage
     ///
     /// For built-in types, the library provides implementations:
     /// @code
     /// int x = 42;
     /// auto json = serialize::toJSON(x);  // "42"
     /// @endcode
     ///
     /// For user types, provide an overload in the same namespace:
     /// @code
     /// namespace user {
     ///   struct Person {
     ///     std::string name;
     ///     int age;
     ///   };
     ///
     ///   // This overload is found via ADL
     ///   std::string toJSON(const Person& p) {
     ///     return "{\"name\": \"" + p.name + "\"," +
     ///            " \"age\": " + std::to_string(p.age) + "}";
     ///   }
     /// }
     ///
     /// Person p{"Alice", 30};
     /// auto json = serialize::toJSON(p);  // Calls user::toJSON via ADL
     /// @endcode
     ///
     /// @tparam T The type to serialize
     /// @param value The object to serialize
     /// @returns JSON string representation
     /// @see clone, customize
     /// @since 1.0
     template<typename T>
     custom std::string toJSON(const T& value);

     /// @brief Serialize integers
     /// @customizable
     /// Built-in implementation for integer serialization.
     /// @param value The integer to serialize
     /// @returns String representation of the integer
     inline std::string toJSON(int value) {
       return std::to_string(value);
     }

   }

Integration with Documentation Tools
=====================================

Doxygen
-------

These commands work with standard Doxygen. They will appear in generated
documentation and can be used in Doxygen's index and cross-references.

Example Doxygen output:

- **@custom** appears as a function attribute in the documentation
- **@customizationpoint** creates a detailed section explaining customization
- **@customizable** adds a note that the function can be customized

Other Documentation Tools
-------------------------

- **Sphinx**: Via Breathe, these commands will be included in generated docs
- **Qt Creator**: Recognizes these as valid Doxygen commands
- **VSCode**: With C++ extension, recognizes these in hover tooltips

Best Practices
==============

1. **Use @custom for the declaration**:
   Mark the primary customization point with ``@custom``.

2. **Use @customizationpoint for details**:
   Provide detailed explanation of how users can customize.

3. **Include examples**:
   Show both the customization point definition and user implementation.

4. **Document the discovery mechanism**:
   Mention ADL explicitly so users understand how overrides are found.

5. **Cross-reference related customization points**:
   Use ``@see`` to link related customization points.

Example Template
================

.. code-block:: cpp

   /// @custom
   /// @brief [Brief description of what this customizes]
   /// @customizationpoint
   /// [Detailed explanation of the customization mechanism]
   ///
   /// ## Default Behavior
   /// [What happens if no customization is provided]
   ///
   /// ## Customization
   /// [How users can provide their own implementation]
   /// @code
   /// [Example code showing user customization]
   /// @endcode
   ///
   /// @tparam T [Template parameter description]
   /// @param [param] [Parameter description]
   /// @returns [Return value description]
   /// @see [Related customization points]
   template<typename T>
   custom ReturnType functionName(const T& param);

Comparison with Other Doxygen Commands
=======================================

==================  ===========================  =============================
Command             Purpose                      When to Use
==================  ===========================  =============================
``@function``       Regular function             Non-customizable functions
``@method``         Class method                 Member functions
``@callback``       Callback function            Function pointers/callbacks
``@custom``         Customization point          Customizable functions
``@customizable``   Customizable (block)         Detailed customization docs
==================  ===========================  =============================

See Also
========

- ``clang/docs/CustomizableFunctions.rst`` - Feature documentation
- ``clang/examples/CustomizableFunctions/`` - Working examples
