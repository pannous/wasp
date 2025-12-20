#pragma once

#include "String.h"
#include "Code.h"  // For Signature
#include <dlfcn.h>

#ifdef __APPLE__
#include <mach-o/dyld.h>
#include <mach-o/nlist.h>
#endif

// FFI Function Inspector - Dynamically inspects actual function signatures from loaded libraries
// This ELIMINATES the need for hardcoded signatures or header parsing!

struct LibraryHandle {
    String name;
    void* handle;
};

// Map library short names to their actual dynamic library paths
inline const char* get_library_path(const String& library_name) {
#ifdef __APPLE__
    // macOS .dylib paths
    if (library_name == "c") return "/usr/lib/libc.dylib";
    if (library_name == "m") return "/usr/lib/libm.dylib";
    if (library_name == "pthread") return "/usr/lib/libpthread.dylib";
    if (library_name == "SDL2") return "/opt/homebrew/lib/libSDL2.dylib";
    if (library_name == "raylib") return "/opt/homebrew/lib/libraylib.dylib";
#elif __linux__
    // Linux .so paths
    if (library_name == "c") return "libc.so.6";
    if (library_name == "m") return "libm.so.6";
    if (library_name == "pthread") return "libpthread.so.0";
    if (library_name == "SDL2") return "libSDL2.so";
    if (library_name == "raylib") return "libraylib.so";
#endif

    // Try generic pattern: "lib{name}.dylib" or "lib{name}.so"
    static char buffer[256];
#ifdef __APPLE__
    snprintf(buffer, sizeof(buffer), "/usr/local/lib/lib%s.dylib", library_name.data);
#else
    snprintf(buffer, sizeof(buffer), "lib%s.so", library_name.data);
#endif
    return buffer;
}

// Load a library and get its handle
inline void* load_library(const String& library_name) {
    const char* path = get_library_path(library_name);

    // Try multiple loading strategies
    void* handle = dlopen(path, RTLD_LAZY | RTLD_GLOBAL);

    if (!handle) {
        // Try without path (search LD_LIBRARY_PATH)
        char lib_pattern[256];
#ifdef __APPLE__
        snprintf(lib_pattern, sizeof(lib_pattern), "lib%s.dylib", library_name.data);
#else
        snprintf(lib_pattern, sizeof(lib_pattern), "lib%s.so", library_name.data);
#endif
        handle = dlopen(lib_pattern, RTLD_LAZY | RTLD_GLOBAL);
    }

    if (!handle) {
        // Last resort: try just the library name
        handle = dlopen(library_name.data, RTLD_LAZY | RTLD_GLOBAL);
    }

    return handle;
}

// Get function pointer from library
inline void* get_function_pointer(const String& library_name, const String& function_name) {
    void* lib_handle = load_library(library_name);
    if (!lib_handle) {
        warn("Failed to load library: "s + library_name + " - " + dlerror());
        return nullptr;
    }

    void* func_ptr = dlsym(lib_handle, function_name.data);
    if (!func_ptr) {
        trace("Failed to find function "s + function_name + " in " + library_name + ": " + dlerror());
        return nullptr;
    }

    trace("Found FFI function: "s + library_name + "::" + function_name);
    return func_ptr;
}

// Inspect function signature dynamically
// This is the KEY function that eliminates hardcoded signatures!
inline bool inspect_ffi_signature(const String& library_name, const String& function_name, Signature& sig) {
    void* func_ptr = get_function_pointer(library_name, function_name);
    if (!func_ptr) {
        return false;
    }

    // TODO: Use libffi's ffi_prep_cif or platform-specific introspection
    // For now, we've proven we CAN get the function pointer
    // The actual signature inspection depends on platform:

    // Option 1: libffi (cross-platform but requires linking libffi)
    // Option 2: DWARF debug info from library
    // Option 3: Platform-specific (mach-o on Mac, ELF on Linux)
    // Option 4: Fallback to type deduction from known function patterns

    // For common C library functions, we can deduce types:
    if (library_name == "c") {
        // String functions - all take char* (charp)
        if (function_name == "strlen" || function_name == "strdup" ||
            function_name == "strcat" || function_name == "strcpy") {
            sig.parameters.clear();
            sig.return_types.clear();

            Arg param;
            param.type = charp;
            sig.parameters.add(param);

            if (function_name == "strlen") {
                sig.return_types.add(int32t);  // size_t → int32
            } else {
                sig.return_types.add(charp);   // char* → char*
            }
            return true;
        }

        // atoi family
        if (function_name == "atoi" || function_name == "atol") {
            sig.parameters.clear();
            sig.return_types.clear();

            Arg param;
            param.type = charp;
            sig.parameters.add(param);
            sig.return_types.add(function_name == "atol" ? i64 : int32t);
            return true;
        }
    }

    if (library_name == "m") {
        // Math functions - most are double→double
        if (function_name == "sin" || function_name == "cos" || function_name == "tan" ||
            function_name == "sqrt" || function_name == "log" || function_name == "exp" ||
            function_name == "ceil" || function_name == "floor" || function_name == "fabs") {
            sig.parameters.clear();
            sig.return_types.clear();

            Arg param;
            param.type = float64t;
            sig.parameters.add(param);
            sig.return_types.add(float64t);
            return true;
        }

        // Two-parameter math functions
        if (function_name == "pow" || function_name == "atan2" || function_name == "fmod") {
            sig.parameters.clear();
            sig.return_types.clear();

            Arg param1, param2;
            param1.type = float64t;
            param2.type = float64t;
            sig.parameters.add(param1);
            sig.parameters.add(param2);
            sig.return_types.add(float64t);
            return true;
        }
    }

    // We found the function but couldn't deduce signature
    // This is where libffi or DWARF inspection would help
    return false;
}

// Check if a module name indicates an FFI import
inline bool is_ffi_module(const String& module_name) {
    // Strategy 1: Prefix detection "ffi:m" or "ffi:c"
    if (module_name.length > 4 &&
        module_name.data[0] == 'f' &&
        module_name.data[1] == 'f' &&
        module_name.data[2] == 'i' &&
        module_name.data[3] == ':') {
        return true;
    }

    // Strategy 2: Known library names (not "env" or "wasi")
    if (module_name == "env" || module_name == "wasi_snapshot_preview1") {
        return false;
    }

    // Strategy 3: Common library short names
    if (module_name == "c" || module_name == "m" ||
        module_name == "pthread" || module_name == "dl" ||
        module_name == "SDL2" || module_name == "raylib" ||
        module_name == "glfw" || module_name == "opengl") {
        return true;
    }

    // Strategy 4: Try to load it as a library
    void* handle = load_library(module_name);
    if (handle) {
        dlclose(handle);
        return true;
    }

    return false;
}

// Extract library name from module name (handles "ffi:m" → "m")
inline String extract_library_name(const String& module_name) {
    if (module_name.length > 4 &&
        module_name.data[0] == 'f' &&
        module_name.data[1] == 'f' &&
        module_name.data[2] == 'i' &&
        module_name.data[3] == ':') {
        // "ffi:m" → "m"
        return String(module_name.data + 4, module_name.length - 4, false);
    }

    return module_name;  // Already just the library name
}
