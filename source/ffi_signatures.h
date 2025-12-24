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


#ifdef NATIVE_FFI
// Try to detect signature from C header files
inline bool old_detect_signature_from_headers(const String& func_name, const String& lib_name, Signature& sig) {
    // Get header files for this library from the mapping
    List<String>* header_paths = get_library_header_files(lib_name);

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
                        if(extractFunctionSignature(decl, ffi_sig) && eq(ffi_sig.name, func_name.data)) {
                            file.close();
                            sig=convert_ffi_signature(ffi_sig);
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
#ifdef NATIVE_FFI
    // Strategy 1: Dynamic inspection via dlsym (BEST - uses actual library!)
    // Placeholder until implemented, we fall back to header parsing
    if (inspect_ffi_signature(library_name, function_name, sig)) {
        trace("FFI signature detected via dynamic inspection: "s + function_name + " from " + library_name);
        return;  // Success! No need for other methods
    }

    // Strategy 2: COMPLETE Header-based reflection
    Module * mod = loadNativeLibrary(library_name);
    if (mod && mod->functions.has(function_name)) {
        sig = mod->functions[function_name].signature;
        trace("FFI signature obtained from loaded native library module: "s + function_name + " from " + library_name);
        return;
    }
    // Strategy 3: OLD redundant Header-based reflection
    // Ensure headers are loaded
    // bool found_in_header = old_detect_signature_from_headers(function_name, library_name, sig);
    // trace("found %s in_header: %d\n"s % function_name % found_in_header);
#endif
}
