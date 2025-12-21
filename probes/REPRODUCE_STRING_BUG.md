# String Const Comparison Bug

## The Problem

The raylib loading failure was caused by a missing const-correct operator in the String class.

## Root Cause

In `source/ffi_inspector.h:24`, the function `get_library_path()` takes a `const String&` parameter:

```cpp
inline const char* get_library_path(const String& library_name) {
    if (library_name == "raylib") return "/opt/homebrew/lib/libraylib.dylib";
    //                  ^^^^^^^^^^ THIS COMPARISON FAILS
```

The String class has:
- `bool operator==(char *c) const` - takes NON-const char*
- `bool operator==(chars c)` - NON-const member function

When `library_name` is a `const String&`, the compiler needs:
- `bool operator==(const char *c) const` - takes const char*, is const member

## To Reproduce

### Test 1: Run the raylib sample (will fail)
```bash
cd /Users/me/dev/apps/wasp
./cmake-build-debug/wasp samples/raylib_simple_use.wasp
```

**Expected Error:**
```
Failed to load library: raylib
```

### Test 2: Check the exact code location

Look at `source/ffi_inspector.h` line 24:
```cpp
if (library_name == "raylib") return "/opt/homebrew/lib/libraylib.dylib";
```

With the current String.h, this comparison does not work correctly when `library_name` is `const String&`.

## The Fix Required

Add to String.h around line 994:

```cpp
bool operator==(const char *c) const {
    return eq(data, c, length);
}
```

## BUT WAIT - There's a Second Bug!

After fixing the above, `testMapOfStrings()` fails at line 1065:

```cpp
Map<String, chars> map;
map["a"] = "1";
check(map.values[0] == "1"s);  // FAILS
//    ^^^^^^^^^^^^^^^^^^^^^^^
//    const char* == String
```

This requires the REVERSE comparison: `const char* == String`

In C++, `a == b` checks:
1. `a.operator==(b)` - can't work (const char* is built-in type)
2. `operator==(a, b)` - needs a global/free function

## Attempted Solution That Failed

Adding a global operator:
```cpp
inline bool operator==(const char* lhs, const String& rhs) {
    return rhs == lhs;
}
```

This causes ambiguity errors in:
- String.h lines 904, 913, 917 (char16_t/char32_t/wchar_t operators)
- Code.h:1417
- Map.h:115

## Challenge

Need to support BOTH:
1. `const String& == const char*` (for raylib loading)
2. `const char* == String` (for testMapOfStrings)

Without causing ambiguity in existing operator overloads.

## Files to Check

- `source/String.h` - operator== definitions (lines 988-1004)
- `source/ffi_inspector.h:24` - raylib comparison
- `test/tests.cpp:1065` - testMapOfStrings failure
- `test/test_string.h:40-60` - regression test for const comparison
