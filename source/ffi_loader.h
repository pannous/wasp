// FFI (Foreign Function Interface) loader for dynamic libraries
// Loads native libraries and resolves function symbols
// Only available on Mac/Linux (native builds), not in WASM/Browser

#pragma once

#ifdef NATIVE_FFI

#include "String.h"
#include "Map.h"
#include <dlfcn.h>

struct FFILibrary {
    String name;
    void* handle;

    FFILibrary() : name(""), handle(0) {}
    FFILibrary(String n, void* h) : name(n), handle(h) {}
};

class FFILoader {
public:
    Map<String, FFILibrary> loaded_libraries;

    void* load_library(const String& lib_name) {
        // Check if already loaded
        if (loaded_libraries.has(lib_name)) {
            return loaded_libraries[lib_name].handle;
        }

        // Try different library name formats
        void* handle = 0;

        // Try as-is first
        handle = dlopen(lib_name, RTLD_LAZY | RTLD_GLOBAL);

        // Try with lib prefix and platform extension
        if (!handle) {
#ifdef __APPLE__
            String lib_path = "lib"s + lib_name + ".dylib";
            handle = dlopen(lib_path, RTLD_LAZY | RTLD_GLOBAL);
#else
            String lib_path = "lib"s + lib_name + ".so";
            handle = dlopen(lib_path, RTLD_LAZY | RTLD_GLOBAL);
#endif
        }

        // Try system library path
        if (!handle) {
#ifdef __APPLE__
            String lib_path = "/usr/lib/lib"s + lib_name + ".dylib";
            handle = dlopen(lib_path, RTLD_LAZY | RTLD_GLOBAL);
#else
            String lib_path = "/usr/lib/lib"s + lib_name + ".so";
            handle = dlopen(lib_path, RTLD_LAZY | RTLD_GLOBAL);
#endif
        }

        // Try Homebrew/local library path (macOS) or lib64 (Linux)
        if (!handle) {
#ifdef __APPLE__
            String lib_path = "/usr/local/lib/lib"s + lib_name + ".dylib";
            handle = dlopen(lib_path, RTLD_LAZY | RTLD_GLOBAL);
            if (!handle) {
                // Try Homebrew ARM64 path
                lib_path = "/opt/homebrew/lib/lib"s + lib_name + ".dylib";
                handle = dlopen(lib_path, RTLD_LAZY | RTLD_GLOBAL);
            }
#else
            String lib_path = "/usr/lib/x86_64-linux-gnu/lib"s + lib_name + ".so";
            handle = dlopen(lib_path, RTLD_LAZY | RTLD_GLOBAL);
#endif
        }

        if (handle) {
            loaded_libraries.add(lib_name, FFILibrary(lib_name, handle));
        } else {
            const char* error = dlerror();
            if (error) {
                warn("Failed to load library "s + lib_name + ": " + error);
            }
        }

        return handle;
    }

    void* get_function(const String& lib_name, const String& func_name) {
        void* lib_handle = load_library(lib_name);
        if (!lib_handle) {
            return 0;
        }

        // Clear any existing error
        dlerror();

        void* func_ptr = dlsym(lib_handle, func_name);

        const char* error = dlerror();
        if (error) {
            warn("Failed to find function "s + func_name + " in " + lib_name + ": " + error);
            return 0;
        }

        return func_ptr;
    }

    ~FFILoader() {
        // Close all loaded libraries
        for (int i = 0; i < loaded_libraries.size(); i++) {
            if (loaded_libraries.values[i].handle) {
                dlclose(loaded_libraries.values[i].handle);
            }
        }
    }
};

// Print function for FFILibrary to satisfy Map template requirements
inline void print(const FFILibrary& lib) {
    printf("FFILibrary{%s, %p}", lib.name.data, lib.handle);
}

// Global FFI loader instance
static FFILoader ffi_loader;

#endif // NATIVE_FFI
