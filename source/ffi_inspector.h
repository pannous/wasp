#pragma once

#include "String.h"
#include "Code.h"  // For Signature
#include "Context.h" // for Module
#include <dlfcn.h>

#include "ffi_header_parser.h"
#include "ffi_library_headers.h"

#ifdef __APPLE__
#include <mach-o/dyld.h>
#include <mach-o/nlist.h>
#endif

// FFI Function Inspector - Dynamically inspects actual function signatures from loaded libraries
// This ELIMINATES the need for hardcoded signatures or header parsing!

struct LibraryHandle {
    String name;
    void *handle;
};

// Map library short names to their actual dynamic library paths
inline const char *get_library_path(const String &library_name) {
    // todo just check common paths for lib{name}.so / .dylib !
#ifdef __APPLE__
    // macOS .dylib paths
    if (library_name == "c") return "/usr/lib/libc.dylib";
    if (library_name == "m") return "/usr/lib/libm.dylib";
    if (library_name == "pthread") return "/usr/lib/libpthread.dylib";
    if (library_name == "SDL2") return "/opt/homebrew/lib/libSDL2.dylib";
    if (library_name == "raylib")
        return "/opt/homebrew/lib/libraylib.dylib";
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

// Load a library and get its handle, competes with .wasm libraries
inline void *find_native_library(const String &library_name) {
    const char *path = get_library_path(library_name);

    // Try multiple loading strategies
    void *handle = dlopen(path, RTLD_LAZY | RTLD_GLOBAL);
    if (handle) return handle;

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

    if (handle) return handle;
    if (!handle) {
        // Last resort: try just the library name
        handle = dlopen(library_name.data, RTLD_LAZY | RTLD_GLOBAL);
    }
    return handle;
}


// Get function pointer from library
inline void *get_function_pointer(const String &library_name, const String &function_name) {
    void *lib_handle = find_native_library(library_name);
    if (!lib_handle) {
        warn("Failed to load library: "s + library_name + " - " + dlerror());
        return nullptr;
    }

    void *func_ptr = dlsym(lib_handle, function_name.data);
    if (!func_ptr) {
        trace("Failed to find function "s + function_name + " in " + library_name + ": " + dlerror());
        return nullptr;
    }

    trace("Found FFI function: "s + library_name + "::" + function_name);
    return func_ptr;
}


// Check if a module name indicates an FFI import
inline bool is_ffi_module(const String &module_name) {
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
    void *handle = find_native_library(module_name);
    if (handle) {
        dlclose(handle);
        return true;
    }

    return false;
}

// Extract library name from module name (handles "ffi:m" → "m")
inline String extract_library_name(const String &module_name) {
    if (module_name.length > 4 &&
        module_name.data[0] == 'f' &&
        module_name.data[1] == 'f' &&
        module_name.data[2] == 'i' &&
        module_name.data[3] == ':') {
        // "ffi:m" → "m"
        return String(module_name.data + 4, module_name.length - 4, false);
    }

    return module_name; // Already just the library name
}

#ifdef __APPLE__
#include <mach-o/dyld.h>
#include <mach-o/nlist.h>
#include <mach-o/loader.h>

// Enumerate all exported functions from a loaded library on macOS
inline void enumerate_library_functions_macos(void *handle, Map<String, Function> &functions,
                                              const String &library_name) {
    // Get the library image index from handle
    Dl_info info;
    if (dladdr(handle, &info) == 0) {
        warn("Could not get library info for "s + library_name);// todo why not?
        return;
    }

    const struct mach_header_64 *header = (const struct mach_header_64 *) info.dli_fbase;
    if (!header) return;

    // Parse Mach-O to find symbol table
    struct load_command *cmd = (struct load_command *) ((char *) header + sizeof(struct mach_header_64));
    struct symtab_command *symtab = nullptr;

    for (uint32_t i = 0; i < header->ncmds; i++) {
        if (cmd->cmd == LC_SYMTAB) {
            symtab = (struct symtab_command *) cmd;
            break;
        }
        cmd = (struct load_command *) ((char *) cmd + cmd->cmdsize);
    }

    if (!symtab) {
        trace("No symbol table found in "s + library_name);
        return;
    }

    // Access symbol table
    struct nlist_64 *symbols = (struct nlist_64 *) ((char *) header + symtab->symoff);
    char *string_table = (char *) header + symtab->stroff;

    int func_count = 0;
    for (uint32_t i = 0; i < symtab->nsyms; i++) {
        struct nlist_64 *sym = &symbols[i];

        // Check if it's an external defined function symbol
        if ((sym->n_type & N_TYPE) == N_SECT && // Defined in a section
            (sym->n_type & N_EXT) && // External symbol
            sym->n_value != 0) {
            // Has an address

            const char *name = string_table + sym->n_un.n_strx;
            if (name && name[0] == '_') name++; // Skip leading underscore on macOS

            // Skip compiler-generated or internal symbols
            if (name[0] == '.' || name[0] == 'L' || name[0] == 'l') continue;
            if (strstr(name, "$") || strstr(name, "OUTLINED")) continue;

            Function func;
            func.name = String(name);
            func.is_ffi = true;
            func.ffi_library = library_name;

            // Try to inspect signature
            void detect_ffi_signature(const String& function_name, const String& library_name, Signature& sig);
            detect_ffi_signature(library_name, func.name, func.signature);

            functions.add(func.name, func);
            func_count++;
        }
    }

    trace("Enumerated "s + formatLong(func_count) + " functions from " + library_name);
}
#endif

#ifdef __linux__
#include <link.h>
#include <elf.h>

// Enumerate all exported functions from a loaded library on Linux
inline void enumerate_library_functions_linux(void *handle, Map<String, Function> &functions,
                                              const String &library_name) {
    struct link_map *map;
    if (dlinfo(handle, RTLD_DI_LINKMAP, &map) != 0) {
        warn("Could not get link map for "s + library_name);
        return;
    }

    // Read ELF symbol table
    // This is simplified - full implementation would parse ELF headers
    trace("Linux ELF enumeration not yet implemented for "s + library_name);
    // TODO: Parse ELF .dynsym section to enumerate symbols
}
#endif

// Enumerate ALL functions from a library (called when importing entire module)
// e.g., "import c" or "use m"
inline void enumerate_library_functions(const String &library_name, Map<String, Function> &functions) {
    void *handle = find_native_library(library_name);
    if (!handle) {
        warn("Failed to load library for enumeration: "s + library_name);
        return;
    }

    trace("Enumerating all functions from library: "s + library_name);

#ifdef __APPLE__
    enumerate_library_functions_macos(handle, functions, library_name);
#elif __linux__
    enumerate_library_functions_linux(handle, functions, library_name);
#else
    warn("Function enumeration not implemented for this platform");
#endif

    // Don't close the handle - we need it for future dlsym calls
    // dlclose(handle);
}


inline bool is_native_library(const String &library_name) {
    void *handle = find_native_library(library_name);
    if (handle) {
        dlclose(handle); // Close immediately, just checking
        return true;
    }
    return false;
}

inline Signature& convert_ffi_signature(FFIHeaderSignature &ffi_sig) {
    Signature& sig = *new Signature();
    sig.abi = ABI::native;
    for (const String &param_type: ffi_sig.param_types) {
        if (eq(param_type, "void")) {
            continue; // Skip void parameters (e.g., func(void))
        }
        Arg param;
        param.type = mapCTypeToWasp(param_type);
        long index = &param_type - &ffi_sig.param_types[0];
        param.name =  ffi_sig.param_names[index];
        // param.modifiers TODO
        sig.parameters.add(param);
    }

    Type ret_type = mapCTypeToWasp(ffi_sig.return_type);
    if (ret_type != nils && ret_type != voids) {
        sig.return_types.add(ret_type);
    }
    return sig;
}

inline void add_ffi_signature(Module *modul, FFIHeaderSignature ffi_sig) {
    if (!modul->functions.has(ffi_sig.name)) {
        Function func;
        func.name = ffi_sig.name;
        func.is_ffi = true;
        func.module = modul;
        func.ffi_library = modul->name;
        func.signature = convert_ffi_signature(ffi_sig);// ffi_sig.signature;
        modul->functions.add(func.name, func);
    } else {
        // Merge signature info
        Function &existing_func = modul->functions[ffi_sig.name];
        if(ffi_sig.name=="atof")
            warn("Override with header info "s % ffi_sig.name);
            // print("Debug atof merge\n");
        existing_func.signature =   convert_ffi_signature(ffi_sig); // ffi_sig.signature; // Override with header info
    }
}

inline void fixNativeSignatures(Module & modul) {
    if(modul.functions.has("InitWindow"s))
        modul.functions["InitWindow"s].signature.parameters[2].type = charp;
}

extern Map<int64, Module *> module_cache;
inline Module *loadNativeLibrary(String library) {
    if(library.empty()) return 0;
    for(Module *m:libraries)
        if(m->name==library)
            return m; // already loaded
    if (module_cache.has(library.hash()))
        return module_cache[library.hash()];
    Module *modul = new Module();
    modul->name = library;
    modul->is_native_library = true;
    enumerate_library_functions(library, modul->functions); // via FFI, next via .h parsing!
    auto headers = get_library_header_files(library);
    for (String header: *headers) {
        if(not fileExists(header)) {
            warn("Header file %s for library %s does not exist!\n"s % header % library);
            continue;
        }
        print("Parsing header file %s for library %s\n"s % header % library);
        List<FFIHeaderSignature> sigs = parseHeaderFile(header, library);
        for (FFIHeaderSignature &ffi_sig: sigs) {
            // print("  Found FFI signature: %s %s\n"s % ffi_sig.name % ffi_sig.raw);
            // if(ffi_sig.name=="BeginDrawing") // debug
                // warn("Adding header signature for %s\n"s % ffi_sig.name);
            add_ffi_signature(modul, ffi_sig);
        }
    }
    fixNativeSignatures(*modul);
    modul->total_func_count=modul->functions.size();
    libraries.add(modul); // current
    module_cache[library.hash()] = modul; // cache it globally
    return modul;
}


// Inspect function signature dynamically
inline bool inspect_ffi_signature(const String &library_name, const String &function_name, Signature &sig) {
    void *func_ptr = get_function_pointer(library_name, function_name);
    if (!func_ptr) {
        return false;
    }
    // TODO: Use libffi's ffi_prep_cif or platform-specific introspection
    // The actual signature inspection depends on platform:
    // Option 1: libffi (cross-platform but requires linking libffi)
    // Option 2: DWARF debug info from library
    // Option 3: Platform-specific (mach-o on Mac, ELF on Linux)
    return false; // Placeholder until implemented, we fall back to header parsing
}



// Detect function signature based on function name and library
// Populates the provided Signature object with parameter and return types
inline void detect_ffi_signature(const String& function_name, const String& library_name, Signature& sig) {
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
        trace("FFI signature for: "s + function_name + " obtained from header files of module " + library_name);
    }
}
