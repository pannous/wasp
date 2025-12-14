// FFI Function Signature Database - Reuses existing Type and Signature classes
#pragma once

#include "String.h"
#include "Code.h"  // For Signature, Arg, Type

// Detect function signature based on function name and library
// Populates the provided Signature object with parameter and return types
static void detect_ffi_signature(const String& func_name, const String& lib_name, Signature& sig) {
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
        if (func_name == "sqrt" || func_name == "floor" || func_name == "ceil" ||
            func_name == "sin" || func_name == "cos" || func_name == "tan" ||
            func_name == "fabs" || func_name == "log" || func_name == "exp") {
            add_param(float64t);
            sig.return_types.add(float64t);
        }
        // Two-parameter math functions: (double, double) -> double
        else if (func_name == "pow" || func_name == "fmod" ||
                 func_name == "fmax" || func_name == "fmin") {
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
        if (func_name.data[0] == 'a' && func_name.data[1] == 'b' && func_name.data[2] == 's' && func_name.length == 3) {
            // int abs(int)
            add_param(int32t);
            sig.return_types.add(int32t);
        }
        else if (func_name == "strcmp" || func_name == "strcoll") {
            // int strcmp(const char*, const char*)
            add_param(charp);
            add_param(charp);
            sig.return_types.add(int32t);
        }
        else if (func_name == "strlen") {
            // size_t strlen(const char*)
            add_param(charp);
            sig.return_types.add(int32t);
        }
        else if (func_name == "atoi") {
            // int atoi(const char*)
            add_param(charp);
            sig.return_types.add(int32t);
        }
        else if (func_name == "atof" || func_name == "strtod") {
            // double atof(const char*)
            add_param(charp);
            sig.return_types.add(float64t);
        }
        else if (func_name == "rand") {
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
