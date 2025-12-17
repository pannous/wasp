#pragma once
#include "String.h"
#include "Node.h"
#include "List.h"
#include "Map.h"
#include "Wasp.h"
#include "Angle.h"
#include "NodeTypes.h"

// Parse C header files and extract FFI function signatures
// Uses Wasp's own parser to understand C function declarations

struct FFIHeaderSignature {
    String name;
    String return_type;
    List<String> param_types;
    String library; // "c" or "m"
};

// Map C type names to Wasp type names
inline Type mapCTypeToWasp(String c_type) {
    // Handle basic types
    if (c_type == "double") return float64t;
    if (c_type == "float") return float32t;
    if (c_type == "int") return int32t;
    if (c_type == "long") return i64;
    if (c_type == "void") return nils;

    // Handle string pointers specially
    if (c_type == "char*" or c_type == "const char*") return charp;
    if (c_type.contains("char") and c_type.contains("*")) return charp;

    // All other pointers (SDL_Window*, SDL_Renderer*, etc.) as opaque i64
    // FFI pointers are handles/tokens - we don't access their internals
    // Note: pointer64 = wasm_int64 = i64, but i64 is clearer
    if (c_type.contains("*")) return i64;

    // Unsigned types
    if (c_type.contains("Uint32") or c_type == "uint32_t") return int32t;
    if (c_type.contains("Uint8") or c_type == "uint8_t") return int32t;

    return unknown_type;
}

// Format parameter list for signature
inline String formatParamList(const List<String>& params) {
    if (params.size() == 0) return "";

    String result;
    for (int i = 0; i < params.size(); i++) {
        if (i > 0) result += ", ";
        Type t = mapCTypeToWasp(params[i]);
        result += typeName(t);
    }
    return result;
}

// Generate FFI signature line for ffi_signatures.h
inline String generateFFISignatureLine(const FFIHeaderSignature& sig) {
    Type ret = mapCTypeToWasp(sig.return_type);
    String params = formatParamList(sig.param_types);

    return "    ADD_FFI_SIG(\""s + sig.name + "\", \"" + sig.library + "\", " +
           typeName(ret) + ", {" + params + "})";
}

// Extract function signature from parsed Node
// Handles patterns like: double sqrt(double x) and int strlen(char* str)
inline bool extractFunctionSignature(Node& node, FFIHeaderSignature& sig) {
    // Pattern 1: "double sqrt(double x)" parses as:
    //   group[reference("double"), group("sqrt", [reference("double"), reference("x")])]
    // Pattern 2: "int strlen(char* str)" parses as:
    //   group[reference("int"), operator("strlen"), group([expression[char, *, str]])]

    if (node.length < 2) return false;

    // First child is always return type
    Node& return_type_node = node.first();
    if (return_type_node.name.empty()) return false;

    String ret_type = return_type_node.name;
    if (ret_type != "double" and ret_type != "float" and
        ret_type != "int" and ret_type != "long" and
        ret_type != "void" and not ret_type.contains("char")) {
        return false; // Not a function declaration
    }
    sig.return_type = ret_type;

    // Determine which pattern we have
    if (node.length == 2) {
        // Pattern 1: function is in second child with parameters as children
        Node& func = node[1];
        sig.name = func.name;
        if (sig.name.empty()) return false;

        // Extract parameters from func children
        for (int i = 0; i < func.length; i++) {
            Node& param = func[i];
            String ptype;
            if (param.kind == groups && param.length > 0) {
                ptype = param.first().name;
            } else if (i % 2 == 0) {
                ptype = param.name;
            } else {
                continue; // Skip param names
            }
            if (not ptype.empty()) sig.param_types.add(ptype);
        }
    } else if (node.length == 3) {
        // Pattern 2: function name is second child, params are in third child group
        sig.name = node[1].name;
        if (sig.name.empty()) return false;

        Node& param_group = node[2];
        if (param_group.length > 0 && param_group[0].length > 0) {
            Node& param_expr = param_group[0]; // expression like [char, *, str]

            // Build parameter type from expression tokens
            String ptype;
            for (int i = 0; i < param_expr.length; i++) {
                String token = param_expr[i].name;
                if (token == "*") {
                    ptype += "*";
                } else if (token != "str" and token != "x" and token != "y") {
                    // It's a type name (char, int, double, etc.)
                    if (not ptype.empty()) {
                        sig.param_types.add(ptype);
                        ptype = "";
                    }
                    ptype = token;
                }
            }
            if (not ptype.empty()) {
                sig.param_types.add(ptype);
            }
        }
    } else {
        return false; // Unknown pattern
    }

    return true;
}

// Parse a C header file and extract all function signatures
inline List<FFIHeaderSignature> parseHeaderFile(const String& header_path, const String& library) {
    List<FFIHeaderSignature> signatures;

    // Read header file
    String content = readFile(header_path);
    if (content.empty()) {
        warn("Failed to read header file: "s + header_path);
        return signatures;
    }

    // Parse using Wasp's parser
    Node& root = parse(content);

    // Walk AST and extract function declarations
    // TODO: Implement tree walk to find all function declarations
    // For now, just process top-level nodes
    for (Node& node : root) {
        FFIHeaderSignature sig;
        sig.library = library;
        if (extractFunctionSignature(node, sig)) {
            signatures.add(sig);
            print("Found FFI function: "s + sig.name);
        }
    }

    return signatures;
}

// Generate ffi_signatures.h content from parsed headers
inline String generateFFISignaturesHeader() {
    String output = "// Auto-generated FFI signatures from C headers\n";
    output += "// Generated by ffi_header_parser.h\n\n";

    // Parse math.h
    List<FFIHeaderSignature> math_sigs = parseHeaderFile("/usr/include/math.h", "m");

    // Parse string.h
    List<FFIHeaderSignature> string_sigs = parseHeaderFile("/usr/include/string.h", "c");

    // Parse stdlib.h
    List<FFIHeaderSignature> stdlib_sigs = parseHeaderFile("/usr/include/stdlib.h", "c");

    output += "// Math functions from math.h\n";
    for (auto& sig : math_sigs) {
        output += generateFFISignatureLine(sig) + "\n";
    }

    output += "\n// String functions from string.h\n";
    for (auto& sig : string_sigs) {
        output += generateFFISignatureLine(sig) + "\n";
    }

    output += "\n// Stdlib functions from stdlib.h\n";
    for (auto& sig : stdlib_sigs) {
        output += generateFFISignatureLine(sig) + "\n";
    }

    return output;
}
