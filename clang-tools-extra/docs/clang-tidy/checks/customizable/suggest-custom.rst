.. title:: clang-tidy - customizable-suggest-custom

customizable-suggest-custom
============================

Suggests adding the ``custom`` keyword to functions that appear to be
customization points based on their naming patterns.

Functions with names like ``allocate``, ``deallocate``, or ending in ``_hook``,
``_impl``, ``_override``, or ``_customization_point`` are likely intended as
customization points and should be marked with the ``custom`` keyword to enable
link-time override capabilities.

The check only applies to:

- Free functions (not member functions)
- Functions with external linkage
- Non-inline functions
- Functions that match customization point naming patterns

Examples
--------

.. code-block:: c++

  // Triggers the warning
  void allocate(size_t size) {
    // Implementation
  }

  // Suggested fix:
  custom void allocate(size_t size) {
    // Implementation
  }

Common naming patterns detected:

- Common allocation/resource functions: ``allocate``, ``deallocate``,
  ``construct``, ``destroy``, ``initialize``, ``finalize``
- Functions ending in: ``_hook``, ``_impl``, ``_override``, ``_custom``,
  ``_policy``, ``_customization_point``
- Functions starting with: ``custom_``, ``hook_``, ``override_``

Configuration
-------------

This check requires the ``-fcustomizable-functions`` compiler flag to be enabled
and C++20 or later.

To use:

.. code-block:: bash

  clang-tidy -checks='customizable-suggest-custom' \
             --extra-arg=-fcustomizable-functions \
             --extra-arg=-std=c++20 \
             file.cpp

See Also
--------

- :doc:`../readability-identifier-naming` for general naming conventions
- Clang documentation on customizable functions
