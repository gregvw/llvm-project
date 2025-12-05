# Ambiguous Override Bug: Scope and Overload Disambiguation Missing

## Critical Design Flaw

The current implementation **ignores scope and overload disambiguation**. The attribute stores only the unqualified identifier, causing all `custom` functions with the same name—across namespaces, overloads, and translation units—to target the **same override symbol**.

## The Problem

### CodeGen Stores Only Unqualified Name

**File:** `clang/lib/CodeGen/CodeGenModule.cpp:6550`
```cpp
// Add attribute marking this as customizable
PublicFn->addFnAttr("clang-customizable-function", D->getName());
```

`D->getName()` returns **only the unqualified identifier** (e.g., `"compute"`), ignoring:
- Namespace (`ns1::compute` vs `ns2::compute`)
- Overloads (`compute(int)` vs `compute(double)`)
- Translation unit boundaries

### LTO Pass Uses Unqualified Name

**File:** `llvm/lib/Transforms/IPO/CustomizableFunctions.cpp:48-54`
```cpp
auto Attr = Wrapper.getFnAttribute("clang-customizable-function");
StringRef LogicalName = Attr.getValueAsString();

// Form the override function name.
std::string OverrideName = ("__custom_override_" + LogicalName).str();
Function *Override = M.getFunction(OverrideName);
```

The pass looks for **ONE** override symbol `__custom_override_<LogicalName>` for **ALL** wrappers with that logical name, regardless of namespace or signature.

## Bug Demonstration

### Scenario 1: Namespace Collision

```cpp
namespace lib1 {
  custom int process(int x) {
    return x * 2;  // Library default
  }
}

namespace lib2 {
  custom int process(int x) {
    return x * 3;  // Different library, different default
  }
}

// User provides ONE override - which process() does it override?
int __custom_override_process(int x) {
  return x * 4;
}

int main() {
  int a = lib1::process(10);  // Gets 40 (overridden)
  int b = lib2::process(10);  // Also gets 40 (WRONGLY overridden!)
  // Both call the SAME override, even though they're different functions!
}
```

**Generated IR:**
```llvm
; Both wrappers have the SAME attribute:
define i32 @_ZN4lib17processEi(i32 %x) #3 {  ; lib1::process
  ; ...
}
attributes #3 = { "clang-customizable-function"="process" ... }

define i32 @_ZN4lib27processEi(i32 %x) #3 {  ; lib2::process
  ; ...
}
attributes #3 = { "clang-customizable-function"="process" ... }

; ONE override symbol:
define i32 @__custom_override_process(i32 %x) {
  ; ...
}
```

**LTO Pass Behavior:**
1. Sees wrapper `lib1::process` with attribute `"process"`
2. Looks for `__custom_override_process` → Found!
3. Signature matches → **Rewrites lib1::process**
4. Sees wrapper `lib2::process` with attribute `"process"`
5. Looks for `__custom_override_process` → Found! (same symbol)
6. Signature matches → **Rewrites lib2::process**

**Result:** Both namespaces are overridden by the same function, even though the user intended to customize only one.

### Scenario 2: Overload Collision

```cpp
custom void handle(int x) {
  printf("Handling int: %d\n", x);
}

custom void handle(double x) {
  printf("Handling double: %f\n", x);
}

// User wants to override only the int version:
void __custom_override_handle(int x) {
  printf("Custom int handler: %d\n", x);
}

int main() {
  handle(42);      // Correctly overridden
  handle(3.14);    // NOT overridden (signature mismatch)
  // But if user provides:
  // void __custom_override_handle(double x);
  // Then ONLY the double version is overridden, not the int version!
}
```

The user **cannot** provide overrides for both overloads simultaneously, because they would both target `__custom_override_handle`.

### Scenario 3: Cross-TU Collision

**File: lib.cpp**
```cpp
namespace mylib {
  custom int compute(int x) {
    return x * 2;
  }
}
```

**File: user.cpp**
```cpp
namespace usercode {
  custom int compute(int x) {
    return x + 100;
  }
}

// User provides override - which compute() does it override?
int __custom_override_compute(int x) {
  return x * 5;
}
```

**Result:** Both `mylib::compute` and `usercode::compute` get overridden, even though they're completely unrelated functions in different namespaces and translation units.

## Why Signature Matching Isn't Enough

The LTO pass **does** check signature matching (line 61):
```cpp
if (Override->getFunctionType() != Wrapper.getFunctionType()) {
  // Skip this wrapper
  return false;
}
```

But this **only prevents type mismatches**, not namespace/scope collisions:

```cpp
namespace lib1 {
  custom int foo(int x);  // Wrapper type: int(int)
}
namespace lib2 {
  custom int foo(int x);  // Wrapper type: int(int) - SAME TYPE!
}

int __custom_override_foo(int x);  // Type: int(int) - MATCHES BOTH!
```

Both wrappers have identical signatures, so **both pass the signature check** and get overridden.

## Real-World Impact

### Problem 1: Silent Miscompilation

```cpp
namespace database {
  custom void close(Connection* conn);  // Close database connection
}

namespace network {
  custom void close(Connection* conn);  // Close network socket
}

// User overrides database::close
void __custom_override_close(Connection* conn) {
  // Database-specific cleanup
}
```

**Result:** Network connections also get "database cleanup", likely causing crashes or resource leaks. No diagnostic, no warning.

### Problem 2: Cannot Override Specific Overloads

```cpp
custom void log(const char* msg);      // Want to override this
custom void log(const std::string& s); // Don't want to override this

void __custom_override_log(const char* msg) {
  // Custom logging for C strings
}
```

**Result:** Only the `const char*` overload is overridden (signature matches). User **cannot** provide both overrides because they'd both be named `__custom_override_log`.

### Problem 3: Library Name Conflicts

```cpp
// Boost library
namespace boost { custom void transform(Data& d); }

// Qt library
namespace Qt { custom void transform(Data& d); }

// User wants to customize Boost but not Qt - IMPOSSIBLE!
```

## Root Cause

The override discovery mechanism has **no disambiguation**:

1. **Attribute payload**: Stores only unqualified identifier
2. **Override symbol naming**: Uses only unqualified identifier
3. **No namespace encoding**: No way to distinguish `ns1::foo` from `ns2::foo`
4. **No overload encoding**: No way to distinguish `foo(int)` from `foo(double)`
5. **No scope checking**: LTO pass doesn't verify namespace/scope match

## Possible Solutions

### Option 1: Use Mangled Names (Most Precise)

**Store mangled name in attribute:**
```cpp
PublicFn->addFnAttr("clang-customizable-function", MangledName);
```

**Override symbol uses mangled name:**
```cpp
std::string OverrideName = ("__custom_override_" + MangledName).str();
// e.g., __custom_override__ZN4lib13fooEi
```

**Pros:**
- Completely unambiguous
- Handles namespaces, overloads, TUs automatically
- No false matches possible

**Cons:**
- Users must know/generate mangled names for overrides
- Less human-readable
- Harder to use

### Option 2: Encode Namespace + Signature (Hybrid)

**Store qualified name + signature hash:**
```cpp
std::string EncodedName = QualifiedName + "_" + SignatureHash;
// e.g., "ns1::compute_a3f2b9"
PublicFn->addFnAttr("clang-customizable-function", EncodedName);
```

**Pros:**
- More readable than full mangling
- Distinguishes namespaces and overloads
- Deterministic

**Cons:**
- Users still need to match encoding scheme
- Signature hash is opaque

### Option 3: Diagnostic + Documentation (Conservative)

**Detect and diagnose ambiguous names:**
```cpp
// At CodeGen or LTO time, detect multiple custom functions with same name
if (hasMultipleCustomFunctionsWithName(Name)) {
  Diag(Loc, "multiple custom functions named '%0' may cause ambiguous overrides")
    << Name;
}
```

**Require users to use unique names:**
```
NOTE: When using 'custom', ensure function names are globally unique
across all namespaces and overloads in your program, or provide
namespace-specific override symbols.
```

**Pros:**
- Documents the limitation
- Helps users avoid the pitfall
- Doesn't break anything

**Cons:**
- Doesn't fix the underlying problem
- Users must work around the limitation
- Restricts valid use cases

### Option 4: Extend Override Syntax (Future)

Allow users to specify qualified override targets:
```cpp
// User specifies which function to override
void __custom_override_ns1_compute(int x)
  __attribute__((overrides("ns1::compute")));

void __custom_override_ns2_compute(int x)
  __attribute__((overrides("ns2::compute")));
```

**Pros:**
- Explicit, clear intent
- Allows fine-grained control
- Human-readable

**Cons:**
- Requires language/attribute extension
- More complex implementation
- Backwards compatibility concerns

## Recommendation

For the **experimental** phase, I recommend **Option 1 (mangled names)** because:

1. **Correctness first**: Prevents silent bugs
2. **Simple implementation**: Minimal code changes
3. **Proves the concept**: Demonstrates correct scoping
4. **Can refine later**: Can add sugar/convenience later

Once the core mechanism is proven, consider **Option 4** for usability.

## Current Status

- ❌ **Bug exists** in current implementation
- ❌ **No disambiguation** of namespaces or overloads
- ❌ **Silent miscompilation** possible
- ⚠️  **Documented limitation** needed urgently

## Test Needed

Create test that verifies:
1. Different namespaces with same function name are distinguished
2. Different overloads of same function are distinguished
3. Providing an override for one doesn't affect the other
4. Cross-TU collisions are handled correctly

This is a **critical correctness issue** that must be addressed before the feature can be considered production-ready.
