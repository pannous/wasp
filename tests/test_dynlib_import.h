#include <dlfcn.h>
#ifdef __APPLE__
#include <mach-o/dyld.h>
#include <mach-o/nlist.h>
#include <mach-o/loader.h>
#endif

// Helper to list exported symbols from a dynamic library
static void list_library_symbols(const char* lib_path) {
    print("Symbols in library: ");
    println(lib_path);

    void* handle = dlopen(lib_path, RTLD_LAZY | RTLD_LOCAL);
    if (!handle) {
        print("Failed to load library: ");
        println(dlerror());
        return;
    }

    // Note: Standard dlopen/dlsym doesn't provide symbol enumeration
    // On macOS, we'd need to parse the Mach-O binary or use external tools
    // For this demo, we'll try a few known symbols

    const char* known_symbols[] = {
        "sin", "cos", "tan", "sqrt", "pow", "exp", "log",
        "malloc", "free", "printf", "strlen", "strcmp",
        0
    };

    int found_count = 0;
    for (int i = 0; known_symbols[i]; i++) {
        void* sym = dlsym(handle, known_symbols[i]);
        if (sym) {
            print("  ✓ ");
            println(known_symbols[i]);
            found_count++;
        }
    }

    print("Found ");
    print(found_count);
    println(" known symbols");

    dlclose(handle);
}

// Test loading a system library and calling a function
static void test_dynlib_import() {
    tests_executed++;
    println("=== Dynamic Library Import Test ===");

    // Test 1: Load libm (math library) and call sqrt
    {
        println("\n--- Test 1: Load libm and call sqrt ---");

#ifdef __APPLE__
        const char* libm_path = "/usr/lib/libm.dylib";
        // On modern macOS, libraries are in dyld cache
        void* handle = dlopen("libm.dylib", RTLD_LAZY);
#else
        const char* libm_path = "libm.so.6";
        void* handle = dlopen(libm_path, RTLD_LAZY);
#endif

        if (!handle) {
            print("Failed to load libm: ");
            println(dlerror());
        } else {
            println("✓ Successfully loaded libm");

            // Get the sqrt function
            typedef double (*sqrt_func)(double);
            sqrt_func sqrt_ptr = (sqrt_func)dlsym(handle, "sqrt");

            if (!sqrt_ptr) {
                print("Failed to find sqrt: ");
                println(dlerror());
            } else {
                println("✓ Found sqrt function");

                // Call sqrt(16.0) - should return 4.0
                double result = sqrt_ptr(16.0);
                print("sqrt(16.0) = ");
                println(result);
                check_is(result, 4.0);

                // Call sqrt(2.0)
                result = sqrt_ptr(2.0);
                print("sqrt(2.0) = ");
                println(result);
                check_is(result > 1.414 && result < 1.415, true);
            }

            dlclose(handle);
        }
    }

    // Test 2: Load libc and call strlen
    {
        println("\n--- Test 2: Load libc and call strlen ---");

        void* handle = dlopen(0, RTLD_LAZY); // RTLD_DEFAULT - search loaded libraries

        if (!handle) {
            print("Failed to open default namespace: ");
            println(dlerror());
        } else {
            typedef size_t (*strlen_func)(const char*);
            strlen_func strlen_ptr = (strlen_func)dlsym(handle, "strlen");

            if (!strlen_ptr) {
                print("Failed to find strlen: ");
                println(dlerror());
            } else {
                println("✓ Found strlen function");

                const char* test_str = "Hello, World!";
                size_t len = strlen_ptr(test_str);
                print("strlen(\"Hello, World!\") = ");
                println((int)len);
                check_is((int)len, 13);
            }

            // Don't close RTLD_DEFAULT handle
        }
    }

    // Test 3: Enumerate some known symbols from libm
    {
        println("\n--- Test 3: Enumerate known math symbols ---");

#ifdef __APPLE__
        list_library_symbols("libm.dylib");
#else
        list_library_symbols("libm.so.6");
#endif
    }

    // Test 4: Load by different methods
    {
        println("\n--- Test 4: Different loading methods ---");

        // Method 1: By name (searches standard paths)
        void* h1 = dlopen("libm.dylib", RTLD_LAZY);
        if (h1) {
            println("✓ Loaded by name: libm.dylib");
            dlclose(h1);
        }

        // Method 2: RTLD_NEXT (search symbols after current library)
        void* h2 = dlopen("libc.dylib", RTLD_LAZY);
        if (h2) {
            println("✓ Loaded by name: libc.dylib");

            // Try to get malloc
            void* malloc_ptr = dlsym(h2, "malloc");
            if (malloc_ptr) {
                println("✓ Found malloc in libc");
            }

            dlclose(h2);
        }

        // Method 3: RTLD_SELF (search in current process)
        typedef void* (*dlsym_func)(void*, const char*);
        dlsym_func dlsym_ptr = (dlsym_func)dlsym(RTLD_DEFAULT, "dlsym");
        if (dlsym_ptr) {
            println("✓ Found dlsym itself using RTLD_DEFAULT");
        }
    }

    println("\n=== Dynamic Library Import Test Complete ===");
}