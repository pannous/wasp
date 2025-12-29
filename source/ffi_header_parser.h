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
    List<String> param_names;
    String library; // "c" or "m"
    String raw; // unparsed
    // Signature signature; // Wasp function signature
};

// Map C type names to Wasp type names
inline Type mapCTypeToWasp(String c_type) {
    // Handle basic types
    if (c_type == "") return voids;
    if (c_type == "typedef") return errors; // error
    if (c_type == "unsigned") return errors; // todo
    if (c_type == "double") return float64t;
    if (c_type == "float") return float32t;
    if (c_type == "int") return int32t;
    if (c_type == "bool") return bools;
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

    // Raylib structs passed by value (small structs <= 4 bytes passed as i32)
    // Color: struct with 4x uint8 (r,g,b,a) = 4 bytes total
    if (c_type == "Color") return int32t;
    // Vector3 Wave Texture2D Camera AudioCallback AudioStream todo get struct before!
    // warn("Unknown C type mapping for FFI: "s + c_type);
    // error("Unknown C type mapping for FFI: "s + c_type);
    return unknown_type;
}

// Format parameter list for signature
inline String formatParamList(const List<String> &params) {
    if (params.size() == 0) return "";

    String result;
    for (int i = 0; i < params.size(); i++) {
        if (i > 0) result += ", ";
        Type t = mapCTypeToWasp(params[i]);
        result += typeName(t);
    }
    return result;
}

// Extract function signature from a C-style declaration string.
// Handles: double ceil(double), float ceilf(float),
//          int strlen(const char* str), void foo(void)
inline bool extractFunctionSignature(String &decl, FFIHeaderSignature &sig) {
    const char *s = decl.data;
    const int n = decl.length;
    if (!s || n == 0) return false;
    decl = decl.trim();
    if (decl.startsWith("//")) return false;
    if (decl.startsWith("return")) return false;
    if (decl.startsWith("/*")) return false; // todo
    if (decl.startsWith("*")) return false; // todo /* … */
    if (not decl.contains('(')) return false;
    if (not decl.contains(')')) return false;
    decl = decl.to("//");
    if (decl.startsWith("extern \"c\"")) decl = decl.substring(10).trim();
    if (decl.startsWith("extern ")) decl = decl.substring(7).trim();
    if (decl.startsWith("const ")) decl = decl.substring(6).trim();
    if (decl.startsWith("static ")) decl = decl.substring(7).trim();
    if (decl.startsWith("inline ")) decl = decl.substring(7).trim();
    if (decl.contains('-')) return false;
    if (decl.contains('[')) return false;
    if (decl.contains('#')) return false;
    if (decl.contains(':') and not decl.contains("::")) return false;
    if (decl.contains(';')) // remove trailing semicolon if present
        s = decl.substring(0, decl.lastIndexOf(";")).data;
    // if(decl.contains("WindowShouldClose"))
        // print("Debug decl: "s + decl);

    // ---- find parentheses ---------------------------------------------------
    int lpar = -1, rpar = -1;
    for (int i = 0; i < n; ++i) {
        if (s[i] == '(') {
            lpar = i;
            break;
        }
    }
    if (lpar < 0) return false;

    for (int i = lpar + 1; i < n; ++i) {
        if (s[i] == ')') {
            rpar = i;
            break;
        }
    }
    if (rpar < 0) return false;

    // ---- function name ------------------------------------------------------
    int name_end = lpar - 1;
    while (name_end >= 0 && isspace((unsigned char) s[name_end])) name_end--;
    if (name_end < 0) return false;

    int name_start = name_end;
    while (name_start > 0) {
        char c = s[name_start - 1];
        if (!(isalnum((unsigned char) c) || c == '_')) break;
        name_start--;
    }
    sig.name = String((char *) s + name_start, name_end - name_start + 1, false);
    // if(sig.name!="SetTargetFPS") return false; // DEBUG!!!
    // if(sig.name!="BeginDrawing") return false; // DEBUG!!!

    // ---- return type --------------------------------------------------------
    int ret_end = name_start - 1;
    while (ret_end >= 0 && isspace((unsigned char) s[ret_end])) ret_end--;
    if (ret_end < 0) return false;

    int ret_start = 0;
    while (ret_start <= ret_end && isspace((unsigned char) s[ret_start]))
        ret_start++;

    String ret((char *) s + ret_start, ret_end - ret_start + 1, false);
    ret = ret.trim();

    // normalize: keep base type + pointer, drop qualifiers
    if (ret.contains(" ")) {
        auto parts = ret.split(" ");
        String last = parts.last();
        if (last == "*")
            ret = parts[parts.size() - 2] + "*";
        else
            ret = last;
    }
    sig.return_type = ret;

    // ---- parameters ---------------------------------------------------------
    sig.param_types.clear();
    const int params_len = rpar - (lpar + 1);
    if (params_len <= 0) return true;
    const char *p = s + lpar + 1;
    int tok_start = 0;
    for (int i = 0; i <= params_len; ++i) {
        if (i < params_len && p[i] != ',') continue;
        int len = i - tok_start;
        const char *tok = p + tok_start;
        while (len > 0 && isspace((unsigned char) *tok)) {
            tok++;
            len--;
        }
        while (len > 0 && isspace((unsigned char) tok[len - 1])) len--;
        tok_start = i + 1;
        if (len == 0) continue;
        String param((char *) tok, len, false);
        param = param.trim();
        if (param == "void") continue;
        if (param.startsWith("const "))
            param = param.substring(6).trim();
        // extract type (base + optional '*')
        int last_star = -1;
        for (int j = 0; j < param.length; ++j)
            if (param.data[j] == '*') last_star = j;
        String type;
        if (last_star >= 0) {
            type = String(param.data, last_star + 1, false);
        } else {
            int k = 0;
            while (k < param.length &&
                   !isspace((unsigned char) param.data[k]))
                k++;
            type = String(param.data, k, false);
        }
        if (type.length > 0)
            sig.param_types.add(type);
        auto param_name = param.replace(type.data, "").trim();
        sig.param_names.add(param_name);
    }
    return true;
}


// Parse a C header file and extract all function signatures
inline List<FFIHeaderSignature> parseHeaderFile(const String &header_path, const String &library) {
    List<FFIHeaderSignature> signatures;

    // Read header file
    String content = readFile(header_path.data);
    if (content.empty()) {
        warn("Failed to read header file: "s + header_path);
        return signatures;
    }
    for (String line: content.split("\n")) {
        String trimmed = line.trim();
        String decl = trimmed;
        FFIHeaderSignature sig;
        sig.library = library;
        if (extractFunctionSignature(decl, sig))
            signatures.add(sig);
    }

    return signatures;
}
