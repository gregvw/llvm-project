# Driver Flag Forwarding Fix for -fcustomizable-functions-sema

## Problem

The `-fcustomizable-functions-sema` flag was defined in `Options.td` but not forwarded by the driver to cc1, making it effectively unusable without the awkward `-Xclang` workaround.

## Before Fix

**Command:**
```bash
clang++ -fcustomizable-functions -fcustomizable-functions-sema test.cpp
```

**Result:**
```
warning: argument unused during compilation: '-fcustomizable-functions-sema' [-Wunused-command-line-argument]
```

**Generated IR:**
```llvm
; Calls library wrapper, NOT user override
call i32 @_ZN3lib5cloneIN4user6MyTypeEEET_RKS3_(...)
```

**Workaround required:**
```bash
# Awkward - users shouldn't need to know about -Xclang
clang++ -fcustomizable-functions -Xclang -fcustomizable-functions-sema test.cpp
```

## After Fix

**Command:**
```bash
clang++ -fcustomizable-functions -fcustomizable-functions-sema test.cpp
```

**Result:**
```
(no warnings)
```

**Generated IR:**
```llvm
; Calls user override directly via ADL!
call i32 @_ZN4user5cloneERKNS_6MyTypeE(...)
```

## The Fix

**File:** `clang/lib/Driver/ToolChains/Clang.cpp`

**Change:**
```cpp
Args.addOptInFlag(CmdArgs, options::OPT_fcustomizable_functions,
                  options::OPT_fno_customizable_functions);
// ADD THIS:
Args.addOptInFlag(CmdArgs, options::OPT_fcustomizable_functions_sema,
                  options::OPT_fno_customizable_functions_sema);
```

**Location:** Around line 6090

## Test Case

```cpp
// test_driver_forward.cpp
namespace lib {
  template<typename T>
  custom T clone(const T& x) {
    return T(x);
  }
}

namespace user {
  struct MyType { int value; };

  // This should be found via ADL with -fcustomizable-functions-sema
  MyType clone(const MyType& x) {
    return MyType{x.value * 2};
  }
}

void test() {
  user::MyType obj{42};
  auto copy = lib::clone(obj);  // Should call user::clone
}
```

**Verify:**
```bash
# Should call user::clone directly (no warning)
clang++ -std=c++20 -fcustomizable-functions -fcustomizable-functions-sema \
  -emit-llvm -S test_driver_forward.cpp -o - | grep "call.*clone"
```

**Expected output:**
```
call i32 @_ZN4user5cloneERKNS_6MyTypeE(...)
```

## Impact

### Before
- Users confused why flag doesn't work
- Had to use undocumented `-Xclang` workaround
- Inconsistent with other driver flags
- Bad user experience

### After
- ✅ Flag works as expected
- ✅ Consistent with other `-f` flags
- ✅ No `-Xclang` needed
- ✅ Better user experience
- ✅ Documentation makes sense

## Why This Matters

**For users:**
- Intuitive flag usage
- No need to know about `-Xclang`
- Consistent with expectations

**For standardization:**
- Shows feature is user-friendly
- No weird workarounds needed
- Matches how other experimental features work

**For testing:**
- Examples in documentation work correctly
- Users can actually try the feature
- Real-world feedback possible

## Related Files

- `clang/include/clang/Options/Options.td` - Flag definitions ✓ (already correct)
- `clang/lib/Driver/ToolChains/Clang.cpp` - Driver forwarding ✓ (now fixed)
- `clang/include/clang/Basic/LangOptions.def` - Lang options ✓ (already correct)

## Verification

**Test added:** `clang/test/Driver/customizable-functions-driver.cpp`

**Test results:**
```bash
# Driver correctly forwards flag to cc1
PASS: Clang :: Driver/customizable-functions-driver.cpp

# Full test suite still passes
All customizable functions tests: PASS (21 tests)
```

**Practical verification:**
```bash
# Without -fcustomizable-functions-sema: calls library wrapper
$ clang++ -fcustomizable-functions -emit-llvm -S test.cpp -o - | grep clone
call i32 @_ZN3lib5cloneIN4user6MyTypeEEET_RKS3_(...)

# With -fcustomizable-functions-sema: calls user override directly
$ clang++ -fcustomizable-functions -fcustomizable-functions-sema -emit-llvm -S test.cpp -o - | grep clone
call i32 @_ZN4user5cloneERKNS_6MyTypeE(...)

# No warnings!
$ clang++ -fcustomizable-functions -fcustomizable-functions-sema test.cpp -c 2>&1
(clean compile, no warnings)
```

## Summary

**What was wrong:** Driver didn't forward `-fcustomizable-functions-sema` to cc1

**What we changed:** Added 2 lines to forward the flag (same pattern as base flag)

**Result:** Flag now works correctly without `-Xclang` workaround

This is a **simple but important fix** for usability.
