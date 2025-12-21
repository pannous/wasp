// FFI Function Signature Database - Reuses existing Type and Signature classes
#pragma once

#include "String.h"
#include "Code.h"  // For Signature, Arg, Type
#include <cstring>

#ifdef NATIVE_FFI
#include "ffi_inspector.h"         // Dynamic signature inspection via dlsym
#include "ffi_header_parser.h"
#include "ffi_library_headers.h"
#include <fstream>
#include <sys/stat.h>
#endif

// Helper: Manual string comparison (workaround for String == operator issue)
static inline bool str_eq(const String& s, const char* literal) {
    int len = strlen(literal);
    if (s.length != len) return false;
    for (int i = 0; i < len; i++) {
        if (s.data[i] != literal[i]) return false;
    }
    return true;
}

#ifdef NATIVE_FFI
// Try to detect signature from C header files
inline bool detect_signature_from_headers(const String& func_name, const String& lib_name, Signature& sig) {
    // Get header files for this library from the mapping
    List<String>* header_paths = get_library_headers(lib_name);

    if (!header_paths) {
        // No known headers for this library
        return false;
    }

    // Try each header file for this library
    for (int i = 0; i < header_paths->size(); i++) {
        const char* header_path = (*header_paths)[i].data;

        struct stat buffer;
        if (stat(header_path, &buffer) == 0) {
            // Header file exists, try to read it
            std::ifstream file(header_path);
            if (file.is_open()) {
                std::string line;
                std::string accumulated;
                bool in_declaration = false;

                // Look for function declaration matching func_name
                while (std::getline(file, line)) {
                    // Skip comments and non-declaration lines
                    size_t comment_pos = line.find("//");
                    if (comment_pos != std::string::npos) {
                        line = line.substr(0, comment_pos);
                    }
                    if (line.find("/*") != std::string::npos || line.find("*/") != std::string::npos) {
                        continue;
                    }
                    if (line.find("*") == 0 || line.find(" *") != std::string::npos) {
                        continue;
                    }

                    // Check if this line starts a declaration with our function
                    if (line.find(func_name.data) != std::string::npos &&
                        (line.find("extern") != std::string::npos ||
                         line.find("DECLSPEC") != std::string::npos)) {
                        in_declaration = true;
                        accumulated = line;
                    } else if (in_declaration) {
                        // Trim leading/trailing whitespace from line
                        std::string trimmed = line;
                        size_t start = trimmed.find_first_not_of(" \t\r\n");
                        if (start != std::string::npos) {
                            trimmed = trimmed.substr(start);
                        }
                        accumulated += " " + trimmed;
                    }

                    // Check if declaration is complete (has semicolon)
                    if (in_declaration && accumulated.find(';') != std::string::npos) {
                        // Clean up the declaration
                        std::string clean_line = accumulated;

                        // Remove SDL-specific macros
                        size_t pos;
                        while ((pos = clean_line.find("DECLSPEC ")) != std::string::npos) {
                            clean_line.erase(pos, 9);
                        }
                        while ((pos = clean_line.find("SDLCALL ")) != std::string::npos) {
                            clean_line.erase(pos, 8);
                        }
                        while ((pos = clean_line.find("extern ")) != std::string::npos) {
                            clean_line.erase(pos, 7);
                        }

                        String decl = clean_line.c_str();

                        FFIHeaderSignature ffi_sig;
                        ffi_sig.library = lib_name;
                        if (extractFunctionSignatureFromString(decl, ffi_sig) &&
                            str_eq(ffi_sig.name, func_name.data)) {
                            // Successfully extracted signature!
                            sig.parameters.clear();
                            sig.return_types.clear();

                            // Convert to Signature format
                            for (const String& param_type : ffi_sig.param_types) {
                                if (str_eq(param_type, "void")) {
                                    continue; // Skip void parameters (e.g., func(void))
                                }
                                Arg param;
                                param.type = mapCTypeToWasp(param_type);
                                sig.parameters.add(param);
                            }

                            Type ret_type = mapCTypeToWasp(ffi_sig.return_type);
                            if (ret_type != nils && ret_type != voids) {
                                sig.return_types.add(ret_type);
                            }

                            file.close();
                            return true;
                        }

                        // Reset for next declaration
                        in_declaration = false;
                        accumulated.clear();
                    }
                }
                file.close();
            }
        }
    }
    return false;
}
#endif

// Detect function signature based on function name and library
// Populates the provided Signature object with parameter and return types
inline void detect_ffi_signature(const String& function_name, const String& library_name, Signature& sig) {
    sig.parameters.clear();
    sig.return_types.clear();

#ifdef NATIVE_FFI
    // Strategy 1: Dynamic inspection via dlsym (BEST - uses actual library!)
    if (inspect_ffi_signature(library_name, function_name, sig)) {
        trace("FFI signature detected via dynamic inspection: "s + function_name + " from " + library_name);
        return;  // Success! No need for other methods
    }

    // Strategy 2: Header-based reflection
    Signature header_sig;
    bool found_in_header = detect_signature_from_headers(function_name, library_name, header_sig);
    Signature hardcoded_sig; // For verification
#endif

    // Build hardcoded signature for comparison/fallback
    // Helper to add parameter
    auto add_param = [&](Type type) {
        Arg param;
        param.type = type;
        sig.parameters.add(param);
    };

    // Minimal fallback signatures (only used when header parsing unavailable)
    // Most signatures now come from header files via detect_signature_from_headers()

    // Simple library-specific defaults
    if (library_name.data[0] == 'm' && library_name.length == 1) {
        // Two-parameter math functions (common fallback)
        if (str_eq(function_name, "pow") || str_eq(function_name, "fmod") ||
            str_eq(function_name, "fmax") || str_eq(function_name, "fmin") ||
            str_eq(function_name, "atan2") || str_eq(function_name, "hypot")) {
            add_param(float64t);
            add_param(float64t);
            sig.return_types.add(float64t);
        }
        else {
            // Math library: default to double -> double
            add_param(float64t);
            sig.return_types.add(float64t);
        }
    }
    else if (library_name.data[0] == 'c' && library_name.length == 1) {
        // Common string functions that take char* parameter
        if (str_eq(function_name, "strlen") || str_eq(function_name, "atoi") ||
            str_eq(function_name, "atof") || str_eq(function_name, "atol")) {
            add_param(charp);
            sig.return_types.add(str_eq(function_name, "atof") ? float64t : int32t);
        }
        // String comparison functions (two char* params)
        else if (str_eq(function_name, "strcmp") || str_eq(function_name, "strcoll")) {
            add_param(charp);
            add_param(charp);
            sig.return_types.add(int32t);
        }
        else {
            // C library: default to int -> int
            add_param(int32t);
            sig.return_types.add(int32t);
        }
    }
    else if (str_eq(library_name, "SDL2")) {
        // Minimal SDL fallback signatures (header parsing should handle most)
        if (str_eq(function_name, "SDL_Init")) {
            add_param(int32t); sig.return_types.add(int32t);
        }
        else if (str_eq(function_name, "SDL_Quit")) {
            // void
        }
        else if (str_eq(function_name, "SDL_GetTicks")) {
            sig.return_types.add(int32t);
        }
        else if (str_eq(function_name, "SDL_CreateWindow")) {
            add_param(charp); add_param(int32t); add_param(int32t);
            add_param(int32t); add_param(int32t); add_param(int32t);
            sig.return_types.add(i64);
        }
        else if (str_eq(function_name, "SDL_CreateRenderer")) {
            add_param(i64); add_param(int32t); add_param(int32t);
            sig.return_types.add(i64);
        }
        else if (str_eq(function_name, "SDL_DestroyWindow") || str_eq(function_name, "SDL_DestroyRenderer") ||
                 str_eq(function_name, "SDL_RenderPresent")) {
            add_param(i64); // void return
        }
        else if (str_eq(function_name, "SDL_SetRenderDrawColor")) {
            add_param(i64); add_param(int32t); add_param(int32t);
            add_param(int32t); add_param(int32t);
            sig.return_types.add(int32t);
        }
        else if (str_eq(function_name, "SDL_RenderClear")) {
            add_param(i64); sig.return_types.add(int32t);
        }
        else if (str_eq(function_name, "SDL_Delay")) {
            add_param(int32t); // void return
        }
        else {
            // SDL default
            add_param(int32t);
            sig.return_types.add(int32t);
        }
    }
    else if (str_eq(library_name, "raylib")) {
        // Minimal raylib fallbacks
        if (str_eq(function_name, "InitWindow")) {
            add_param(int32t); add_param(int32t); add_param(charp);
        }
        else if (str_eq(function_name, "CloseWindow") || str_eq(function_name, "BeginDrawing") ||
                 str_eq(function_name, "EndDrawing")) {
            // void -> void
        }
        else if (str_eq(function_name, "WindowShouldClose") || str_eq(function_name, "IsWindowReady")) {
            sig.return_types.add(int32t);
        }
        else if (str_eq(function_name, "SetTargetFPS") || str_eq(function_name, "ClearBackground")) {
            add_param(int32t);
        }
        else if (str_eq(function_name, "DrawCircle")) {
            // void DrawCircle(int centerX, int centerY, float radius, Color color)
            add_param(int32t); add_param(int32t); add_param(float32t); add_param(int32t);
        }
        else if (str_eq(function_name, "DrawRectangle")) {
            // void DrawRectangle(int posX, int posY, int width, int height, Color color)
            add_param(int32t); add_param(int32t); add_param(int32t); add_param(int32t); add_param(int32t);
        }
        else if (str_eq(function_name, "DrawText")) {
            // void DrawText(const char *text, int posX, int posY, int fontSize, Color color)
            add_param(charp); add_param(int32t); add_param(int32t); add_param(int32t); add_param(int32t);
        }
        else if (str_eq(function_name, "GetFrameTime")) {
            sig.return_types.add(float32t);
        }
        else if (str_eq(function_name, "GetTime")) {
            sig.return_types.add(float64t);
        }
        else {
            // Raylib default: int -> void (most drawing functions)
            add_param(int32t);
        }
    }
    else {
        // Unknown library: default to double -> double
        add_param(float64t);
        sig.return_types.add(float64t);
    }

#ifdef NATIVE_FFI
    // Verification step: compare header-parsed signature with hardcoded
    if (found_in_header) {
        hardcoded_sig = sig; // Save the hardcoded signature we just built

        // Compare parameter counts
        bool matches = (header_sig.parameters.size() == hardcoded_sig.parameters.size());

        // Compare parameter types
        if (matches) {
            for (size_t i = 0; i < header_sig.parameters.size(); i++) {
                if (header_sig.parameters[i].type != hardcoded_sig.parameters[i].type) {
                    matches = false;
                    break;
                }
            }
        }

        // Compare return types
        if (matches) {
            matches = (header_sig.return_types.size() == hardcoded_sig.return_types.size());
            if (matches) {
                for (size_t i = 0; i < header_sig.return_types.size(); i++) {
                    if (header_sig.return_types[i] != hardcoded_sig.return_types[i]) {
                        matches = false;
                        break;
                    }
                }
            }
        }

        if (!matches) {
            printf("⚠️  Signature mismatch for %s from '%s':\n", function_name.data, library_name.data);
            printf("   Header: (");
            for (size_t i = 0; i < header_sig.parameters.size(); i++) {
                printf("%s%s", typeName(header_sig.parameters[i].type),
                       i < header_sig.parameters.size() - 1 ? ", " : "");
            }
            printf(") -> ");
            if (header_sig.return_types.size() > 0) {
                printf("%s\n", typeName(header_sig.return_types[0]));
            } else {
                printf("void\n");
            }
            printf("   Hardcoded: (");
            for (size_t i = 0; i < hardcoded_sig.parameters.size(); i++) {
                printf("%s%s", typeName(hardcoded_sig.parameters[i].type),
                       i < hardcoded_sig.parameters.size() - 1 ? ", " : "");
            }
            printf(") -> ");
            if (hardcoded_sig.return_types.size() > 0) {
                printf("%s\n", typeName(hardcoded_sig.return_types[0]));
            } else {
                printf("void\n");
            }
        }

        // Use header signature (it's more accurate)
        sig = header_sig;
        return;
    }
#endif
}
