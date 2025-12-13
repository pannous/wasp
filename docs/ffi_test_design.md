# FFI Test Design: Five Additional Parameter Type Tests

## Overview
This document outlines five additional FFI test cases covering diverse parameter type combinations, demonstrating the extensibility of the FFI marshaller system.

## Test Cases with Signatures

### 1. Integer Absolute Value: `abs(int) -> int`
**Library**: `libc` (stdlib.h)
**Function**: `abs`
**Wasp Signature**:
```cpp
Signature abs_sig;
abs_sig.add(int32t, "value");
abs_sig.return_types.add(int32t);
```
**Marshaller Wrapper**: `ffi_i32_i32` (int32 -> int32)
**Test Case**:
```wasp
import abs from "c"
abs(-42)  // Expected: 42
```

---

### 2. Floor Function: `floor(double) -> double`
**Library**: `libm` (math.h)
**Function**: `floor`
**Wasp Signature**:
```cpp
Signature floor_sig;
floor_sig.add(float64t, "value");
floor_sig.return_types.add(float64t);
```
**Marshaller Wrapper**: `ffi_f64_f64` (already exists)
**Test Case**:
```wasp
import floor from "m"
floor(3.7)  // Expected: 3.0
```

---

### 3. String Length: `strlen(char*) -> size_t`
**Library**: `libc` (string.h)
**Function**: `strlen`
**Wasp Signature**:
```cpp
Signature strlen_sig;
strlen_sig.add(charp, "str");
strlen_sig.return_types.add(int32t);  // size_t maps to i32 in wasm
```
**Marshaller Wrapper**: `ffi_str_i32` (char* -> int32)
**Test Case**:
```wasp
import strlen from "c"
strlen("hello")  // Expected: 5
```

---

### 4. String to Float: `atof(char*) -> double`
**Library**: `libc` (stdlib.h)
**Function**: `atof`
**Wasp Signature**:
```cpp
Signature atof_sig;
atof_sig.add(charp, "str");
atof_sig.return_types.add(float64t);
```
**Marshaller Wrapper**: `ffi_str_f64` (char* -> double)
**Test Case**:
```wasp
import atof from "c"
atof("3.14159")  // Expected: 3.14159
```

---

### 5. Minimum of Two Doubles: `fmin(double, double) -> double`
**Library**: `libm` (math.h)
**Function**: `fmin`
**Wasp Signature**:
```cpp
Signature fmin_sig;
fmin_sig.add(float64t, "x");
fmin_sig.add(float64t, "y");
fmin_sig.return_types.add(float64t);
```
**Marshaller Wrapper**: `ffi_f64_f64_f64` (already exists as `ffi_wrapper_f64_f64_f64`)
**Test Case**:
```wasp
import fmin from "m"
fmin(3.5, 2.1)  // Expected: 2.1
```

---

## Signature Type Summary

The test cases demonstrate these parameter type patterns:

1. **int32 → int32**: Integer transformations (abs)
2. **float64 → float64**: Single-parameter math (floor)
3. **char* → int32**: String analysis (strlen)
4. **char* → float64**: String parsing (atof)
5. **float64, float64 → float64**: Two-parameter math (fmin)

## Required Marshaller Wrappers

### New Wrappers Needed:

1. **ffi_i32_i32** (int32 -> int32)
   ```cpp
   // WasmEdge
   WasmEdge_Result ffi_wrapper_i32_i32(void *func_ptr, ...) {
       typedef int32_t (*ffi_func_t)(int32_t);
       ffi_func_t func = (ffi_func_t)func_ptr;
       int32_t arg = WasmEdge_ValueGetI32(In[0]);
       int32_t c_arg = FFIMarshaller::to_c_int32(arg);
       int32_t c_result = func(c_arg);
       int32_t result = FFIMarshaller::from_c_int32(c_result);
       Out[0] = WasmEdge_ValueGenI32(result);
       return WasmEdge_Result_Success;
   }

   // Wasmtime
   wrap(ffi_i32_i32) {
       typedef int32_t (*ffi_func_t)(int32_t);
       ffi_func_t func = (ffi_func_t)env;
       int32_t arg = args[0].of.i32;
       int32_t c_arg = FFIMarshaller::to_c_int32(arg);
       int32_t c_result = func(c_arg);
       int32_t result = FFIMarshaller::from_c_int32(c_result);
       results[0].of.i32 = result;
       return NULL;
   }
   ```

2. **ffi_str_i32** (char* -> int32)
   ```cpp
   // WasmEdge
   WasmEdge_Result ffi_wrapper_str_i32(void *func_ptr, ...) {
       typedef int32_t (*ffi_func_t)(const char*);
       ffi_func_t func = (ffi_func_t)func_ptr;
       int32_t offset = WasmEdge_ValueGetI32(In[0]);
       const char* c_str = FFIMarshaller::offset_to_c_string(wasm_memory, offset);
       int32_t c_result = func(c_str);
       int32_t result = FFIMarshaller::from_c_int32(c_result);
       Out[0] = WasmEdge_ValueGenI32(result);
       return WasmEdge_Result_Success;
   }

   // Wasmtime
   wrap(ffi_str_i32) {
       typedef int32_t (*ffi_func_t)(const char*);
       ffi_func_t func = (ffi_func_t)env;
       int32_t offset = args[0].of.i32;
       const char* c_str = FFIMarshaller::offset_to_c_string(wasm_memory, offset);
       int32_t c_result = func(c_str);
       int32_t result = FFIMarshaller::from_c_int32(c_result);
       results[0].of.i32 = result;
       return NULL;
   }
   ```

3. **ffi_str_f64** (char* -> double)
   ```cpp
   // WasmEdge
   WasmEdge_Result ffi_wrapper_str_f64(void *func_ptr, ...) {
       typedef double (*ffi_func_t)(const char*);
       ffi_func_t func = (ffi_func_t)func_ptr;
       int32_t offset = WasmEdge_ValueGetI32(In[0]);
       const char* c_str = FFIMarshaller::offset_to_c_string(wasm_memory, offset);
       double c_result = func(c_str);
       double result = FFIMarshaller::from_c_double(c_result);
       Out[0] = WasmEdge_ValueGenF64(result);
       return WasmEdge_Result_Success;
   }

   // Wasmtime
   wrap(ffi_str_f64) {
       typedef double (*ffi_func_t)(const char*);
       ffi_func_t func = (ffi_func_t)env;
       int32_t offset = args[0].of.i32;
       const char* c_str = FFIMarshaller::offset_to_c_string(wasm_memory, offset);
       double c_result = func(c_str);
       double result = FFIMarshaller::from_c_double(c_result);
       results[0].of.f64 = result;
       return NULL;
   }
   ```

## Signature Detection System

To automatically detect and match the appropriate wrapper, extend `FFIMarshaller::detect_signature()`:

```cpp
struct SignaturePattern {
    List<CType> params;
    CType return_type;
    String wrapper_name;
};

static SignaturePattern detect_signature(Signature& sig) {
    SignaturePattern pattern;

    // Detect parameter types
    for (int i = 0; i < sig.parameters.size(); i++) {
        pattern.params.add(detect_c_type(sig.parameters[i].type));
    }

    // Detect return type
    Type ret_type = sig.return_types.last(none);
    pattern.return_type = detect_c_type(ret_type);

    // Match to wrapper name
    if (pattern.params.size() == 1 &&
        pattern.params[0] == CType::Int32 &&
        pattern.return_type == CType::Int32) {
        pattern.wrapper_name = "ffi_i32_i32";
    }
    else if (pattern.params.size() == 1 &&
             pattern.params[0] == CType::String &&
             pattern.return_type == CType::Int32) {
        pattern.wrapper_name = "ffi_str_i32";
    }
    else if (pattern.params.size() == 1 &&
             pattern.params[0] == CType::String &&
             pattern.return_type == CType::Float64) {
        pattern.wrapper_name = "ffi_str_f64";
    }
    // ... more patterns

    return pattern;
}
```

## Library Name Mapping

Common library name shortcuts:
- `"c"` → `libc.dylib` / `libc.so.6`
- `"m"` → `libm.dylib` / `libm.so.6`
- `"pthread"` → `libpthread.dylib` / `libpthread.so.0`

## Complete Test File

```wasp
// Test 1: Integer absolute value
import abs from "c"
abs(-42)  // 42

// Test 2: Floor function
import floor from "m"
floor(3.7)  // 3.0

// Test 3: String length
import strlen from "c"
strlen("hello")  // 5

// Test 4: String to float
import atof from "c"
atof("3.14159")  // 3.14159

// Test 5: Minimum of two doubles
import fmin from "m"
fmin(3.5, 2.1)  // 2.1

// Combined test
import sqrt from "m"
sqrt(abs(-16))  // 4.0
```

## Implementation Priority

1. **Phase 1**: Add basic integer and string return wrappers (i32_i32, str_i32, str_f64)
2. **Phase 2**: Implement signature detection and automatic wrapper selection
3. **Phase 3**: Add comprehensive test suite with all five test cases
4. **Phase 4**: Extend to handle more complex types (structs, arrays, callbacks)

## Type Coverage Matrix

| From/To   | int32 | float64 | char* | void |
|-----------|-------|---------|-------|------|
| **int32** | ✓ abs | -       | -     | -    |
| **float64** | -   | ✓ floor, fmin | - | -  |
| **char*** | strlen | atof  | -     | -    |
| **void**  | -     | -       | -     | -    |

This demonstrates comprehensive type conversion coverage across the FFI boundary.
