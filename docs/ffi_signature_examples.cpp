// FFI Signature Examples
// Demonstrates how to use Wasp's Signature class to capture
// native library function signatures in a universal way

#include "Code.h"
#include "NodeTypes.h"
#include "ffi_marshaller.h"

// ==============================================================================
// Example 1: abs(int) -> int
// ==============================================================================
void example_abs_signature() {
    // Function: int abs(int value)
    // Library: libc (stdlib.h)

    Signature abs_sig;
    abs_sig.add(int32t, "value");
    abs_sig.return_types.add(int32t);

    // Detect wrapper name using FFI marshaller
    List<FFIMarshaller::CType> param_types(1);
    param_types.add(FFIMarshaller::detect_c_type(int32t));
    FFIMarshaller::CType return_type = FFIMarshaller::detect_c_type(int32t);

    String wrapper = FFIMarshaller::detect_wrapper_name(param_types, return_type);
    // Result: wrapper == "ffi_i32_i32"

    String sig_str = FFIMarshaller::format_signature(param_types, return_type);
    // Result: sig_str == "int32 (int32)"

    print("abs signature: " + sig_str);
    print("abs wrapper: " + wrapper);
}

// ==============================================================================
// Example 2: floor(double) -> double
// ==============================================================================
void example_floor_signature() {
    // Function: double floor(double value)
    // Library: libm (math.h)

    Signature floor_sig;
    floor_sig.add(float64t, "value");
    floor_sig.return_types.add(float64t);

    // Detect wrapper
    List<FFIMarshaller::CType> param_types(1);
    param_types.add(FFIMarshaller::detect_c_type(float64t));
    FFIMarshaller::CType return_type = FFIMarshaller::detect_c_type(float64t);

    String wrapper = FFIMarshaller::detect_wrapper_name(param_types, return_type);
    // Result: wrapper == "ffi_f64_f64"

    String sig_str = FFIMarshaller::format_signature(param_types, return_type);
    // Result: sig_str == "float64 (float64)"

    print("floor signature: " + sig_str);
    print("floor wrapper: " + wrapper);
}

// ==============================================================================
// Example 3: strlen(char*) -> size_t
// ==============================================================================
void example_strlen_signature() {
    // Function: size_t strlen(const char* str)
    // Library: libc (string.h)
    // Note: size_t maps to int32t in wasm

    Signature strlen_sig;
    strlen_sig.add(charp, "str");
    strlen_sig.return_types.add(int32t);  // size_t -> i32

    // Detect wrapper
    List<FFIMarshaller::CType> param_types(1);
    param_types.add(FFIMarshaller::detect_c_type(charp));
    FFIMarshaller::CType return_type = FFIMarshaller::detect_c_type(int32t);

    String wrapper = FFIMarshaller::detect_wrapper_name(param_types, return_type);
    // Result: wrapper == "ffi_str_i32"

    String sig_str = FFIMarshaller::format_signature(param_types, return_type);
    // Result: sig_str == "int32 (char*)"

    print("strlen signature: " + sig_str);
    print("strlen wrapper: " + wrapper);
}

// ==============================================================================
// Example 4: atof(char*) -> double
// ==============================================================================
void example_atof_signature() {
    // Function: double atof(const char* str)
    // Library: libc (stdlib.h)

    Signature atof_sig;
    atof_sig.add(charp, "str");
    atof_sig.return_types.add(float64t);

    // Detect wrapper
    List<FFIMarshaller::CType> param_types(1);
    param_types.add(FFIMarshaller::detect_c_type(charp));
    FFIMarshaller::CType return_type = FFIMarshaller::detect_c_type(float64t);

    String wrapper = FFIMarshaller::detect_wrapper_name(param_types, return_type);
    // Result: wrapper == "ffi_str_f64"

    String sig_str = FFIMarshaller::format_signature(param_types, return_type);
    // Result: sig_str == "float64 (char*)"

    print("atof signature: " + sig_str);
    print("atof wrapper: " + wrapper);
}

// ==============================================================================
// Example 5: fmin(double, double) -> double
// ==============================================================================
void example_fmin_signature() {
    // Function: double fmin(double x, double y)
    // Library: libm (math.h)

    Signature fmin_sig;
    fmin_sig.add(float64t, "x");
    fmin_sig.add(float64t, "y");
    fmin_sig.return_types.add(float64t);

    // Detect wrapper
    List<FFIMarshaller::CType> param_types(2);
    param_types.add(FFIMarshaller::detect_c_type(float64t));
    param_types.add(FFIMarshaller::detect_c_type(float64t));
    FFIMarshaller::CType return_type = FFIMarshaller::detect_c_type(float64t);

    String wrapper = FFIMarshaller::detect_wrapper_name(param_types, return_type);
    // Result: wrapper == "ffi_f64_f64_f64"

    String sig_str = FFIMarshaller::format_signature(param_types, return_type);
    // Result: sig_str == "float64 (float64, float64)"

    print("fmin signature: " + sig_str);
    print("fmin wrapper: " + wrapper);
}

// ==============================================================================
// Universal Signature Detection Function
// ==============================================================================
String detect_ffi_wrapper(Signature& sig) {
    // Convert Wasp signature to FFI parameter types
    List<FFIMarshaller::CType> param_types(sig.parameters.size());
    for (int i = 0; i < sig.parameters.size(); i++) {
        Type wasp_type = sig.parameters[i].type;
        FFIMarshaller::CType c_type = FFIMarshaller::detect_c_type(wasp_type);
        param_types.add(c_type);
    }

    // Get return type
    Type return_wasp_type = sig.return_types.last(none);
    FFIMarshaller::CType return_c_type = FFIMarshaller::detect_c_type(return_wasp_type);

    // Detect appropriate wrapper
    String wrapper_name = FFIMarshaller::detect_wrapper_name(param_types, return_c_type);

    if (wrapper_name.empty()) {
        String sig_str = FFIMarshaller::format_signature(param_types, return_c_type);
        warn("No FFI wrapper found for signature: " + sig_str);
    }

    return wrapper_name;
}

// ==============================================================================
// Comprehensive Signature Type Coverage
// ==============================================================================
void demonstrate_signature_coverage() {
    print("=== FFI Signature Coverage ===\n");

    // Type: int32 -> int32
    example_abs_signature();
    print("");

    // Type: float64 -> float64
    example_floor_signature();
    print("");

    // Type: char* -> int32
    example_strlen_signature();
    print("");

    // Type: char* -> float64
    example_atof_signature();
    print("");

    // Type: float64, float64 -> float64
    example_fmin_signature();
    print("");

    print("=== Additional Supported Patterns ===");
    print("ffi_i32_i32_i32:     int32 (int32, int32)");
    print("ffi_str_str:         char* (char*)");
    print("ffi_str_str_i32:     int32 (char*, char*)");
    print("ffi_void_i32:        int32 (void)");
    print("ffi_void_f64:        float64 (void)");
}

// ==============================================================================
// Usage Example: Registering FFI Function with Signature
// ==============================================================================
struct FFIFunctionRegistration {
    String function_name;
    String library_name;
    Signature signature;
    String wrapper_name;

    void register_function() {
        // Automatically detect wrapper from signature
        wrapper_name = detect_ffi_wrapper(signature);

        print("Registering FFI function:");
        print("  Name: " + function_name);
        print("  Library: " + library_name);
        print("  Wrapper: " + wrapper_name);
    }
};

void example_registration() {
    // Register abs function
    FFIFunctionRegistration abs_reg;
    abs_reg.function_name = "abs";
    abs_reg.library_name = "c";
    abs_reg.signature.add(int32t, "value");
    abs_reg.signature.return_types.add(int32t);
    abs_reg.register_function();
    // Output: Wrapper: ffi_i32_i32

    print("");

    // Register strlen function
    FFIFunctionRegistration strlen_reg;
    strlen_reg.function_name = "strlen";
    strlen_reg.library_name = "c";
    strlen_reg.signature.add(charp, "str");
    strlen_reg.signature.return_types.add(int32t);
    strlen_reg.register_function();
    // Output: Wrapper: ffi_str_i32

    print("");

    // Register fmin function
    FFIFunctionRegistration fmin_reg;
    fmin_reg.function_name = "fmin";
    fmin_reg.library_name = "m";
    fmin_reg.signature.add(float64t, "x");
    fmin_reg.signature.add(float64t, "y");
    fmin_reg.signature.return_types.add(float64t);
    fmin_reg.register_function();
    // Output: Wrapper: ffi_f64_f64_f64
}
