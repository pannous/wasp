// FFI Function Signature Database
// Maps common library functions to their signatures

#pragma once

#include "String.h"
#include "List.h"
#include "ffi_marshaller.h"

// Use the types from FFIMarshaller
using CType = FFIMarshaller::CType;
using FFISignature = FFIMarshaller::FFISignature;

// Signature detection based on function name and library
static FFISignature detect_signature(const String& func_name, const String& lib_name, void* func_ptr) {
    FFISignature sig;
    sig.function_name = func_name;
    sig.function_ptr = func_ptr;

    // Math library functions (libm)
    if (lib_name.data[0] == 'm' && lib_name.length == 1) {
        // Most math functions: double -> double
        if (func_name == "sqrt" || func_name == "floor" || func_name == "ceil" ||
            func_name == "sin" || func_name == "cos" || func_name == "tan" ||
            func_name == "fabs" || func_name == "log" || func_name == "exp") {
            sig.param_types.add(CType::Float64);
            sig.return_type = CType::Float64;
        }
        // Two-parameter math functions: (double, double) -> double
        else if (func_name == "pow" || func_name == "fmod" ||
                 func_name == "fmax" || func_name == "fmin") {
            sig.param_types.add(CType::Float64);
            sig.param_types.add(CType::Float64);
            sig.return_type = CType::Float64;
        }
        else {
            // Default: double -> double
            sig.param_types.add(CType::Float64);
            sig.return_type = CType::Float64;
        }
    }
    // C library functions (libc)
    else if (lib_name.data[0] == 'c' && lib_name.length == 1) {
        if (func_name.data[0] == 'a' && func_name.data[1] == 'b' && func_name.data[2] == 's' && func_name.length == 3) {
            // int abs(int)
            sig.param_types.add(CType::Int32);
            sig.return_type = CType::Int32;
        }
        else if (func_name == "strcmp" || func_name == "strcoll") {
            // int strcmp(const char*, const char*)
            sig.param_types.add(CType::String);
            sig.param_types.add(CType::String);
            sig.return_type = CType::Int32;
        }
        else if (func_name == "strlen") {
            // size_t strlen(const char*)
            sig.param_types.add(CType::String);
            sig.return_type = CType::Int32;
        }
        else if (func_name == "atoi") {
            // int atoi(const char*)
            sig.param_types.add(CType::String);
            sig.return_type = CType::Int32;
        }
        else if (func_name == "atof" || func_name == "strtod") {
            // double atof(const char*)
            sig.param_types.add(CType::String);
            sig.return_type = CType::Float64;
        }
        else if (func_name == "rand") {
            // int rand(void)
            sig.return_type = CType::Int32;
        }
        else {
            // Default: int -> int
            sig.param_types.add(CType::Int32);
            sig.return_type = CType::Int32;
        }
    }
    else {
        // Unknown library - default to double -> double
        sig.param_types.add(CType::Float64);
        sig.return_type = CType::Float64;
    }

    return sig;
}
