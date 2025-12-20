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
    // Runtime FFI Context (wraps Signature with function pointer)
    // ============================================================================

    struct FFIContext {
        void* function_ptr;
        String function_name;
        class Signature* signature;

        FFIContext() : function_ptr(nullptr), function_name(""), signature(nullptr) {}
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
    // Helper: Extract parameter CType from Signature
    // ============================================================================

    static CType get_param_ctype(const class Signature& sig, int index) {
        if (index >= sig.parameters.size()) return CType::Unknown;
        return wasp_type_to_ctype(sig.parameters[index].type);
    }

    // ============================================================================
    // Helper: Extract return CType from Signature
    // ============================================================================

    static CType get_return_ctype(const class Signature& sig) {
        if (sig.return_types.size() == 0) return CType::Void;
        return wasp_type_to_ctype(sig.return_types[sig.return_types.size() - 1]);
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
    static FFIValue call_dynamic(FFIContext& ctx, List<FFIValue>& args) {
        FFIValue result;
        result.i64 = 0;

        if (!ctx.signature || !ctx.function_ptr) return result;

        int param_count = ctx.signature->parameters.size();
        CType return_type = get_return_ctype(*ctx.signature);

        // 0 parameters
        if (param_count == 0) {
            if (return_type == CType::Int32) {
                typedef int32_t (*func_t)();
                result.i32 = ((func_t)ctx.function_ptr)();
            } else if (return_type == CType::Float64) {
                typedef double (*func_t)();
                result.f64 = ((func_t)ctx.function_ptr)();
            } else if (return_type == CType::Void) {
                typedef void (*func_t)();
                ((func_t)ctx.function_ptr)();
            }
        }
        // 1 parameter
        else if (param_count == 1) {
            CType p1 = get_param_ctype(*ctx.signature, 0);

            if (p1 == CType::Int32 && return_type == CType::Int32) {
                typedef int32_t (*func_t)(int32_t);
                result.i32 = ((func_t)ctx.function_ptr)(args[0].i32);
            }
            else if (p1 == CType::Int32 && return_type == CType::Void) {
                typedef void (*func_t)(int32_t);
                ((func_t)ctx.function_ptr)(args[0].i32);
            }
            else if (p1 == CType::Float64 && return_type == CType::Float64) {
                typedef double (*func_t)(double);
                result.f64 = ((func_t)ctx.function_ptr)(args[0].f64);
            }
            else if (p1 == CType::String && return_type == CType::Int32) {
                typedef int32_t (*func_t)(const char*);
                result.i32 = ((func_t)ctx.function_ptr)(args[0].str);
            }
            else if (p1 == CType::String && return_type == CType::Float64) {
                typedef double (*func_t)(const char*);
                result.f64 = ((func_t)ctx.function_ptr)(args[0].str);
            }
        }
        // 2 parameters
        else if (param_count == 2) {
            CType p1 = get_param_ctype(*ctx.signature, 0);
            CType p2 = get_param_ctype(*ctx.signature, 1);

            if (p1 == CType::Float64 && p2 == CType::Float64 && return_type == CType::Float64) {
                typedef double (*func_t)(double, double);
                result.f64 = ((func_t)ctx.function_ptr)(args[0].f64, args[1].f64);
            }
            else if (p1 == CType::Int32 && p2 == CType::Int32 && return_type == CType::Int32) {
                typedef int32_t (*func_t)(int32_t, int32_t);
                result.i32 = ((func_t)ctx.function_ptr)(args[0].i32, args[1].i32);
            }
            else if (p1 == CType::String && p2 == CType::String && return_type == CType::Int32) {
                typedef int32_t (*func_t)(const char*, const char*);
                result.i32 = ((func_t)ctx.function_ptr)(args[0].str, args[1].str);
            }
        }
        // 3 parameters
        else if (param_count == 3) {
            CType p1 = get_param_ctype(*ctx.signature, 0);
            CType p2 = get_param_ctype(*ctx.signature, 1);
            CType p3 = get_param_ctype(*ctx.signature, 2);

            if (p1 == CType::Float64 && p2 == CType::Float64 && p3 == CType::Float64 &&
                return_type == CType::Float64) {
                typedef double (*func_t)(double, double, double);
                result.f64 = ((func_t)ctx.function_ptr)(args[0].f64, args[1].f64, args[2].f64);
            }
            else if (p1 == CType::Int32 && p2 == CType::Int32 && p3 == CType::Int32 &&
                     return_type == CType::Int32) {
                typedef int32_t (*func_t)(int32_t, int32_t, int32_t);
                result.i32 = ((func_t)ctx.function_ptr)(args[0].i32, args[1].i32, args[2].i32);
            }
            // InitWindow(int, int, char*) -> void
            else if (p1 == CType::Int32 && p2 == CType::Int32 && p3 == CType::String &&
                     return_type == CType::Void) {
                typedef void (*func_t)(int32_t, int32_t, const char*);
                ((func_t)ctx.function_ptr)(args[0].i32, args[1].i32, args[2].str);
            }
        }
        // 4 parameters
        else if (param_count == 4) {
            CType p1 = get_param_ctype(*ctx.signature, 0);
            CType p2 = get_param_ctype(*ctx.signature, 1);
            CType p3 = get_param_ctype(*ctx.signature, 2);
            CType p4 = get_param_ctype(*ctx.signature, 3);

            // DrawCircle(int, int, float, int) -> void
            if (p1 == CType::Int32 && p2 == CType::Int32 && p3 == CType::Float32 && p4 == CType::Int32 &&
                return_type == CType::Void) {
                typedef void (*func_t)(int32_t, int32_t, float, int32_t);
                ((func_t)ctx.function_ptr)(args[0].i32, args[1].i32, args[2].f32, args[3].i32);
            }
        }
        // 5 parameters
        else if (param_count == 5) {
            CType p1 = get_param_ctype(*ctx.signature, 0);
            CType p2 = get_param_ctype(*ctx.signature, 1);
            CType p3 = get_param_ctype(*ctx.signature, 2);
            CType p4 = get_param_ctype(*ctx.signature, 3);
            CType p5 = get_param_ctype(*ctx.signature, 4);

            // DrawText(char*, int, int, int, int) -> void
            if (p1 == CType::String && p2 == CType::Int32 && p3 == CType::Int32 &&
                p4 == CType::Int32 && p5 == CType::Int32 && return_type == CType::Void) {
                typedef void (*func_t)(const char*, int32_t, int32_t, int32_t, int32_t);
                ((func_t)ctx.function_ptr)(args[0].str, args[1].i32, args[2].i32, args[3].i32, args[4].i32);
            }
            // DrawRectangle(int, int, int, int, int) -> void
            else if (p1 == CType::Int32 && p2 == CType::Int32 && p3 == CType::Int32 &&
                     p4 == CType::Int32 && p5 == CType::Int32 && return_type == CType::Void) {
                typedef void (*func_t)(int32_t, int32_t, int32_t, int32_t, int32_t);
                ((func_t)ctx.function_ptr)(args[0].i32, args[1].i32, args[2].i32, args[3].i32, args[4].i32);
            }
        }

        return result;
    }
};
