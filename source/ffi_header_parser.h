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

// todo GET RID and use Signature
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

// Generate DEBUG FFI signature line for ffi_signatures.h
inline String generateFFISignatureLine(const FFIHeaderSignature& sig) {
    Type ret = mapCTypeToWasp(sig.return_type);
    String params = formatParamList(sig.param_types);
    return "    ADD_FFI_SIG(\""s + sig.name + "\", \"" + sig.library + "\", " +
           typeName(ret) + ", {" + params + "})";
}

// Extract function signature directly from C declaration string
// REPLACES AST-based parsing which was fragile and failed on "double ceil(double)"
// Handles: double ceil(double), float ceilf(float), int strlen(const char* str), etc.
inline bool extractFunctionSignatureFromString(const String& decl, FFIHeaderSignature& sig) {
    const char* s = decl.data;
    int len = decl.length;
    if (len == 0) return false;

    // Find opening paren
    int paren_start = -1;
    for (int i = 0; i < len; i++) {
        if (s[i] == '(') { paren_start = i; break; }
    }
    if (paren_start == -1) return false;

    // Find closing paren
    int paren_end = -1;
    for (int i = paren_start + 1; i < len; i++) {
        if (s[i] == ')') { paren_end = i; break; }
    }
    if (paren_end == -1) return false;

    // Extract function name (work backwards from '(' skipping whitespace)
    int name_end = paren_start - 1;
    while (name_end >= 0 && (s[name_end] == ' ' || s[name_end] == '\t')) name_end--;
    if (name_end < 0) return false;

    int name_start = name_end;
    while (name_start > 0 && (
        (s[name_start-1] >= 'a' && s[name_start-1] <= 'z') ||
        (s[name_start-1] >= 'A' && s[name_start-1] <= 'Z') ||
        (s[name_start-1] >= '0' && s[name_start-1] <= '9') ||
        s[name_start-1] == '_')) {
        name_start--;
    }

    sig.name = String((char*)(s + name_start), name_end - name_start + 1, false);

    // Extract return type (everything before function name, trimmed)
    int ret_end = name_start - 1;
    while (ret_end >= 0 && (s[ret_end] == ' ' || s[ret_end] == '\t')) ret_end--;
    if (ret_end < 0) return false;

    int ret_start = 0;
    while (ret_start <= ret_end && (s[ret_start] == ' ' || s[ret_start] == '\t')) ret_start++;

    sig.return_type = String((char*)(s + ret_start), ret_end - ret_start + 1, false);

    // Parse parameters
    int params_start = paren_start + 1;
    int params_len = paren_end - params_start;

    // Skip if empty or just whitespace/void
    if (params_len > 0) {
        const char* params = s + params_start;

        // Split by commas
        int tok_start = 0;
        for (int i = 0; i <= params_len; i++) {
            if (i == params_len || params[i] == ',') {
                int tok_len = i - tok_start;

                // Trim whitespace
                const char* tok = params + tok_start;
                while (tok_len > 0 && (*tok == ' ' || *tok == '\t')) { tok++; tok_len--; }
                while (tok_len > 0 && (tok[tok_len-1] == ' ' || tok[tok_len-1] == '\t')) tok_len--;

                if (tok_len > 0) {
                    String param_full((char*)tok, tok_len, false);

                    // Skip "void"
                    if (param_full == "void") {
                        tok_start = i + 1;
                        continue;
                    }

                    // Remove "const " prefix
                    String param_clean = param_full;
                    if (param_full.length > 6 && param_full.data[0] == 'c' &&
                        param_full.data[1] == 'o' && param_full.data[2] == 'n' &&
                        param_full.data[3] == 's' && param_full.data[4] == 't' &&
                        param_full.data[5] == ' ') {
                        param_clean = String((char*)(param_full.data + 6), param_full.length - 6, false);

                        // Trim again
                        const char* pc = param_clean.data;
                        int pc_len = param_clean.length;
                        while (pc_len > 0 && (*pc == ' ' || *pc == '\t')) { pc++; pc_len--; }
                        param_clean = String((char*)pc, pc_len, false);
                    }

                    // Extract type (handle "char*", "double", "int foo", "char* bar", etc.)
                    String param_type;
                    bool has_star = false;
                    int last_star = -1;

                    for (int j = 0; j < param_clean.length; j++) {
                        if (param_clean.data[j] == '*') {
                            has_star = true;
                            last_star = j;
                        }
                    }

                    if (has_star) {
                        // Include type + pointer: "char*"
                        param_type = String(param_clean.data, last_star + 1, false);
                    } else {
                        // No pointer - extract first word (the type)
                        int word_end = 0;
                        while (word_end < param_clean.length &&
                               param_clean.data[word_end] != ' ' &&
                               param_clean.data[word_end] != '\t') {
                            word_end++;
                        }
                        param_type = String(param_clean.data, word_end, false);
                    }

                    if (param_type.length > 0) {
                        sig.param_types.add(param_type);
                    }
                }

                tok_start = i + 1;
            }
        }
    }

    return true;
}

// OLD AST-based extraction - DEPRECATED, kept for reference
inline bool extractFunctionSignature(Node& node, FFIHeaderSignature& sig) {
    // AST parsing was too fragile - failed on "double ceil(double)" vs "float ceilf(float)"
    // Now we just convert the node back to string and use string parsing
    // TODO: Remove this once all callers use extractFunctionSignatureFromString
    return false;
}

// Parse a C header file and extract all function signatures
inline List<FFIHeaderSignature> parseHeaderFile(const String& header_path, const String& library) {
    List<FFIHeaderSignature> signatures;

    // Read header file
    String content = readFile(header_path.data);
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
