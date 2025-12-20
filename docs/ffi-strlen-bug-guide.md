# FFI strlen Bug Guide - CRITICAL

**Status:** This bug has been fixed and broken at least 3 times. Read this before modifying FFI code.

## TL;DR - The Bug

`import strlen from "c"` crashes with SIGSEGV because function name and library name get swapped when re-detecting signatures from WASM binaries.

## The Critical Line

**File:** `source/wasm_reader.cpp:238-239`

```cpp
// CORRECT (DO NOT CHANGE):
detect_ffi_signature(name1, mod, func.signature);

// WRONG (causes crash):
detect_ffi_signature(mod, name1, func.signature);
```

**Function signature:** `detect_ffi_signature(func_name, lib_name, sig)`

**Import context:**
- `mod` = library name (e.g., "c", "m", "raylib")
- `name1` = function name (e.g., "strlen", "ceil", "floor")

## Why This Keeps Breaking

1. **Non-obvious parameter order** - It's easy to assume (library, function) when it's actually (function, library)
2. **Compiles fine** - Both are String types, so compiler doesn't catch the swap
3. **Breaks most FFI** - Affects all C library imports, not just strlen
4. **Similar variable names** - `mod` and `name1` don't clearly indicate which is which

## What Happens When Broken

### Symptom
```
$ ./wasp test
import strlen from "c"
strlen("hello")
Process finished with exit code 139 (interrupted by signal 11:SIGSEGV)
```

### Root Cause Chain

1. **WASM binary loses type info**: `charp` parameter → `i32` in WASM
2. **Need to restore types**: Call `detect_ffi_signature()` when reading WASM
3. **If arguments swapped**: strlen detected as func="c", lib="strlen"
4. **Wrong signature lookup**: Tries to find function "c" in library "strlen"
5. **Falls through to default**: Gets `float64 -> float64` instead of `charp -> int32`
6. **Type mismatch at runtime**: Wasmtime expects `(func (param i32) (result i32))` but gets `(func (param f64) (result f64))`
7. **CRASH**: Segmentation fault when trying to call with wrong types

### Debug Output When Broken
```
DEBUG detect_ffi_signature: func_name='c' (len=1), lib_name='strlen' (len=6)
DEBUG: Entering lib='m' branch   # WRONG! Should enter lib='c' branch
DEBUG add_param: added type 0x7c (real)   # Should be 0xc0 (chars)
FFI wrapper: param wasp_type = 0x7c, CType = 4  # Should be 0xc0, CType = 5
```

### Debug Output When Fixed
```
DEBUG detect_ffi_signature: func_name='strlen' (len=6), lib_name='c' (len=1)
DEBUG: Entering lib='c' branch   # CORRECT!
DEBUG: Matched strlen/atoi/atof/atol - adding charp parameter
DEBUG add_param: added type 0xc0 (chars)   # CORRECT!
FFI wrapper: param wasp_type = 0xc0, CType = 5  # CORRECT!
```

## Type Value Reference

Critical for debugging:
- `0xC0` = charp (char pointer, string parameter) - **CORRECT for strlen**
- `0x7F` = int32t
- `0x7C` = float64t (real) - **WRONG for strlen, indicates bug**
- CType 5 = String - **CORRECT**
- CType 4 = Float64 - **WRONG**

## Complete Fix History

### Bug Instance 1 (Original)
- **Commit:** Unknown
- **Cause:** Initial implementation had swapped arguments
- **Fixed:** Swapped arguments to correct order

### Bug Instance 2 (Regression)
- **Commit:** Between 8d36a3e8 (271 tests passing) and HEAD
- **Cause:** Code refactoring re-introduced swapped arguments
- **Fixed:** Commit c5e7b0de
- **Additional fixes:**
  - String corruption bug (using references instead of copies)
  - Added signature re-detection when reading WASM

### Bug Instance 3 (This time)
- **Commit:** c5e7b0de
- **Cause:** Same parameter swap issue
- **Fixed:** Current commit
- **Note:** Also removed extensive debug tracing added during investigation

## Related Bugs Fixed Simultaneously

### 1. String Corruption in wasm_reader.cpp

**Lines 215-216:**
```cpp
// WRONG - references to same buffer, second call corrupts first:
String &mod = name(payload);
String &name1 = name(payload);

// CORRECT - copies prevent corruption:
String mod = name(payload);    // MUST be copy, not reference!
String name1 = name(payload);  // MUST be copy, not reference!
```

**Symptom:** `ffi_library='cstrlen'` instead of `'c'`

### 2. Missing Signature Re-detection

**Why needed:** WASM binary format loses semantic type information. All pointers become `i32`, so `charp` becomes indistinguishable from regular `int32`. We must restore the semantic type by re-detecting the signature.

```cpp
// Check if this is an FFI import from a native library module
if (mod != "env" && mod != "wasi_snapshot_preview1") {
    func.is_ffi = true;
    func.ffi_library = mod;

    // Re-detect signature from library/function name
    // (WASM loses type info like charp -> i32, so we need to restore it)
    detect_ffi_signature(name1, mod, func.signature);  // CRITICAL: correct order!
}
```

## How to Test the Fix

### Quick Test
```bash
cat > /tmp/test_strlen.wasp << 'EOF'
import strlen from "c"
strlen("hello")
EOF

./wasp /tmp/test_strlen.wasp
# Expected output: » 5
# Wrong output: Exit code 139 (SIGSEGV)
```

### Test Suite
```bash
./wasp test
# Should show: 271+ CURRENT TESTS PASSED
```

### Test Other C Library Functions
```bash
# atoi - also takes charp parameter
echo 'import atoi from "c"\natoi("42")' > /tmp/test_atoi.wasp
./wasp /tmp/test_atoi.wasp  # Expected: 42

# strcmp - takes two charp parameters
echo 'import strcmp from "c"\nstrcmp("hello", "hello")' > /tmp/test_strcmp.wasp
./wasp /tmp/test_strcmp.wasp  # Expected: 0
```

## How to Prevent Future Breaks

### 1. Code Review Checklist
- [ ] Check `wasm_reader.cpp:238` - arguments to `detect_ffi_signature` in correct order?
- [ ] Verify `name1, mod` not `mod, name1`
- [ ] Test strlen before committing FFI changes

### 2. Automated Test
The test suite includes strlen tests at `test/test_ffi.h:63-65`:
```cpp
assert_emit("import strlen from \"c\"\nstrlen(\"hello\")", 5);
assert_emit("import strlen from \"c\"\nstrlen(\"\")", 0);
assert_emit("import strlen from \"c\"\nstrlen(\"Wasp\")", 4);
```

Always run full test suite after FFI modifications.

### 3. Better Variable Names (Future Improvement)

Consider renaming in `wasm_reader.cpp`:
```cpp
// Instead of:
String mod = name(payload);
String name1 = name(payload);

// Use more explicit names:
String library_name = name(payload);
String function_name = name(payload);

// Then call becomes obvious:
detect_ffi_signature(function_name, library_name, func.signature);
```

### 4. Add Static Assert or Comment

Add a loud warning comment:
```cpp
// ⚠️  CRITICAL: Arguments MUST be (function_name, library_name) NOT (library, function)!
// ⚠️  This has broken 3+ times. Test with strlen before committing!
// ⚠️  See docs/ffi-strlen-bug-guide.md for details
detect_ffi_signature(name1, mod, func.signature);
```

## Key Files to Review

When debugging FFI issues, check these files in order:

1. **source/wasm_reader.cpp** (lines 230-240)
   - Import parsing
   - Signature re-detection call
   - **Most common source of this bug**

2. **source/ffi_signatures.h** (lines 164+)
   - Signature detection logic
   - Hardcoded signatures by library
   - lib='c' branch for strlen (lines 202-208)

3. **source/ffi_dynamic_wrapper.h** (lines 127+)
   - Runtime wrapper for FFI calls
   - Parameter marshalling
   - Type conversion

4. **source/wasmtime_runner.cpp** (lines 300+)
   - FFI function linking
   - Wasmtime integration
   - Runtime library loading

5. **source/ffi_marshaller.h** (lines 44-52)
   - Type mapping (Wasp types → C types)
   - Critical: `charp → CType::String`

## Affected Functions

When this bug occurs, these C library functions are also affected (same root cause):
- `strlen`, `atoi`, `atof`, `atol` (single charp parameter)
- `strcmp`, `strcoll` (two charp parameters)
- Any function from libc that takes string parameters

Math library functions (`ceil`, `floor`, `sin`, `cos`, etc.) from libm are also affected but fail differently (get wrong signature for their parameter types).

## References

- Last working commit (271 tests): `8d36a3e8`
- Latest fix commit: `c5e7b0de`
- Related files: See "Key Files to Review" above
- Test location: `test/test_ffi.h:60-65` (`test_ffi_strlen()`)

## Contact

If this bug recurs again, consult this document and check:
1. `wasm_reader.cpp:238` - argument order
2. Run strlen test - should return 5
3. Check debug output for type values (0xC0 vs 0x7C)

---

**Last Updated:** 2024-12-20
**Last Fix:** Commit c5e7b0de
**Times Fixed:** 3+
**Severity:** CRITICAL - Breaks all C library string function imports
