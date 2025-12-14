// FFI Marshaller - Runtime type conversion for dynamic FFI wrapper
// Minimal runtime support for calling native C functions from WASM

#pragma once

#include "String.h"
#include "List.h"
#include "Code.h"  // For Type and Signature
#include <cstring>

class FFIMarshaller {
public:
    // ============================================================================
    // C Type Enum (for runtime dispatch only)
    // ============================================================================

    enum class CType {
        Void,
        Int32,
        Int64,
        Float32,
        Float64,
        String,    // char*
        Unknown
    };

    // ============================================================================
    // Runtime FFI Signature (used by dynamic wrapper)
    // ============================================================================

    struct FFISignature {
        List<CType> param_types;
        CType return_type;
        void* function_ptr;
        String function_name;

        FFISignature() : param_types(8), function_ptr(nullptr), function_name("") {
            return_type = CType::Void;
        }
    };

    // ============================================================================
    // Conversion from Wasp's Type to runtime CType
    // ============================================================================

    static CType wasp_type_to_ctype(Type type) {
        switch (type.value) {
            case int32t: return CType::Int32;
            case i64: return CType::Int64;
            case float32t: return CType::Float32;
            case float64t: return CType::Float64;
            case charp: return CType::String;
            case nils: return CType::Void;
            default: return CType::Unknown;
        }
    }

    // ============================================================================
    // Conversion from Wasp's Signature to runtime FFISignature
    // ============================================================================

    static FFISignature wasp_signature_to_ffi(Signature& wasp_sig, void* func_ptr, const String& func_name) {
        FFISignature ffi_sig;
        ffi_sig.function_ptr = func_ptr;
        ffi_sig.function_name = func_name;

        // Convert parameters
        for (int i = 0; i < wasp_sig.parameters.size(); i++) {
            CType ctype = wasp_type_to_ctype(wasp_sig.parameters[i].type);
            ffi_sig.param_types.add(ctype);
        }

        // Convert return type
        if (wasp_sig.return_types.size() > 0) {
            ffi_sig.return_type = wasp_type_to_ctype(wasp_sig.return_types.last());
        } else {
            ffi_sig.return_type = CType::Void;
        }

        return ffi_sig;
    }

    // ============================================================================
    // Helper: Convert C string to WASM memory offset (for runtime)
    // ============================================================================

    static int32_t c_string_to_offset(void* wasm_memory, const char* c_str) {
        if (!wasm_memory || !c_str) return 0;
        // Calculate offset from base of WASM memory
        return (int32_t)((uint8_t*)c_str - (uint8_t*)wasm_memory);
    }

    // ============================================================================
    // Helper: Detect CType from Wasp Type (for legacy code)
    // ============================================================================

    static CType detect_c_type(Type wasp_type) {
        return wasp_type_to_ctype(wasp_type);
    }

    // ============================================================================
    // Test Helpers: Format signature for debugging/testing
    // ============================================================================

    static String format_c_type(CType type) {
        switch (type) {
            case CType::Void: return "void";
            case CType::Int32: return "int32";
            case CType::Int64: return "int64";
            case CType::Float32: return "float32";
            case CType::Float64: return "float64";
            case CType::String: return "char*";
            default: return "unknown";
        }
    }

    static String format_signature(const List<CType>& params, CType return_type) {
        String result = format_c_type(return_type) + " (";
        for (int i = 0; i < params.size(); i++) {
            if (i > 0) result = result + ", ";
            result = result + format_c_type(params[i]);
        }
        result = result + ")";
        return result;
    }

    static String ctype_short_name(CType type) {
        switch (type) {
            case CType::Int32: return "i32";
            case CType::Int64: return "i64";
            case CType::Float32: return "f32";
            case CType::Float64: return "f64";
            case CType::String: return "str";
            case CType::Void: return "void";
            default: return "unk";
        }
    }

    static String detect_wrapper_name(const List<CType>& params, CType return_type) {
        // Legacy test helper - returns wrapper name in format: ffi_<params>_<return>
        String name = "ffi";
        for (int i = 0; i < params.size(); i++) {
            name = name + "_" + ctype_short_name(params[i]);
        }
        name = name + "_" + ctype_short_name(return_type);
        return name;
    }

    // ============================================================================
    // Runtime Value Union (for marshalling between WASM and C)
    // ============================================================================

    union FFIValue {
        int32_t i32;
        int64_t i64;
        float f32;
        double f64;
        const char* str;
        void* ptr;
    };

    // ============================================================================
    // String Conversions (for runtime)
    // ============================================================================

    // Convert wasm memory offset to C string
    static inline const char* offset_to_c_string(void* wasm_memory, int32_t offset) {
        if (!wasm_memory || offset < 0) return "";
        return (const char*)((uint8_t*)wasm_memory + offset);
    }

    // ============================================================================
    // Basic Type Conversions (pass-through, for consistency)
    // ============================================================================

    static inline double to_c_double(double val) { return val; }
    static inline float to_c_float(float val) { return val; }
    static inline int64_t to_c_int64(int64_t val) { return val; }
    static inline int32_t to_c_int32(int32_t val) { return val; }

    static inline double from_c_double(double val) { return val; }
    static inline float from_c_float(float val) { return val; }
    static inline int64_t from_c_int64(int64_t val) { return val; }
    static inline int32_t from_c_int32(int32_t val) { return val; }

    // ============================================================================
    // Dynamic FFI Function Calling (runtime dispatch)
    // ============================================================================

    // Dynamically call a C function based on runtime signature information
    // This eliminates the need for hardcoded wrappers for each signature pattern
    static FFIValue call_dynamic(FFISignature& sig, List<FFIValue>& args) {
        FFIValue result;
        result.i64 = 0;

        int param_count = sig.param_types.size();

        // 0 parameters
        if (param_count == 0) {
            if (sig.return_type == CType::Int32) {
                typedef int32_t (*func_t)();
                result.i32 = ((func_t)sig.function_ptr)();
            } else if (sig.return_type == CType::Float64) {
                typedef double (*func_t)();
                result.f64 = ((func_t)sig.function_ptr)();
            }
        }
        // 1 parameter
        else if (param_count == 1) {
            CType p1 = sig.param_types[0];

            if (p1 == CType::Int32 && sig.return_type == CType::Int32) {
                typedef int32_t (*func_t)(int32_t);
                result.i32 = ((func_t)sig.function_ptr)(args[0].i32);
            }
            else if (p1 == CType::Float64 && sig.return_type == CType::Float64) {
                typedef double (*func_t)(double);
                result.f64 = ((func_t)sig.function_ptr)(args[0].f64);
            }
            else if (p1 == CType::String && sig.return_type == CType::Int32) {
                typedef int32_t (*func_t)(const char*);
                result.i32 = ((func_t)sig.function_ptr)(args[0].str);
            }
            else if (p1 == CType::String && sig.return_type == CType::Float64) {
                typedef double (*func_t)(const char*);
                result.f64 = ((func_t)sig.function_ptr)(args[0].str);
            }
        }
        // 2 parameters
        else if (param_count == 2) {
            CType p1 = sig.param_types[0];
            CType p2 = sig.param_types[1];

            if (p1 == CType::Float64 && p2 == CType::Float64 && sig.return_type == CType::Float64) {
                typedef double (*func_t)(double, double);
                result.f64 = ((func_t)sig.function_ptr)(args[0].f64, args[1].f64);
            }
            else if (p1 == CType::Int32 && p2 == CType::Int32 && sig.return_type == CType::Int32) {
                typedef int32_t (*func_t)(int32_t, int32_t);
                result.i32 = ((func_t)sig.function_ptr)(args[0].i32, args[1].i32);
            }
            else if (p1 == CType::String && p2 == CType::String && sig.return_type == CType::Int32) {
                typedef int32_t (*func_t)(const char*, const char*);
                result.i32 = ((func_t)sig.function_ptr)(args[0].str, args[1].str);
            }
        }
        // 3 parameters
        else if (param_count == 3) {
            CType p1 = sig.param_types[0];
            CType p2 = sig.param_types[1];
            CType p3 = sig.param_types[2];

            if (p1 == CType::Float64 && p2 == CType::Float64 && p3 == CType::Float64 &&
                sig.return_type == CType::Float64) {
                typedef double (*func_t)(double, double, double);
                result.f64 = ((func_t)sig.function_ptr)(args[0].f64, args[1].f64, args[2].f64);
            }
            else if (p1 == CType::Int32 && p2 == CType::Int32 && p3 == CType::Int32 &&
                     sig.return_type == CType::Int32) {
                typedef int32_t (*func_t)(int32_t, int32_t, int32_t);
                result.i32 = ((func_t)sig.function_ptr)(args[0].i32, args[1].i32, args[2].i32);
            }
        }

        return result;
    }
};
