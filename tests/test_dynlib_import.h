#include <dlfcn.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#ifdef __APPLE__
#include <mach-o/loader.h>
#include <mach-o/nlist.h>
#include <mach-o/fat.h>
#elif defined(__linux__)
#include <elf.h>
#include <link.h>
#endif

// Generic symbol structure for cross-platform use
struct SymbolInfo {
    String name;
    uint64_t address;
    bool is_function;
};

#ifdef __APPLE__
// Parse Mach-O binary to extract symbols
static List<SymbolInfo> parse_macho_symbols(const char* lib_path) {
    List<SymbolInfo> symbols;

    int fd = open(lib_path, O_RDONLY);
    if (fd < 0) {
        print("Failed to open file: ");
        println(lib_path);
        return symbols;
    }

    struct stat st;
    if (fstat(fd, &st) < 0) {
        close(fd);
        return symbols;
    }

    // Read the header
    uint32_t magic;
    if (read(fd, &magic, sizeof(magic)) != sizeof(magic)) {
        close(fd);
        return symbols;
    }

    lseek(fd, 0, SEEK_SET);

    // Handle both 64-bit and 32-bit Mach-O
    bool is_64bit = (magic == MH_MAGIC_64 || magic == MH_CIGAM_64);
    bool is_fat = (magic == FAT_MAGIC || magic == FAT_CIGAM);

    if (is_fat) {
        println("Fat binary detected - using first slice");
        struct fat_header fat;
        lseek(fd, 0, SEEK_SET);
        read(fd, &fat, sizeof(fat));

        struct fat_arch arch;
        read(fd, &arch, sizeof(arch));
        uint32_t offset_val = ntohl(arch.offset);
        lseek(fd, offset_val, SEEK_SET);
        read(fd, &magic, sizeof(magic));
        is_64bit = (magic == MH_MAGIC_64);
        lseek(fd, offset_val, SEEK_SET);
    }

    if (is_64bit) {
        struct mach_header_64 header;
        lseek(fd, 0, SEEK_SET);
        if (read(fd, &header, sizeof(header)) != sizeof(header)) {
            close(fd);
            return symbols;
        }

        // Read load commands
        uint64_t offset = sizeof(header);
        struct symtab_command* symtab = 0;

        for (uint32_t i = 0; i < header.ncmds; i++) {
            struct load_command lc;
            lseek(fd, offset, SEEK_SET);
            read(fd, &lc, sizeof(lc));

            if (lc.cmd == LC_SYMTAB) {
                symtab = (struct symtab_command*)malloc(lc.cmdsize);
                lseek(fd, offset, SEEK_SET);
                read(fd, symtab, lc.cmdsize);
                break;
            }
            offset += lc.cmdsize;
        }

        if (symtab) {
            // Read string table
            char* strtab = (char*)malloc(symtab->strsize);
            lseek(fd, symtab->stroff, SEEK_SET);
            read(fd, strtab, symtab->strsize);

            // Read symbol table
            struct nlist_64* nlist = (struct nlist_64*)malloc(symtab->nsyms * sizeof(struct nlist_64));
            lseek(fd, symtab->symoff, SEEK_SET);
            read(fd, nlist, symtab->nsyms * sizeof(struct nlist_64));

            // Extract external symbols only
            for (uint32_t i = 0; i < symtab->nsyms; i++) {
                if ((nlist[i].n_type & N_EXT) && nlist[i].n_value) {
                    if (nlist[i].n_un.n_strx < symtab->strsize) {
                        const char* name = strtab + nlist[i].n_un.n_strx;
                        // Skip leading underscore on macOS
                        if (name[0] == '_') name++;

                        SymbolInfo info;
                        info.name = String(name);
                        info.address = nlist[i].n_value;
                        info.is_function = (nlist[i].n_type & N_TYPE) == N_SECT;
                        symbols.add(info);
                    }
                }
            }

            free(nlist);
            free(strtab);
            free(symtab);
        }
    }

    close(fd);
    return symbols;
}
#endif

#ifdef __linux__
// Parse ELF binary to extract symbols
static List<SymbolInfo> parse_elf_symbols(const char* lib_path) {
    List<SymbolInfo> symbols;

    int fd = open(lib_path, O_RDONLY);
    if (fd < 0) {
        print("Failed to open file: ");
        println(lib_path);
        return symbols;
    }

    // Read ELF header
    Elf64_Ehdr ehdr;
    if (read(fd, &ehdr, sizeof(ehdr)) != sizeof(ehdr)) {
        close(fd);
        return symbols;
    }

    // Verify ELF magic
    if (ehdr.e_ident[EI_MAG0] != ELFMAG0 ||
        ehdr.e_ident[EI_MAG1] != ELFMAG1 ||
        ehdr.e_ident[EI_MAG2] != ELFMAG2 ||
        ehdr.e_ident[EI_MAG3] != ELFMAG3) {
        close(fd);
        return symbols;
    }

    bool is_64bit = (ehdr.e_ident[EI_CLASS] == ELFCLASS64);

    if (is_64bit) {
        // Read section headers
        Elf64_Shdr* shdrs = (Elf64_Shdr*)malloc(ehdr.e_shnum * sizeof(Elf64_Shdr));
        lseek(fd, ehdr.e_shoff, SEEK_SET);
        read(fd, shdrs, ehdr.e_shnum * sizeof(Elf64_Shdr));

        // Find .dynsym and .dynstr sections
        Elf64_Shdr* dynsym = 0;
        Elf64_Shdr* dynstr = 0;

        for (int i = 0; i < ehdr.e_shnum; i++) {
            if (shdrs[i].sh_type == SHT_DYNSYM) {
                dynsym = &shdrs[i];
            }
        }

        if (dynsym) {
            dynstr = &shdrs[dynsym->sh_link];

            // Read string table
            char* strtab = (char*)malloc(dynstr->sh_size);
            lseek(fd, dynstr->sh_offset, SEEK_SET);
            read(fd, strtab, dynstr->sh_size);

            // Read symbol table
            int sym_count = dynsym->sh_size / sizeof(Elf64_Sym);
            Elf64_Sym* syms = (Elf64_Sym*)malloc(dynsym->sh_size);
            lseek(fd, dynsym->sh_offset, SEEK_SET);
            read(fd, syms, dynsym->sh_size);

            // Extract symbols
            for (int i = 0; i < sym_count; i++) {
                // Only exported symbols
                if (ELF64_ST_BIND(syms[i].st_info) == STB_GLOBAL &&
                    syms[i].st_shndx != SHN_UNDEF) {

                    const char* name = strtab + syms[i].st_name;

                    SymbolInfo info;
                    info.name = String(name);
                    info.address = syms[i].st_value;
                    info.is_function = ELF64_ST_TYPE(syms[i].st_info) == STT_FUNC;
                    symbols.add(info);
                }
            }

            free(syms);
            free(strtab);
        }

        free(shdrs);
    }

    close(fd);
    return symbols;
}
#endif

// Generic cross-platform symbol parser
static List<SymbolInfo> parse_library_symbols(const char* lib_path) {
#ifdef __APPLE__
    return parse_macho_symbols(lib_path);
#elif defined(__linux__)
    return parse_elf_symbols(lib_path);
#else
    return List<SymbolInfo>();
#endif
}

// Helper to list exported symbols from a dynamic library
static void list_library_symbols(const char* lib_path) {
    print("Symbols in library: ");
    println(lib_path);

    List<SymbolInfo> symbols = parse_library_symbols(lib_path);

    if (symbols.size_ == 0) {
        println("No symbols found or failed to parse");
        return;
    }

    print("Found ");
    print(symbols.size_);
    println(" exported symbols");

    // Show first 20 function symbols
    int shown = 0;
    for (int i = 0; i < symbols.size_ && shown < 20; i++) {
        if (symbols[i].is_function) {
            print("  ");
            println(symbols[i].name);
            shown++;
        }
    }

    if (shown < symbols.size_) {
        print("  ... and ");
        print(symbols.size_ - shown);
        println(" more");
    }
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

    // Test 3: Parse library binary and enumerate all symbols
    {
        println("\n--- Test 3: Parse library binary and enumerate symbols ---");

#ifdef __APPLE__
        // Try to find an actual library file to parse
        // Modern macOS has libraries in dyld cache, so we look for homebrew libs
        const char* test_libs[] = {
            "/usr/local/lib/libgc.1.dylib",
            "/opt/homebrew/lib/libgc.1.dylib",
            "/usr/lib/libz.dylib",
            0
        };

        for (int i = 0; test_libs[i]; i++) {
            struct stat st;
            if (stat(test_libs[i], &st) == 0) {
                list_library_symbols(test_libs[i]);
                break;
            }
        }
#else
        list_library_symbols("/lib/x86_64-linux-gnu/libm.so.6");
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

        // Method 2: RTLD_NEXT (instructs dlsym to search for symbols in libraries that were loaded 
        // after the current library in the linking order. This is useful for symbol interposition 
        // where you want to find the "next" occurrence of a symbol after the current library)
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