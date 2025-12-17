// FFI Function Signature Database - Reuses existing Type and Signature classes
#pragma once

#include "String.h"
#include "Code.h"  // For Signature, Arg, Type
#include <cstring>

// Helper: Manual string comparison (workaround for String == operator issue)
static inline bool str_eq(const String& s, const char* literal) {
    int len = strlen(literal);
    if (s.length != len) return false;
    for (int i = 0; i < len; i++) {
        if (s.data[i] != literal[i]) return false;
    }
    return true;
}

// Detect function signature based on function name and library
// Populates the provided Signature object with parameter and return types
inline void detect_ffi_signature(const String& func_name, const String& lib_name, Signature& sig) {
    sig.parameters.clear();
    sig.return_types.clear();

    // Helper to add parameter
    auto add_param = [&](Type type) {
        Arg param;
        param.type = type;
        sig.parameters.add(param);
    };

    // Math library functions (libm)
    if (lib_name.data[0] == 'm' && lib_name.length == 1) {
        // Most math functions: double -> double
        if (str_eq(func_name, "sqrt") || str_eq(func_name, "floor") || str_eq(func_name, "ceil") ||
            str_eq(func_name, "sin") || str_eq(func_name, "cos") || str_eq(func_name, "tan") ||
            str_eq(func_name, "fabs") || str_eq(func_name, "log") || str_eq(func_name, "exp")) {
            add_param(float64t);
            sig.return_types.add(float64t);
        }
        // Two-parameter math functions: (double, double) -> double
        else if (str_eq(func_name, "pow") || str_eq(func_name, "fmod") ||
                 str_eq(func_name, "fmax") || str_eq(func_name, "fmin")) {
            add_param(float64t);
            add_param(float64t);
            sig.return_types.add(float64t);
        }
        else {
            // Default: double -> double
            add_param(float64t);
            sig.return_types.add(float64t);
        }
    }
    // C library functions (libc)
    else if (lib_name.data[0] == 'c' && lib_name.length == 1) {
        if (str_eq(func_name, "abs")) {
            // int abs(int)
            add_param(int32t);
            sig.return_types.add(int32t);
        }
        else if (str_eq(func_name, "strcmp") || str_eq(func_name, "strcoll")) {
            // int strcmp(const char*, const char*)
            add_param(charp);
            add_param(charp);
            sig.return_types.add(int32t);
        }
        else if (str_eq(func_name, "strlen")) {
            // size_t strlen(const char*)
            add_param(charp);
            sig.return_types.add(int32t);
        }
        else if (str_eq(func_name, "atoi")) {
            // int atoi(const char*)
            add_param(charp);
            sig.return_types.add(int32t);
        }
        else if (str_eq(func_name, "atof") || str_eq(func_name, "strtod")) {
            // double atof(const char*)
            add_param(charp);
            sig.return_types.add(float64t);
        }
        else if (str_eq(func_name, "rand")) {
            // int rand(void)
            sig.return_types.add(int32t);
        }
        else {
            // Default: int -> int
            add_param(int32t);
            sig.return_types.add(int32t);
        }
    }
    else {
        // Unknown library - default to double -> double
        add_param(float64t);
        sig.return_types.add(float64t);
    }
}
