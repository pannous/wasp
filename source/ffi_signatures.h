// FFI Function Signature Database - Reuses existing Type and Signature classes
#pragma once

#include "String.h"
#include "Code.h"  // For Signature, Arg, Type
#include <cstring>

#ifdef NATIVE_FFI
#include "ffi_header_parser.h"
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
    // Common header file locations - check specific headers first
    // todo find headers BY NAME:
    /*
    e.g. 
    import raylib =>
    search raylib.h in:
    /usr/include/
    /usr/local/include/
    /opt/homebrew/include/
    …
*/
    const char* header_paths[] = {
        "/opt/homebrew/include/SDL2/SDL_events.h",
        "/usr/local/include/SDL2/SDL_events.h",
        "/usr/include/SDL2/SDL_events.h",
        "/opt/homebrew/include/SDL2/SDL_render.h",
        "/usr/local/include/SDL2/SDL_render.h",
        "/usr/include/SDL2/SDL_render.h",
        "/opt/homebrew/include/SDL2/SDL_timer.h",
        "/usr/local/include/SDL2/SDL_timer.h",
        "/usr/include/SDL2/SDL_timer.h",
        "/opt/homebrew/include/SDL2/SDL.h",
        "/usr/local/include/SDL2/SDL.h",
        "/usr/include/SDL2/SDL.h",
        "/opt/homebrew/include/raylib.h",
        "/usr/local/include/raylib.h",
        "/usr/include/raylib.h",
        "/usr/include/math.h",
        "/usr/include/stdlib.h",
        "/usr/include/string.h",
        nullptr
    };

    // Try to find and read relevant header file based on library name
    String header_content;
    for (int i = 0; header_paths[i] != nullptr; i++) {
        struct stat buffer;
        if (stat(header_paths[i], &buffer) == 0) {
            // Header file exists, try to read it
            std::ifstream file(header_paths[i]);
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
                        Node& parsed = parse(decl, { .data_mode = true });

                        FFIHeaderSignature ffi_sig;
                        ffi_sig.library = lib_name;
                        if (extractFunctionSignature(parsed, ffi_sig) &&
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
inline void detect_ffi_signature(const String& func_name, const String& lib_name, Signature& sig) {
    sig.parameters.clear();
    sig.return_types.clear();

#ifdef NATIVE_FFI
    // Try header-based reflection first
    if (detect_signature_from_headers(func_name, lib_name, sig)) {
        return; // Successfully detected from headers
    }
#endif

    // Fall back to hardcoded signature database
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
    // SDL2 library functions
    else if (str_eq(lib_name, "SDL2")) {
        if (str_eq(func_name, "SDL_Init")) {
            // int SDL_Init(Uint32 flags)
            add_param(int32t);
            sig.return_types.add(int32t);
        }
        else if (str_eq(func_name, "SDL_Quit")) {
            // void SDL_Quit(void) - no return value, leave return_types empty
        }
        else if (str_eq(func_name, "SDL_CreateWindow")) {
            // SDL_Window* SDL_CreateWindow(const char* title, int x, int y, int w, int h, Uint32 flags)
            add_param(charp);     // title
            add_param(int32t);    // x
            add_param(int32t);    // y
            add_param(int32t);    // w
            add_param(int32t);    // h
            add_param(int32t);    // flags
            sig.return_types.add(i64); // window pointer
        }
        else if (str_eq(func_name, "SDL_DestroyWindow")) {
            // void SDL_DestroyWindow(SDL_Window* window) - no return value
            add_param(i64); // window pointer
        }
        else if (str_eq(func_name, "SDL_GetTicks")) {
            // Uint32 SDL_GetTicks(void)
            sig.return_types.add(int32t);
        }
        else if (str_eq(func_name, "SDL_GetError")) {
            // const char* SDL_GetError(void)
            sig.return_types.add(charp);
        }
        // Hardcoded SDL rendering functions (multi-line parser needs work)
        else if (str_eq(func_name, "SDL_CreateRenderer")) {
            // SDL_Renderer* SDL_CreateRenderer(SDL_Window* window, int index, Uint32 flags)
            add_param(i64);    // window
            add_param(int32t); // index
            add_param(int32t); // flags
            sig.return_types.add(i64); // renderer
        }
        else if (str_eq(func_name, "SDL_DestroyRenderer")) {
            // void SDL_DestroyRenderer(SDL_Renderer* renderer)
            add_param(i64);
        }
        else if (str_eq(func_name, "SDL_SetRenderDrawColor")) {
            // int SDL_SetRenderDrawColor(SDL_Renderer* renderer, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
            add_param(i64);    // renderer
            add_param(int32t); // r
            add_param(int32t); // g
            add_param(int32t); // b
            add_param(int32t); // a
            sig.return_types.add(int32t);
        }
        else if (str_eq(func_name, "SDL_RenderClear")) {
            // int SDL_RenderClear(SDL_Renderer* renderer)
            add_param(i64);
            sig.return_types.add(int32t);
        }
        else if (str_eq(func_name, "SDL_RenderPresent")) {
            // void SDL_RenderPresent(SDL_Renderer* renderer)
            add_param(i64);
        }
        else if (str_eq(func_name, "SDL_RenderFillRect")) {
            // int SDL_RenderFillRect(SDL_Renderer* renderer, const SDL_Rect* rect)
            add_param(i64); // renderer
            add_param(i64); // rect (can be NULL)
            sig.return_types.add(int32t);
        }
        else {
            // All other SDL functions: try header reflection, fallback to int->int
            add_param(int32t);
            sig.return_types.add(int32t);
        }
    }
    // Raylib library functions
    else if (str_eq(lib_name, "raylib")) {
        if (str_eq(func_name, "InitWindow")) {
            // void InitWindow(int width, int height, const char* title)
            add_param(int32t);  // width
            add_param(int32t);  // height
            add_param(charp);   // title
        }
        else if (str_eq(func_name, "CloseWindow")) {
            // void CloseWindow(void)
        }
        else if (str_eq(func_name, "WindowShouldClose")) {
            // bool WindowShouldClose(void)
            sig.return_types.add(int32t);
        }
        else if (str_eq(func_name, "IsWindowReady")) {
            // bool IsWindowReady(void)
            sig.return_types.add(int32t);
        }
        else if (str_eq(func_name, "BeginDrawing")) {
            // void BeginDrawing(void)
        }
        else if (str_eq(func_name, "EndDrawing")) {
            // void EndDrawing(void)
        }
        else if (str_eq(func_name, "ClearBackground")) {
            // void ClearBackground(Color color) - Color is RGBA as 4 bytes packed in i32
            add_param(int32t);
        }
        else if (str_eq(func_name, "DrawCircle")) {
            // void DrawCircle(int centerX, int centerY, float radius, Color color)
            add_param(int32t);   // centerX
            add_param(int32t);   // centerY
            add_param(float32t); // radius
            add_param(int32t);   // color (RGBA packed)
        }
        else if (str_eq(func_name, "DrawRectangle")) {
            // void DrawRectangle(int posX, int posY, int width, int height, Color color)
            add_param(int32t);  // posX
            add_param(int32t);  // posY
            add_param(int32t);  // width
            add_param(int32t);  // height
            add_param(int32t);  // color
        }
        else if (str_eq(func_name, "DrawText")) {
            // void DrawText(const char* text, int posX, int posY, int fontSize, Color color)
            add_param(charp);   // text
            add_param(int32t);  // posX
            add_param(int32t);  // posY
            add_param(int32t);  // fontSize
            add_param(int32t);  // color
        }
        else if (str_eq(func_name, "SetTargetFPS")) {
            // void SetTargetFPS(int fps)
            add_param(int32t);
        }
        else if (str_eq(func_name, "GetFrameTime")) {
            // float GetFrameTime(void)
            sig.return_types.add(float32t);
        }
        else if (str_eq(func_name, "GetTime")) {
            // double GetTime(void)
            sig.return_types.add(float64t);
        }
        else {
            // Default raylib function: void -> void
        }
    }
    else {
        // Unknown library - default to double -> double
        add_param(float64t);
        sig.return_types.add(float64t);
    }
}
