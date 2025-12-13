// FFI Marshaller - Shared type conversion between Wasp and C/FFI
// Used by both WasmEdge and Wasmtime runners

#pragma once

#include "String.h"
#include "Node.h"
#include "List.h"  // For List template
#include <cstring>

class FFIMarshaller {
public:
    // ============================================================================
    // Basic Type Conversions (pass-through, no conversion needed)
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
    // String Conversions
    // ============================================================================

    // Convert Wasp String to C string (char*)
    // Note: Wasp strings are already null-terminated and stored in wasm memory
    static inline const char* to_c_string(const String& str) {
        return str.data;
    }

    // Convert C string to Wasp String
    static inline String from_c_string(const char* cstr) {
        if (!cstr) return String("");
        return String(cstr);
    }

    // For wasm memory offsets (when strings are passed as i32 offsets)
    static inline const char* offset_to_c_string(void* wasm_memory, int32_t offset) {
        if (!wasm_memory || offset < 0) return "";
        return (const char*)((uint8_t*)wasm_memory + offset);
    }

    // Allocate string result in wasm memory
    // Returns offset where the string was written
    static inline int32_t c_string_to_offset(void* wasm_memory, const char* cstr, int32_t heap_offset = 0x10000) {
        if (!wasm_memory || !cstr) return 0;
        // TODO: Proper heap management instead of fixed offset
        strcpy((char*)((uint8_t*)wasm_memory + heap_offset), cstr);
        return heap_offset;
    }

    // ============================================================================
    // Node/Struct Conversions (Placeholders for future implementation)
    // ============================================================================

    // TODO: Convert Wasp Node to C struct
    // This will require:
    // 1. Analyzing the Node structure
    // 2. Creating a C struct layout in wasm memory
    // 3. Copying Node fields to the struct
    template<typename CStruct>
    static CStruct* node_to_c_struct(Node* node, void* wasm_memory) {
        // Placeholder - to be implemented
        warn("node_to_c_struct not yet implemented");
        return nullptr;
    }

    // TODO: Convert C struct to Wasp Node
    // This will require:
    // 1. Reading struct fields from wasm memory
    // 2. Creating a new Node with those values
    // 3. Setting appropriate types for each field
    template<typename CStruct>
    static Node* c_struct_to_node(const CStruct* c_struct) {
        // Placeholder - to be implemented
        warn("c_struct_to_node not yet implemented");
        return nullptr;
    }

    // ============================================================================
    // Array/Buffer Conversions (Placeholders)
    // ============================================================================

    // TODO: Convert Wasp array to C array
    static void* array_to_c_array(Node* array_node, void* wasm_memory) {
        // Placeholder - to be implemented
        warn("array_to_c_array not yet implemented");
        return nullptr;
    }

    // TODO: Convert C array to Wasp array
    static Node* c_array_to_node(const void* c_array, size_t count, size_t element_size) {
        // Placeholder - to be implemented
        warn("c_array_to_node not yet implemented");
        return nullptr;
    }

    // ============================================================================
    // Type Signature Detection (for automatic marshalling)
    // ============================================================================

    enum class CType {
        Void,
        Int32,
        Int64,
        Float32,
        Float64,
        String,    // char*
        Struct,    // Generic struct
        Array,     // Generic array/buffer
        Unknown
    };

    // Detect C type from Wasp Type
    static CType detect_c_type(Type wasp_type) {
        switch (wasp_type.value) {
            case int32t: return CType::Int32;
            case i64: return CType::Int64;
            case float32t: return CType::Float32;
            case float64t: return CType::Float64;
            case charp: return CType::String;
            case strings: return CType::String;
            // TODO: Add struct/array detection
            default: return CType::Unknown;
        }
    }

    // Get size of C type for memory allocation
    static size_t c_type_size(CType type) {
        switch (type) {
            case CType::Int32: return 4;
            case CType::Int64: return 8;
            case CType::Float32: return 4;
            case CType::Float64: return 8;
            case CType::String: return sizeof(char*);
            default: return 0;
        }
    }

    // ============================================================================
    // Signature Pattern Detection (for automatic wrapper selection)
    // ============================================================================

    struct SignaturePattern {
        List<CType> param_types;
        CType return_type;
        String wrapper_name;

        SignaturePattern() : param_types(4), wrapper_name("") {
            return_type = CType::Void;
        }

        // Helper to match signature patterns
        bool matches(int param_count, CType p1, CType p2, CType p3, CType ret) {
            if (param_types.size() != param_count) return false;
            if (return_type != ret) return false;
            if (param_count >= 1 && param_types[0] != p1) return false;
            if (param_count >= 2 && param_types[1] != p2) return false;
            if (param_count >= 3 && param_types[2] != p3) return false;
            return true;
        }
    };

    // Detect function signature pattern and return wrapper name
    // This enables automatic selection of the appropriate FFI wrapper
    static String detect_wrapper_name(List<CType>& param_types, CType return_type) {
        int param_count = param_types.size();

        // Single parameter functions
        if (param_count == 1) {
            CType p1 = param_types[0];

            // int32 -> int32 (abs, etc.)
            if (p1 == CType::Int32 && return_type == CType::Int32)
                return "ffi_i32_i32";

            // float64 -> float64 (floor, sqrt, etc.)
            if (p1 == CType::Float64 && return_type == CType::Float64)
                return "ffi_f64_f64";

            // char* -> int32 (strlen, etc.)
            if (p1 == CType::String && return_type == CType::Int32)
                return "ffi_str_i32";

            // char* -> float64 (atof, etc.)
            if (p1 == CType::String && return_type == CType::Float64)
                return "ffi_str_f64";

            // char* -> char* (strdup, etc.)
            if (p1 == CType::String && return_type == CType::String)
                return "ffi_str_str";
        }

        // Two parameter functions
        if (param_count == 2) {
            CType p1 = param_types[0];
            CType p2 = param_types[1];

            // float64, float64 -> float64 (fmin, fmax, pow, fmod, etc.)
            if (p1 == CType::Float64 && p2 == CType::Float64 && return_type == CType::Float64)
                return "ffi_f64_f64_f64";

            // int32, int32 -> int32 (max, min, gcd, etc.)
            if (p1 == CType::Int32 && p2 == CType::Int32 && return_type == CType::Int32)
                return "ffi_i32_i32_i32";

            // char*, char* -> int32 (strcmp, etc.)
            if (p1 == CType::String && p2 == CType::String && return_type == CType::Int32)
                return "ffi_str_str_i32";
        }

        // No parameter functions
        if (param_count == 0) {
            // void -> int32 (rand, etc.)
            if (return_type == CType::Int32)
                return "ffi_void_i32";

            // void -> float64 (time-related, etc.)
            if (return_type == CType::Float64)
                return "ffi_void_f64";
        }

        return ""; // Unknown signature
    }

    // Convert CType to readable string for debugging
    static String c_type_name(CType type) {
        switch (type) {
            case CType::Void: return "void";
            case CType::Int32: return "int32";
            case CType::Int64: return "int64";
            case CType::Float32: return "float32";
            case CType::Float64: return "float64";
            case CType::String: return "char*";
            case CType::Struct: return "struct";
            case CType::Array: return "array";
            default: return "unknown";
        }
    }

    // Format signature as readable string
    static String format_signature(List<CType>& param_types, CType return_type) {
        String sig = c_type_name(return_type) + " (";
        for (int i = 0; i < param_types.size(); i++) {
            if (i > 0) sig += ", ";
            sig += c_type_name(param_types[i]);
        }
        sig += ")";
        return sig;
    }
};

// Helper macros for common conversions
#define FFI_TO_C_DOUBLE(val) FFIMarshaller::to_c_double(val)
#define FFI_FROM_C_DOUBLE(val) FFIMarshaller::from_c_double(val)
#define FFI_TO_C_STRING(str) FFIMarshaller::to_c_string(str)
#define FFI_FROM_C_STRING(cstr) FFIMarshaller::from_c_string(cstr)
