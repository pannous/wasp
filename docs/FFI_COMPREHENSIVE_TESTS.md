## FFI Comprehensive Tests: Five Additional Parameter Type Combinations

### Overview

This document describes five comprehensive FFI test cases demonstrating diverse parameter type combinations. Each test showcases how the Wasp `Signature` class captures native library function signatures in a universal way, and how the `FFIMarshaller` automatically detects the appropriate wrapper for type conversion.

---

## Test Case 1: Integer Absolute Value

### Library Function
```c
int abs(int value);  // stdlib.h from libc
```

### Wasp Signature Representation
```cpp
Signature abs_sig;
abs_sig.add(int32t, "value");
abs_sig.return_types.add(int32t);
```

### Signature Pattern
- **Parameters**: `int32`
- **Return Type**: `int32`
- **Wrapper**: `ffi_i32_i32`
- **Library**: `libc` (imported as "c")

### Wasp Test Code
```wasp
import abs from "c"
abs(-42)  // Expected: 42
```

### Type Conversion Flow
```
Wasp int32 -> FFIMarshaller::to_c_int32() -> C int
C int -> FFIMarshaller::from_c_int32() -> Wasp int32
```

---

## Test Case 2: Floor Function

### Library Function
```c
double floor(double value);  // math.h from libm
```

### Wasp Signature Representation
```cpp
Signature floor_sig;
floor_sig.add(float64t, "value");
floor_sig.return_types.add(float64t);
```

### Signature Pattern
- **Parameters**: `float64`
- **Return Type**: `float64`
- **Wrapper**: `ffi_f64_f64` (already implemented)
- **Library**: `libm` (imported as "m")

### Wasp Test Code
```wasp
import floor from "m"
floor(3.7)  // Expected: 3.0
```

### Type Conversion Flow
```
Wasp float64 -> FFIMarshaller::to_c_double() -> C double
C double -> FFIMarshaller::from_c_double() -> Wasp float64
```

---

## Test Case 3: String Length

### Library Function
```c
size_t strlen(const char* str);  // string.h from libc
```

### Wasp Signature Representation
```cpp
Signature strlen_sig;
strlen_sig.add(charp, "str");
strlen_sig.return_types.add(int32t);  // size_t maps to int32 in wasm
```

### Signature Pattern
- **Parameters**: `char*` (string)
- **Return Type**: `int32` (size_t in wasm)
- **Wrapper**: `ffi_str_i32`
- **Library**: `libc` (imported as "c")

### Wasp Test Code
```wasp
import strlen from "c"
strlen("hello")  // Expected: 5
```

### Type Conversion Flow
```
Wasp string offset (i32) -> FFIMarshaller::offset_to_c_string() -> C char*
C size_t -> FFIMarshaller::from_c_int32() -> Wasp int32
```

---

## Test Case 4: String to Float Conversion

### Library Function
```c
double atof(const char* str);  // stdlib.h from libc
```

### Wasp Signature Representation
```cpp
Signature atof_sig;
atof_sig.add(charp, "str");
atof_sig.return_types.add(float64t);
```

### Signature Pattern
- **Parameters**: `char*` (string)
- **Return Type**: `float64`
- **Wrapper**: `ffi_str_f64`
- **Library**: `libc` (imported as "c")

### Wasp Test Code
```wasp
import atof from "c"
atof("3.14159")  // Expected: 3.14159
```

### Type Conversion Flow
```
Wasp string offset (i32) -> FFIMarshaller::offset_to_c_string() -> C char*
C double -> FFIMarshaller::from_c_double() -> Wasp float64
```

---

## Test Case 5: Minimum of Two Doubles

### Library Function
```c
double fmin(double x, double y);  // math.h from libm
```

### Wasp Signature Representation
```cpp
Signature fmin_sig;
fmin_sig.add(float64t, "x");
fmin_sig.add(float64t, "y");
fmin_sig.return_types.add(float64t);
```

### Signature Pattern
- **Parameters**: `float64, float64`
- **Return Type**: `float64`
- **Wrapper**: `ffi_f64_f64_f64` (already implemented)
- **Library**: `libm` (imported as "m")

### Wasp Test Code
```wasp
import fmin from "m"
fmin(3.5, 2.1)  // Expected: 2.1
```

### Type Conversion Flow
```
Wasp float64 -> FFIMarshaller::to_c_double() -> C double (arg1)
Wasp float64 -> FFIMarshaller::to_c_double() -> C double (arg2)
C double -> FFIMarshaller::from_c_double() -> Wasp float64
```

---

## Signature Detection System

### How It Works

The `FFIMarshaller` class provides automatic signature detection:

```cpp
// 1. Detect parameter types from Wasp Signature
List<FFIMarshaller::CType> param_types(sig.parameters.size());
for (int i = 0; i < sig.parameters.size(); i++) {
    Type wasp_type = sig.parameters[i].type;
    FFIMarshaller::CType c_type = FFIMarshaller::detect_c_type(wasp_type);
    param_types.add(c_type);
}

// 2. Detect return type
Type return_wasp_type = sig.return_types.last(none);
FFIMarshaller::CType return_c_type = FFIMarshaller::detect_c_type(return_wasp_type);

// 3. Automatically select appropriate wrapper
String wrapper_name = FFIMarshaller::detect_wrapper_name(param_types, return_c_type);
```

### Type Mapping

| Wasp Type | FFI CType | C Type | Wrapper Component |
|-----------|-----------|---------|-------------------|
| `int32t` | `CType::Int32` | `int32_t` | `i32` |
| `i64` | `CType::Int64` | `int64_t` | `i64` |
| `float32t` | `CType::Float32` | `float` | `f32` |
| `float64t` | `CType::Float64` | `double` | `f64` |
| `charp` | `CType::String` | `char*` | `str` |
| `strings` | `CType::String` | `char*` | `str` |

### Wrapper Name Patterns

The system generates wrapper names following this pattern:
```
ffi_{param1}_{param2}_{...}_{return}
```

Examples:
- `ffi_i32_i32`: `int32 (int32)` - one int parameter, int return
- `ffi_f64_f64`: `float64 (float64)` - one double parameter, double return
- `ffi_str_i32`: `int32 (char*)` - string parameter, int return
- `ffi_str_f64`: `float64 (char*)` - string parameter, double return
- `ffi_f64_f64_f64`: `float64 (float64, float64)` - two doubles, double return

---

## Complete Test Suite

```wasp
// Test 1: abs(-42) = 42
import abs from "c"
abs(-42)

// Test 2: floor(3.7) = 3.0
import floor from "m"
floor(3.7)

// Test 3: strlen("hello") = 5
import strlen from "c"
strlen("hello")

// Test 4: atof("3.14159") = 3.14159
import atof from "c"
atof("3.14159")

// Test 5: fmin(3.5, 2.1) = 2.1
import fmin from "m"
fmin(3.5, 2.1)

// Combined: sqrt(abs(-16)) = 4.0
import sqrt from "m"
sqrt(abs(-16))
```

---

## Type Coverage Matrix

| Parameter Types | Return Type | Wrapper Name | Example Function |
|----------------|-------------|--------------|------------------|
| `int32` | `int32` | `ffi_i32_i32` | `abs` |
| `float64` | `float64` | `ffi_f64_f64` | `floor`, `sqrt` |
| `char*` | `int32` | `ffi_str_i32` | `strlen` |
| `char*` | `float64` | `ffi_str_f64` | `atof` |
| `float64, float64` | `float64` | `ffi_f64_f64_f64` | `fmin`, `pow` |
| `int32, int32` | `int32` | `ffi_i32_i32_i32` | (future: max, min) |
| `char*, char*` | `int32` | `ffi_str_str_i32` | (future: strcmp) |
| `char*` | `char*` | `ffi_str_str` | (future: strdup) |

---

## Implementation Files

### Core FFI Components

1. **`source/ffi_loader.h`**
   - Dynamic library loading (dlopen/dlsym)
   - Platform-specific library name mapping
   - Function pointer retrieval

2. **`source/ffi_marshaller.h`**
   - Type conversion utilities
   - Signature pattern detection
   - Automatic wrapper name generation
   - Memory offset handling for strings

3. **`source/Context.h/cpp`**
   - FFI function registry (`List<FFIFunctionInfo>`)
   - Bridge between compilation and runtime

4. **`source/Angle.cpp`**
   - Parser support for `import func from "library"` syntax
   - Function signature capture
   - FFI function registration

### Runtime Integration

5. **`source/wasm_runner_edge.cpp`**
   - WasmEdge FFI wrappers
   - Signature-based wrapper selection
   - Type marshalling for WasmEdge runtime

6. **`source/wasmtime_runner.cpp`**
   - Wasmtime FFI wrappers
   - Signature-based wrapper selection
   - Type marshalling for Wasmtime runtime

### Test Files

7. **`test/test_dynlib_import.h`**
   - Basic FFI smoke tests
   - Initial sqrt/pow examples

8. **`test/test_ffi_comprehensive.h`**
   - Five comprehensive parameter type tests
   - Signature detection validation
   - Combined FFI function tests

9. **`samples/test_ffi_comprehensive.wasp`**
   - End-user example code
   - Complete test suite with expected results

---

## Signature Class Usage Examples

### Example 1: Capturing abs() Signature
```cpp
// Function: int abs(int value)
Signature sig;
sig.add(int32t, "value");       // Add parameter with type and name
sig.return_types.add(int32t);   // Add return type

// Signature now represents: int32 abs(int32 value)
```

### Example 2: Capturing fmin() Signature
```cpp
// Function: double fmin(double x, double y)
Signature sig;
sig.add(float64t, "x");          // First parameter
sig.add(float64t, "y");          // Second parameter
sig.return_types.add(float64t);  // Return type

// Signature now represents: float64 fmin(float64 x, float64 y)
```

### Example 3: Capturing strlen() Signature
```cpp
// Function: size_t strlen(const char* str)
Signature sig;
sig.add(charp, "str");          // String parameter
sig.return_types.add(int32t);   // size_t -> int32 in wasm

// Signature now represents: int32 strlen(char* str)
```

### Example 4: Universal Signature Detection
```cpp
// Generic function to detect wrapper for any signature
String detect_ffi_wrapper(Signature& sig) {
    // Convert Wasp types to FFI types
    List<FFIMarshaller::CType> param_types(sig.parameters.size());
    for (int i = 0; i < sig.parameters.size(); i++) {
        Type wasp_type = sig.parameters[i].type;
        param_types.add(FFIMarshaller::detect_c_type(wasp_type));
    }

    // Get return type
    Type ret_type = sig.return_types.last(none);
    FFIMarshaller::CType ret_c_type = FFIMarshaller::detect_c_type(ret_type);

    // Automatic wrapper detection
    return FFIMarshaller::detect_wrapper_name(param_types, ret_c_type);
}
```

---

## Future Extensibility

The signature detection system is designed to be easily extensible:

### Adding New Type Combinations

1. **Add detection in `ffi_marshaller.h`:**
```cpp
// In detect_wrapper_name()
if (param_count == 3) {
    if (p1 == CType::Int32 && p2 == CType::Int32 && p3 == CType::Int32 &&
        return_type == CType::Int32)
        return "ffi_i32_i32_i32_i32";
}
```

2. **Implement wrapper in runtimes:**
```cpp
// In wasm_runner_edge.cpp and wasmtime_runner.cpp
WasmEdge_Result ffi_wrapper_i32_i32_i32_i32(...) {
    // Three int32 parameters, one int32 return
}
```

3. **Test the new pattern:**
```wasp
import my_func from "mylib"
my_func(1, 2, 3)  // Automatically uses ffi_i32_i32_i32_i32
```

---

## Conclusion

These five test cases demonstrate comprehensive FFI support with:
- **Universal signature representation** using Wasp's Signature class
- **Automatic type detection** and wrapper selection
- **Clean separation** between library loading, type marshalling, and runtime integration
- **Extensible design** for future type combinations

The signature detection system enables seamless integration of native library functions with minimal manual configuration, while maintaining type safety across the FFI boundary.
