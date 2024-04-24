# REMOVE CMakeCache.txt after switching between mac and linux / " Cannot generate into … " errors

cmake_minimum_required(VERSION 3.10)
project(wasp)

set(CMAKE_VERBOSE_MAKEFILE OFF) # DEBUG CMAKE

#set(CROSSCOMPILE x86) # needs more work / just use different docker images
if (CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "x86_64")
    set(X86_64 1)
endif ()

# ⚠️ SET these configurations VIA TOOLCHAIN: via CLion Preferences… Build->CMake->Profiles
#  MANUALLY force set these only for urgent debugging
#set(RELEASE 1) # no tests todo VS:
#set(NO_TESTS 1) # no tests
#set(RUNTIME_ONLY 1) # no Angle eval emit merge etc! ≠ NO_TESTS
#set(VERBOSE 1)
#set(DEBUG 1)
#set(TRACE 1)
set(STRICT 1)
#set(SDL 1) # Graphics
#set(WASM 1) # SET VIA TOOLCHAIN!
#set(INCLUDE_MERGER 0) # include wasm_linker.cpp
#set(WABT_MERGE 1) # not the whole WABT though, heavy
#ADD_DEFINITIONS(-DMULTI_VALUE) # change ABI to always return (value, type) tuple
#ADD_DEFINITIONS(-DDEBUG_WASM3) # WASM3 best for development!
#ADD_DEFINITIONS(-DWINDOWS)
#ADD_COMPILE_DEFINITIONS(-DWINDOWS)
# binaryen_wasm target : https://github.com/WebAssembly/binaryen/blob/main/CMakeLists.txt#L407.

if (APPLE)
    message("APPLE")
else ()
    message("LINUX")
    set(LINUX 1)
    set(SANITIZE 0) # debian undefined reference to symbol 'dladdr@@GLIBC_2.17'
    ADD_DEFINITIONS(-DLINUX)
endif ()

if (X86_64)
    LINK_DIRECTORIES(bin/mac/lib.x86)
    #elseif (LINUX) # ok, other extension .so
    #    LINK_DIRECTORIES(lib/linux)
else ()
    LINK_DIRECTORIES(lib/)
    #    LINK_DIRECTORIES(bin/mac/lib)
    #    LINK_DIRECTORIES(bin/mac/lib/aarch64)
    #        LINK_DIRECTORIES(bin/linux/arm64/lib/)
    #    LINK_DIRECTORIES(Frameworks/wasmtime-dev-aarch64-macos-c-api/lib)
endif ()


if (EMSCRIPTEN)
    message("EMSCRIPTEN")
    set(WASM 1)
    ADD_DEFINITIONS(-DEMSCRIPTEN)
endif ()


if (NOT WASM)
    set(CONSOLE 1)
    if (NOT RUNTIME_ONLY)
        # TO EXECUTE wasm provide _run_wasm :
        # https://github.com/WebAssembly/wasm-c-api implemented by V8 Wabt Wasmtime Wasmer
        # as of 2022-10 wasm-c-api encompasses multiple return values and reference types, but not yet threads.
        SET(WASMEDGE 1) # fastest, easiest, bestest. todo smart multi-value returns
        #        SET(WASMEDGE 0)
        # SET(WASMTIME 1) # FASTEST but unstable / elusive bugs "object used with the wrong store" etc
        #        set(WEBAPP 1) # FAST!! WebView for electron like standalone apps.
        #  set(WASM3 1) # GOOD (backtrace…) wasm_runner_wasm3.cpp BEST API but dying to Russia :( 2022-12: last release on Jun 2, 2021
        #  set(MICRO 1) # WAMR 2023-02 still NOT M1 READY! wasm-micro-runtime => wasm_runner_micro.cpp TODO: use AOT/JIT mode!?
        #  SET(V8 1) # 2022-10 Exception: EXC_BAD_ACCESS (code=1, address=0x0) :( and VERY SLOW !?
        #  set(WABT 1) # 2022-12 broken! EXTREMELY SLOW on Mac M1!  wasm_runner_wabt.cpp
        #  set(WASMX 1)  # doesn't work: cannot insert imports or function types!
        #  set(WASMER 1) # only good for calling tested code, otherwise it gives ZERO info on what went wrong!
        # others: WAVM WAMR
        if (WEBAPP)
            set(INCLUDE_MERGER 1) # should work !?
        else ()
            set(INCLUDE_MERGER 1) # works with WASI
        endif ()
    endif ()
    #    include_directories(SYSTEM /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/usr/include)
    # ^^ fixes <cmath> mac errors!

endif () # else :
#
#if(WASP_FULL)
#    # experiment: whole runtime included!
#    SET(WASM3 1)
#endif()

FUNCTION(ADD_COMPILE_FLAG value)
    MESSAGE(STATUS "Building ${TARGET} with ${value}")
    FOREACH (variable CMAKE_C_FLAGS CMAKE_CXX_FLAGS)
        SET(${variable} "${${variable}} ${value}" PARENT_SCOPE)
    ENDFOREACH (variable)
ENDFUNCTION()


if (WASM)
    set(WASI 0) # increases size by factor 10!! 100873 wasp.wasm => 1074221 wasp.wasm_10x_with_WASI
    set(MY_WASI 1) # manually write via fd_write, NOT via wasi-sdk printf=>fd_write
    ADD_DEFINITIONS(-DMY_WASI)
    ADD_DEFINITIONS(-DNO_CPP_DEMANGLE)
    if (DEBUG)
        message("WASM DEBUG with DWARF!")
        ADD_COMPILE_FLAG(-g3)
        ADD_COMPILE_FLAG(-gdwarf-5)
        ADD_COMPILE_FLAG(-gembed-source)
        ADD_COMPILE_FLAG(-fforce-dwarf-frame)
    endif ()
    #    set(RUNTIME_ONLY 1) NO, we want compiler version too!
    #    ADD_DEFINITIONS(-DRUNTIME_ONLY) # no wasm reader != NO_TESTS
    if (MY_WASM)
        #        execute_process(COMMAND node clean.js WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})
        message("MY_WASM!")
        ADD_DEFINITIONS(-DMY_WASM) # use custom host functions run_wasm() put() panic() ... !
    endif ()
    if (NOT WASI AND NOT MY_WASI)
        ADD_DEFINITIONS(-DPURE_WASM)
    endif ()
endif ()

if (RELEASE)
    MESSAGE(RELEASE)
    ADD_DEFINITIONS(-DRELEASE)

    #    set(RUNTIME_ONLY 1)
    set(NO_TESTS 1)
    SET(COPY_PHASE_STRIP YES)
    SET(STRIP_INSTALLED_PRODUCT YES)
    SET(STRIP_STYLE all)
    SET(SEPARATE_STRIP YES)
endif ()


# V8 has wasm, so no m3 needed in release. can we even debug through V8??
# interestingly if we have a native app we don't need our wasp.wasm runtime because it's c++ compiled into app.
# but we would need a million bindings and calls, also we would LEAVE OUR SANDBOX,
# so definitely ship wasp.wasm runtime in release!

if (WEBAPP)
    ADD_DEFINITIONS(-DWEBAPP)
    ADD_DEFINITIONS(-DGRAFIX)
    SET(WASM 0)
    set(WASI 0)
    set(CONSOLE 0)
    #    SET(RUNTIME_ONLY 1) # we can emit to V8, right??
    set(SOURCE_FILES ${SOURCE_FILES} source/ErrorHandler.cpp) # no SIGSEGV mac popup
endif ()
if (WASM)
    ADD_DEFINITIONS(-DWASM)
    #    ADD_COMPILE_FLAG("-O0 -s WASM_BIGINT") should work in WebKit but we only use standard MVP features for now!
    #    multi-value referenceTypes bulkMemory works in FF
    REMOVE_DEFINITIONS(WASM3) # ?
    SET(WASM3 0) # No wasm-VM inside wasm
    SET(WEBAPP 0) # just be sure, don't pack browser into wasm ;)
    SET(SDL 0) # no c++ graphics in wasm, use canvas!
    # ^^ UNSET, or better don't set manually at all!
endif ()

if (SDL)
    message("INCLUDE SDL")
    ADD_DEFINITIONS(-DGRAFIX)
    find_package(SDL2 REQUIRED)
    ADD_DEFINITIONS(-DSDL)
    remove_definitions(SERVER)
    include_directories(${SDL2_INCLUDE_DIRS})
    set(SOURCE_FILES ${SOURCE_FILES} source/Paint.cpp) # if SDL
    #    add_executable(wasp.sdl ${SOURCE_FILES}) # same as add_library, with or without --export-all
endif ()


set(CMAKE_CXX_STANDARD 20)

ADD_COMPILE_FLAG("-fshort-enums") # enum types only use as many bytes as it needs for the declared range of possible values.
# makes TypedValue 1+8 bytes, doesn't help for multi-value (int32 int64)


if (DEBUG)
    MESSAGE("DEBUG!")
    ADD_DEFINITIONS(-DDEBUG)
    SET(GCC_GENERATE_DEBUGGING_SYMBOLS YES)
    SET(GCC_DEBUGGING_SYMBOLS full)
    SET(DEBUG_INFORMATION_FORMAT dwarf-with-dsym)
    SET(DWARF_DSYM_FILE_NAME "$(TARGET_NAME).dSYM")
    SET(DWARF_DSYM_FOLDER_PATH "$(CONFIGURATION_BUILD_DIR)/dSyms")
    SET(DEPLOYMENT_POSTPROCESSING YES)
    ADD_COMPILE_FLAG(" -g3") # DEBUG DWARF output only emscripten?
    #    Compilers like Rust and Clang already support emitting DWARF information in WebAssembly modules.
endif ()

ADD_COMPILE_FLAG("-fno-inline") # why not? debug?

# ENABLE:
if (STRICT)
    MESSAGE("STRICT!")
    ADD_COMPILE_FLAG("-Wall")
    ADD_COMPILE_FLAG("-Werror") # WARNINGS AS ERRORS!  use -Wno-error=… for exceptions
endif ()
ADD_COMPILE_FLAG("-Wformat") # essential !! print("%s",string) => memory corruption!
ADD_COMPILE_FLAG("-Wreturn-type") # VERY USEFUL : non-void function does not return a value
ADD_COMPILE_FLAG("-Wunused-result") # VERY USEFUL : check for nodiscard, e.g. in non-self-modifying replace()
ADD_COMPILE_FLAG("-Wno-tautological-undefined-compare") # harmless this == 0 check
if (LINUX)
    ADD_COMPILE_FLAG("-Wno-ambiguous-reversed-operator")
    ADD_COMPILE_FLAG("-Wno-writable-strings") # dangerous! fix assert_is("square 3", 9)
    # DEBIAN
    # 2023-10-9 /wasp/source/tests.cpp:48:25: gcc internal compiler error
    # todo remove when gcc fixed
    #    set(CMAKE_C_COMPILER clang)
    #    set(CMAKE_CXX_COMPILER clang++)


    #    find_package(Threads REQUIRED)
    #    find_library(DL_LIBRARY dl)

    ADD_COMPILE_FLAG("-w") # hide warnings because gcc is worse than clang
    ADD_COMPILE_FLAG("-Wno-error=attributes") # [[maybe_unused]] attribute ignored ok
    ADD_COMPILE_FLAG("-Wno-error=literal-suffix") # allow String operator ""s(chars c, unsigned long);
    ADD_COMPILE_FLAG("-fpermissive") #  -fpermissive vs -pedantic-errors !
    ADD_COMPILE_FLAG("-Wno-error=write-strings") # message = "error"; ??
    #    error: declaration of ‘codepoint Value::codepoint’ changes meaning of ‘codepoint’  todo: just rename
endif ()
ADD_COMPILE_FLAG("-Wno-unknown-attributes") # import_module("wasi_unstable")
ADD_COMPILE_FLAG("-Wno-varargs") # contra: "passing an object that undergoes default argument promotion to 'va_start' has undefined behavior" ??

#if(NOT LINUX)
#ADD_COMPILE_FLAG("-Wno-non-pod-varargs") #  usually only Plain Old Data structures!
#ADD_COMPILE_FLAG("-Wno-unused-command-line-argument") # may conflict between builds, so disable freely!
#endif()
# todo Just use initiator lists? ^^
if (APPLE)
    ADD_COMPILE_FLAG("-Wreturn-stack-address") # VERY USEFUL : disallow stack memory associated return!
endif ()

#if (X86_64)
if (CROSSCOMPILE=x86)
    message("CROSSCOMPILE!")
    set(CMAKE_SYSTEM_PROCESSOR x86_64) # crosscompile from aarch64
    set(CMAKE_C_COMPILER_TARGET x86_64-linux-gnu)
    set(CMAKE_CXX_COMPILER_TARGET x86_64-linux-gnu)
endif ()

if (NOT TRACE)
    if (APPLE)
        #    ADD_COMPILE_FLAG("-Wall")
        # disable:
        if (DEBUG)
            if (NOT WASM)
                ADD_COMPILE_FLAG("-Wno-error=#warnings") # allow pragma warnings as message
            endif ()
        endif ()
        ADD_COMPILE_FLAG("-Wno-unused-function")
        ADD_COMPILE_FLAG("-Wno-unused-variable")
        ADD_COMPILE_FLAG("-Wno-unused-private-field")
        ADD_COMPILE_FLAG("-Wno-comment") # allow /* an old nested /* comment */ */
        ADD_COMPILE_FLAG("-Wno-typedef-redefinition")
        ADD_COMPILE_FLAG("-Wno-undefined-bool-conversion") # if (!this)return false
        ADD_COMPILE_FLAG("-Wno-user-defined-literals") # allow "hi"s for string("hi")
    endif ()
endif ()

if (GCC)
    ADD_COMPILE_FLAG("-Wno-write-strings") # useless
    ADD_COMPILE_FLAG("-Wno-literal-suffix") # useless
    ADD_COMPILE_FLAG("-Wno-error=unused-variable")
    ADD_COMPILE_FLAG("-Wno-unused-variable")
    ADD_COMPILE_FLAG("-Wno-format") # useful!
elseif (APPLE)
    ADD_COMPILE_FLAG("-Wno-writable-strings") # useless
endif ()

if (VERBOSE)
    ADD_COMPILE_FLAG("-v") # DEBUG lld
endif ()

#if (DEBUG)
#else ()
if (RELEASE)
    #    ADD_COMPILE_FLAG("-w") # hide warnings
    ADD_COMPILE_FLAG("-Os")
    if (NOT WASM AND NOT LINUX)
        ADD_COMPILE_FLAG("-Oz") # NO WASM GENERATED wth. size should be ok? -O1 removes symbol names https://bugs.llvm.org/show_bug.cgi?id=45602 !
    endif ()
endif ()

#ADD_DEFINITIONS(-DNO_CPP_DEMANGLE)

# COMMON FOR ALL
set(SOURCE_FILES ${SOURCE_FILES} source/String.cpp source/Node.cpp source/Map.cpp source/Util.cpp source/List.cpp
        source/Wasp.cpp source/wasm_helpers.cpp source/Code.cpp source/NodeTypes.cpp source/smart_types.cpp)


if (CONSOLE)
    message(CONSOLE)
    ADD_DEFINITIONS(-DCONSOLE)
    ADD_DEFINITIONS(-DUSE_READLINE)
    link_libraries(readline)
    set(SOURCE_FILES ${SOURCE_FILES} source/console.cpp) # Needs to come after, because conflicting Function symbol
endif ()

if (WASM)
    #elseif (MY_WASM)
else ()
    set(CURL 1)  # only 800 bytes via -lcurl !
endif ()

if (CURL)
    find_package(CURL REQUIRED)
    include_directories(${CURL_INCLUDE_DIRS})
    set(SOURCE_FILES ${SOURCE_FILES} source/Fetch.cpp)
    #    only 800 bytes via -lcurl !
endif ()

if (NO_TESTS)
    ADD_DEFINITIONS(-DNO_TESTS)
else ()
    set(SOURCE_FILES ${SOURCE_FILES} source/tests.cpp)
endif ()

if (RUNTIME_ONLY)
    message("RUNTIME_ONLY!")
    ADD_DEFINITIONS(-DRUNTIME_ONLY)
    set(INCLUDE_MERGER 0)
else ()
    message("NOT RUNTIME_ONLY")
    set(SOURCE_FILES ${SOURCE_FILES} source/Code.cpp)
    set(SOURCE_FILES ${SOURCE_FILES} source/Angle.cpp)
    set(SOURCE_FILES ${SOURCE_FILES} source/Interpret.cpp)
    set(SOURCE_FILES ${SOURCE_FILES} source/wasm_emitter.cpp)
    set(SOURCE_FILES ${SOURCE_FILES} source/WitReader.h)
    set(SOURCE_FILES ${SOURCE_FILES} source/wasm_reader.cpp)
endif ()


if (INCLUDE_MERGER)
    message(INCLUDE_MERGER)
    ADD_DEFINITIONS(-DINCLUDE_MERGER)
    set(SOURCE_FILES ${SOURCE_FILES} source/wasm_linker.cpp)
    set(SOURCE_FILES ${SOURCE_FILES} source/own_merge/common.cc)
    set(SOURCE_FILES ${SOURCE_FILES} source/own_merge/binary.cc)
    set(SOURCE_FILES ${SOURCE_FILES} source/own_merge/binary-writer.cc)
    set(SOURCE_FILES ${SOURCE_FILES} source/own_merge/binary-reader-linker.cc)
    set(SOURCE_FILES ${SOURCE_FILES} source/own_merge/binary-reader.cc)
    set(SOURCE_FILES ${SOURCE_FILES} source/own_merge/opcode-code-table.c)
    #set(SOURCE_FILES ${SOURCE_FILES} FILES_MATCHING PATTERN "source/own_merge/*.cc")
    #set(SOURCE_FILES ${SOURCE_FILES} source/own_merge/binary-writer-spec.cc)
    set(SOURCE_FILES ${SOURCE_FILES} source/own_merge/binding-hash.cc)
    set(SOURCE_FILES ${SOURCE_FILES} source/own_merge/feature.cc)
    set(SOURCE_FILES ${SOURCE_FILES} source/own_merge/leb128.cc)
    set(SOURCE_FILES ${SOURCE_FILES} source/own_merge/stream.cc)
    set(SOURCE_FILES ${SOURCE_FILES} source/own_merge/opcode.cc)
    set(SOURCE_FILES ${SOURCE_FILES} source/own_merge/utf8.cc)
    set(SOURCE_FILES ${SOURCE_FILES} source/own_merge/ir.cc)
endif ()


if (APPLE)
    add_compile_options(-Werror=deprecated-enum-compare) # check(Primitive::charp == (Valtype) Primitive::charp) FAILS!!!
    add_compile_options(-Wdeprecated-enum-compare)
    #add_compile_options(-Wall)
    add_compile_options(-Wno-tautological-constant-out-of-range-compare) # todo remove
    ADD_COMPILE_FLAG("-Wno-unknown-pragmas") # always allow
    ADD_COMPILE_FLAG("-Wno-unused-parameter") # always
    ADD_COMPILE_FLAG("-Wno-deprecated-copy-with-user-provided-copy") # what's this?
endif ()
add_compile_options(-Werror)


if (TRACE)
    message("TRACE => tracing=true")
    ADD_DEFINITIONS(-DTRACE)

    #    ADD_COMPILE_FLAG("-Werror") # WARNINGS AS ERRORS!  use -Wno-error=… for exceptions
    #    add_compile_options(-Wall)
    #    add_compile_options(-Wextra)
endif ()

if (SANITIZE) #  OR TRACE
    message("SANITIZE")

    if (NOT EMSCRIPTEN)
        ADD_DEFINITIONS(-D_Backtrace_)
        set(SOURCE_FILES ${SOURCE_FILES} source/Backtrace.cpp)
    endif ()

    # Often gdb would be a better tool to debug this problem. Run `gdb wasp -ex r` until it crashes.
    # Then print stacktrace with bt to see what was going wrong.
    MESSAGE("ASAN sanitize=address …")
    if (GCC)
        ADD_COMPILE_FLAG("-static-libasan")
    else ()
        ADD_COMPILE_FLAG("-static-libsan")
    endif ()
    if (VERBOSE)
        set(ASAN_OPTIONS verbosity=3)
    endif ()
    set(ASAN_OPTIONS=detect_stack_use_after_return=1) # unnecessary because -Wreturn-stack-address
    add_compile_options(-g)
    #    add_compile_options(-enable-coverage)
    add_compile_options(-Wno-missing-field-initializers)
    add_compile_options(-Wformat=2)
    #    #    add_compile_options(-Wpointer-arith)
    #    add_compile_options(-m64)
    #    add_compile_options(-D_LARGEFILE_SOURCE)
    #    add_compile_options(-D_FILE_OFFSET_BITS=64)
    #    add_compile_options(-D_GLIBCXX_USE_CXX11_ABI=0)

    add_compile_options(-fno-stack-protector)
    add_compile_options(-fno-omit-frame-pointer)
    add_compile_options(-fsanitize=undefined)
    add_compile_options(-fsanitize-recover=address)
    add_compile_options(-fsanitize-trap=undefined)
    if (!WASM)
        add_compile_options("-fsanitize=address") # unsupported option '-fsanitize=address' for target 'wasm32-unknown-unknown-wasm'
    endif ()

    if (!APPLE)
        message("LINUX")
        add_compile_options("-fsanitize=leak") # : unsupported option '-fsanitize=leak' for target 'arm64-apple-darwin22.1.0'
        add_compile_options(-fsanitize-address-use-after-return) #not supported by clang
        add_compile_options(-Wbugprone-sizeof-expression)
        add_compile_options(-Werror=bugprone-sizeof-expression)
    else ()
        #        add_compile_options(-fsanitize-trap=undefined)
        #        add_link_options(-fsanitize-trap=undefined)
    endif ()

    #   add_compile & add_link NEEDS TO BE IN SYNC!
    #    link_libraries("-fsanitize=address")
    add_link_options(-fsanitize=address)  # currently crashes with AsanDie so don't use :(
    add_link_options(-fsanitize=undefined)
    add_link_options(-fno-omit-frame-pointer)
    add_link_options(-fsanitize-recover=address)
endif ()

if (WASM)
    set(SOURCE_FILES ${SOURCE_FILES} source/wasm_helpers_wasm.cpp)
    if (NO_TESTS)
    else ()
        set(SOURCE_FILES ${SOURCE_FILES} source/test_browser.cpp)
    endif ()
else ()
    set(SOURCE_FILES ${SOURCE_FILES} source/wasm_helpers_host.cpp)
endif ()

#
# END OF GENERAL CONFIGURATION
#
# DIFFERENT TARGETS :
if (WASM) # OR WASI)
    MESSAGE("USING PURE WASM")
    message(WASM)
    ADD_DEFINITIONS(-DWASM)
    if (EMSCRIPTEN)
        ADD_COMPILE_FLAG("-sERROR_ON_UNDEFINED_SYMBOLS=0")
        set(__aarch64__ 1)
        ADD_DEFINITIONS(-D__aarch64__)
    else ()
        include(wasm.toolchain.cmake)
        SET(CMAKE_SYSROOT /opt/wasm/wasi-sdk/share/wasi-sysroot) # also for WASM include!
    endif ()
    #    ADD_COMPILE_FLAG("-Wno-address-of-temporary") # ok when there is no stack / GC

    if (WASI)
        ADD_DEFINITIONS(-DWASI) # same as -DWASI=1
        set(CMAKE_ISYSROOT "") # type? =>
        set(CMAKE_SYSROOT "") # via -DCMAKE_TOOLCHAIN_FILE=wasm.toolchain.cmake
        ADD_COMPILE_FLAG("--target=wasm32-wasi") # default # IF USING OTHER CLANG ... as before switch
    else ()
        ADD_COMPILE_FLAG("--target=wasm32-unknown-unknown-wasm") # default:wasi ok
        ADD_COMPILE_FLAG("-nostdlib -march=wasm") #-v
        ADD_LINK_OPTIONS("-Wl,--no-entry") # if as library
    endif ()

    #    ADD_COMPILE_FLAG("-fvisibility=hidden") # ??
    ADD_COMPILE_FLAG("-o wasp.wasm") # DIRECT OUTPUT!
    ADD_COMPILE_FLAG("-Wl,--export=__heap_base")
    ADD_COMPILE_FLAG("-Wl,--export=__data_end")
    #    ADD_COMPILE_FLAG("-nostdlib") # use our OWN memcpy (and printf?)
    #    ADD_COMPILE_FLAG("-fno-builtin")
    #    ADD_COMPILE_FLAG("-fno-exceptions") # cannot use 'throw' with exceptions disabled. ok better than typeinfo for char const
    # AVOID   -search_paths_first BUG! : if ( NOT WASM ) in Darwin.cmake (homebrew) or /Applications/CLion.app/Contents/bin/cmake/...
    #    OR  remove from CMAKE_CXX_LINK_FLAGS how
    #-lwasmer --entry=main,

    #  16ca2e: 10 9d 82 80 80 00  | call 285 <Map<String, List<String> >::operator[](String)>
    # names in wasm name section are demangled in debug build, BUT exports stay mangled no matter what!
    ADD_COMPILE_FLAG("-Wl,--demangle,--allow-undefined,--no-check-features") # wasm-ld flags forwarded from clang
    #   #--allow-undefined-file= ,--export-all FUCKS UP __class_type_info!
    #--import-memory    Import memory from the environment
    add_definitions(-DMEMORY_SIZE=117964800)

    ADD_COMPILE_FLAG("-Wl,--initial-memory=1073741824") # 1GB, 4294967296==4GB, 65536000
    ADD_COMPILE_FLAG("   -Wl,-z,stack-size=16000000") #  16-byte aligned 1MB*10
    # SOME "memory access out of bounds" come from too small stack-size! "corrupt string" also if too low or TOO HIGH!?
    #    used by  __wasm_call_ctors etc pointers will START from high and go lower!
    #    ADD_COMPILE_FLAG("-Wl,-z,stack-size=1179648") # 1MB*10  pointers will START from high and go lower!
    #    ADD_COMPILE_FLAG("-Wl,-z,stack-size=0") # do stack-stuff on heap via inspectable byte stack_hack[]
    #    ADD_COMPILE_FLAG("-Qn") # no producer metadata


    ADD_COMPILE_FLAG("-Wl,--whole-archive") #    Force load of all members in a static library
    ADD_COMPILE_FLAG("-Wl,--no-gc-sections") # keep name section?
    ADD_COMPILE_FLAG("-fvisibility-inlines-hidden") # faster linking
    #    ADD_COMPILE_FLAG("-Wl,--pie") #  position independent executable, does it help? NO
    #    ADD_COMPILE_FLAG("-fPIC") # does not help creating PIEs, with -pie, is not yet stable

    if (VERBOSE)
        ADD_COMPILE_FLAG("-Wl,-t,--verbose,--export-table -O0") # ,-O0  VERBOSE!
        ADD_COMPILE_FLAG("-Wl,--emit-relocs")
        #    ADD_COMPILE_FLAG("-Wl,-vvv")
    endif ()
    if (DEBUG)
        ADD_COMPILE_FLAG("-O0")
        ADD_COMPILE_FLAG("-g")
        #        ADD_COMPILE_FLAG("-Oz") # makes wasm Linking SLOOW! OK keeps variable names! DWARF only in emcc
        #        ADD_COMPILE_FLAG("-save-temps=all") # keep intermediates   c stuff not wasm yet
        #        the DWARF section information/references are invalid after wasm-pack, wasm-opt or any other tools,
    endif ()
    if (NOT RELEASE)
        ADD_COMPILE_FLAG("-Wl,--export-all") # preserve functions even if optimized
    endif ()
    if (RELEASE)
        if (RUNTIME_ONLY)
            ADD_COMPILE_FLAG("-Wl,--export-all") # preserve functions even if optimized
            # wasmtime bug can't handle with exported globals
        else ()
            ADD_COMPILE_FLAG("-Wl,--export=_start") # deletes all unused functions!
        endif ()
        #        ADD_COMPILE_FLAG("-Wl,--export-dynamic") #   Put symbols in the dynamic symbol table
        ADD_COMPILE_FLAG("-v")
        #  --reproduce=<value>    Dump linker invocation and input files for debugging

        # No support for creating shared libraries. The spec for shared libraries in WebAssembly is still in flux
        # https://github.com/rust-lang/rust/issues/60231#issuecomment-654645065
        #    ADD_COMPILE_FLAG("-Wl,--shared")
        #relocation R_WASM_MEMORY_ADDR_SLEB cannot be used against symbol nil_name; recompile with -fPIC
        ADD_COMPILE_FLAG("-Wl,--compress-relocations") # incompatible with output debug information. Please pass --strip-debug
        ADD_COMPILE_FLAG("-Wl,--strip-debug") # DEBUG-RELEASE
        set(SOURCE_FILES ${SOURCE_FILES} source/exceptions.cpp)
        ADD_COMPILE_FLAG("-flto") # Add metadata for link-time optimizations => wasp.wasm only 30 kb (without tests) YAY
        #    --no-validation   --debug
        ADD_COMPILE_FLAG("-Oz -Qn")
        ADD_COMPILE_FLAG("-Wl,--gc-sections")
        ADD_COMPILE_FLAG("-Wl,-O4")
        ADD_COMPILE_FLAG("-O4")
        # RELEASE optimize size 130kb as module, 180kb all tests+emitter OK
        # todo ^^   warning: unknown warning option '-Ws,--lto-O3,-O4,--gc-sections'  --lto-O3,
        ADD_COMPILE_FLAG("-fno-inline") # helps https://developers.google.com/web/updates/2020/12/webassembly
        if (EMCC) # emscripten
            ADD_COMPILE_FLAG("-gseparate-dwarf=debug.wasm")
        endif ()
    endif ()
    #    add_library(wasp.wasm SHARED ${SOURCE_FILES}) # libwasp.wasm.so todo: rename
    if (RUNTIME_ONLY)
        add_executable(wasp-runtime.wasm ${SOURCE_FILES}) # same as add_library, with or without --export-all
    elseif (WASP_FULL)
        add_executable(wasp-full.wasm ${SOURCE_FILES}) # same as add_library, with or without --export-all
    else ()
        add_executable(wasp.wasm ${SOURCE_FILES}) # same as add_library, with or without --export-all
    endif ()
elseif (WASI)
    MESSAGE("USING PURE WASI")
    #    TODO "System is unknown to cmake"
    message(WASI)
    ADD_DEFINITIONS(-DWASI) # same as -DWASI=1
    #    only works with wasm.toolchain.cmake
    #    if(NOT EMSCRIPTEN)
    #    SET(CMAKE_CXX_COMPILER /opt/wasm/wasi-sdk/bin/clang)
    #    SET(CMAKE_SYSROOT /opt/wasm/wasi-sdk/share/wasi-sysroot) # also for WASM include!
    #    ADD_COMPILE_FLAG("--sysroot=/opt/wasm/wasi-sdk/share/wasi-sysroot")
    #    endif()
    #  ADD_COMPILE_FLAG("-Wl,--entry=main,--allow-undefined") # __cxa_throw ?
    #${WAMR_ROOT_DIR}/wamr-sdk/app/libc-builtin-sysroot/include/ # stdbool :(
    add_executable(wasp.wasm ${SOURCE_FILES})
    add_custom_target(BuildWasi ALL DEPENDS wasp.wasm)
elseif (MICRO) # wasm-micro-vm:
    MESSAGE("USING WASM MICRO VM")
    if (WASI)
        message(WASI and wasm-micro-vm no good!?)
        return()
    endif ()

    set(SOURCE_FILES ${SOURCE_FILES} source/wasm_runner_micro.cpp)
    message("USING INTERNAL WASM MICRO RUNTIME TO DEBUG (not Mac M1 ready!!)")
    #    quit()
    ADD_DEFINITIONS(-DWASM_ENABLE_INTERP)
    set(WAMR_BUILD_INTERP 1)
    SET(WAMR_ROOT_DIR Frameworks/wasm-micro-runtime)
    if (APPLE)
        add_definitions(-DBH_PLATFORM_DARWIN)
        SET(WAMR_BUILD_PLATFORM darwin)
    endif ()

    # Reset default linker flags
    set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")
    set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "")

    # Build as X86_32 by default in 32-bit platform
    #    set (WAMR_BUILD_TARGET "X86_32")
    if (X86_64)
        set(WAMR_BUILD_TARGET "X86_64")
    else ()
        set(WAMR_BUILD_TARGET "AARCH64")
    endif ()
    set(WAMR_BUILD_LIBC_BUILTIN 1)

    ################  wasm-micro-runtime  ################
    include(${WAMR_ROOT_DIR}/build-scripts/runtime_lib.cmake)
    ################  application related  ################
    include(${SHARED_DIR}/utils/uncommon/shared_uncommon.cmake)

    add_library(vmlib ${WAMR_RUNTIME_LIB_SOURCE})

    #    ADD_DEFINITIONS(-DWASM_ENABLE_FAST_INTERP)
    ADD_COMPILE_FLAG("-dM -ferror-limit=5 -g -O0")#debug

    add_executable(wasp ${SOURCE_FILES} ${UNCOMMON_SHARED_SOURCE})


    #    add_custom_target(BuildEmitter ALL DEPENDS wasp) #.exe
    target_include_directories(wasp PUBLIC
            ${WAMR_ROOT_DIR}/core/iwasm/interpreter/
            ${WAMR_ROOT_DIR}/include
            ${WAMR_ROOT_DIR}/core/iwasm/
            ${WAMR_ROOT_DIR}/core/iwasm/include/
            ${WAMR_ROOT_DIR}/core/shared/utils
            ${WAMR_ROOT_DIR}/core/shared/utils/uncommon
            ${WAMR_ROOT_DIR}/core/shared/platform/include/
            ${WAMR_ROOT_DIR}/core/shared/platform/darwin/
            ${WAMR_ROOT_DIR}/core/shared/mem-alloc
            #    VERSUS /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk
    )
elseif (WABT)
    message(WABT)
    ADD_DEFINITIONS(-DWABT)
    ADD_COMPILE_FLAG("-dM -ferror-limit=5 -g")
    #        include(wabt/cmake_install.cmake)
    #        add_subdirectory(wabt)
    set(SOURCE_FILES ${SOURCE_FILES} source/wasm_runner_wabt.cpp)
    add_executable(wasp ${SOURCE_FILES})
    target_include_directories(wasp PUBLIC Frameworks/wabt/include/)
    target_include_directories(wasp PUBLIC Frameworks/wabt/include/wabt)
    target_include_directories(wasp PUBLIC Frameworks/wabt/)
    target_include_directories(wasp PUBLIC Frameworks/wabt/src)
    target_link_libraries(wasp PRIVATE wasm)  # libwabt is called libwasm.dylib DANGER!
elseif (WASMER)
    MESSAGE("USING WASMER")
    set(SOURCE_FILES ${SOURCE_FILES} source/wasmer_runner.cpp)
    add_executable(wasp ${SOURCE_FILES})
    target_include_directories(wasp PUBLIC Frameworks/wasmer-c-api)
    target_link_libraries(wasp PRIVATE wasmer)
elseif (WASMEDGE)
    MESSAGE("USING WASMEDGE")
    ADD_DEFINITIONS(-DWASMEDGE)
    set(SOURCE_FILES ${SOURCE_FILES} source/wasm_runner_edge.cpp)
    set(SOURCE_FILES source/wasm_runner_edge.cpp ${SOURCE_FILES})
    add_executable(wasp ${SOURCE_FILES})
    #    target_include_directories(wasp PUBLIC Frameworks/wasmedge)
    target_include_directories(wasp PUBLIC Frameworks/wasmedge/include/api/)

    #    target_include_directories(wasp PUBLIC Frameworks/wasmedge_api)
    target_link_libraries(wasp PUBLIC -lcurl)
    target_link_libraries(wasp PRIVATE wasmedge) # libwasmedge.dylib 88MB -> now 5MB ok
elseif (WASMTIME)
    MESSAGE("USING WASMTIME")
    ADD_DEFINITIONS(-DWASMTIME)
    set(SOURCE_FILES ${SOURCE_FILES} source/wasmtime_runner.cpp)
    add_executable(wasp ${SOURCE_FILES})
    target_include_directories(wasp PUBLIC Frameworks/wasmtime-c-api/include/)
    target_include_directories(wasp PUBLIC Frameworks/wasmtime-c-api/c-api/include/)
    target_include_directories(wasp PUBLIC Frameworks/wasmtime-c-api/wasm-c-api/include/)
    if (X86_64)
        target_include_directories(wasp PUBLIC Frameworks/wasmtime-dev-x86_64-macos-c-api/include/)
        LINK_DIRECTORIES(Frameworks/wasmtime-dev-x86_64-macos-c-api/lib/)
    endif ()
    #    target_include_directories(wasp PUBLIC Frameworks/wasmtime-dev-aarch64-macos-c-api/include)
    #    LINK_DIRECTORIES(Frameworks/wasmtime-dev-aarch64-macos-c-api/lib)
    target_link_libraries(wasp PRIVATE wasm)
    target_link_libraries(wasp PRIVATE wasmtime)
elseif (WEBAPP)
    ADD_DEFINITIONS(-DWEBAPP)
    include(source/WebView.cmake)
    set(SOURCE_FILES ${SOURCE_FILES} source/WebApp.cpp source/WebServer.cpp)
    add_executable(wasp-app ${SOURCE_FILES})
    target_link_libraries(wasp-app PUBLIC webview)
    if (CURL)
        target_link_libraries(wasp-app PUBLIC ${CURL_LIBRARIES})
        target_link_libraries(wasp-app PUBLIC -lcurl)
    endif ()
elseif (WASM3)
    MESSAGE(wasm3 VM)
    ADD_DEFINITIONS(-DWASM3)
    ADD_COMPILE_FLAG("-dM -ferror-limit=5 -g")
    #    add_subdirectory(Frameworks/wasm3/source ${CMAKE_BINARY_DIR}/m3) # wat?
    include(Frameworks/uvwasi/cmake_install.cmake)
    include(Frameworks/wasm3/cmake_install.cmake)    #    // 2022-12: last release on Jun 2, 2021
    set(SOURCE_FILES ${SOURCE_FILES} source/wasm_runner_wasm3.cpp)
    #    source/types/Number.cpp source/types/Number.h
    add_executable(wasp ${SOURCE_FILES})
    #    target_include_directories(wasp PUBLIC Frameworks/wasm3/platforms/cpp/wasm3_cpp/include/)
    target_include_directories(wasp PUBLIC Frameworks/wasm3/source)
    target_include_directories(wasp PUBLIC Frameworks/uvwasi/include)
    target_link_libraries(wasp PRIVATE m3)
    if (SDL)
        target_link_libraries(wasp PRIVATE SDL2::SDL2)
    endif ()
    #    target_link_libraries(wasp ${SDL2_LIBRARIES})
    #    add_custom_target(BuildWasm3 ALL DEPENDS angle) #.exe
    #    TO EXECUTE wasm, add ONE of these: wasm_runner_console.cpp  wasm_runner_wasm3.cpp wasm_runner_wabt.cpp wasm_runner_wabt.cpp  wasm_runner_micro.cpp
elseif (WASMX)
    set(SOURCE_FILES ${SOURCE_FILES} source/wasm_runner_console.cpp)
    add_executable(wasp ${SOURCE_FILES})
elseif (V8)
    ADD_DEFINITIONS(-DV8_COMPRESS_POINTERS)
    set(SOURCE_FILES source/wasm_runner_v8.cpp ${SOURCE_FILES})
    add_executable(wasp ${SOURCE_FILES})
    target_include_directories(wasp PUBLIC Frameworks/wasm-c-api/include/)
    target_include_directories(wasp PUBLIC Frameworks/wasm-c-api/v8/include/)
    target_link_libraries(wasp PRIVATE v8 v8_libplatform wasm)
else () # NORMAL:
    message(NO WASM "${WASM}")
    if (RELEASE)
        add_library(wasp SHARED ${SOURCE_FILES})
    else ()
        add_executable(wasp ${SOURCE_FILES})
    endif ()
    #    add_custom_target(BuildWasp ALL DEPENDS angle)
    target_include_directories(${PROJECT_NAME} PUBLIC source/own_merge)
    if (CURL)
        target_link_libraries(${PROJECT_NAME} ${CURL_LIBRARIES})
        target_link_libraries(${PROJECT_NAME} -lcurl)
    endif ()
endif ()

if (NOT WASM)
    if (WABT_MERGE)
        target_include_directories(wasp PUBLIC Frameworks/wabt)
        target_include_directories(${PROJECT_NAME} PUBLIC Frameworks/wabt/src)
        target_include_directories(${PROJECT_NAME} PUBLIC Frameworks/wabt_merge/)
        target_include_directories(${PROJECT_NAME} PUBLIC Frameworks/wabt_merge/tools)
        target_link_libraries(wasp PRIVATE wasm) #    libwabt is called libwasm.dylib DANGER!

        #    target_include_directories(wasp PUBLIC Frameworks/wabt/src/interp)
    endif ()
endif ()

# todo CONSOLE checke above!?
if (CONSOLE AND NOT WEBAPP)
    find_package(Readline)
    find_library(readline NAMES Readline)
    if (WASM)
        TARGET_LINK_LIBRARIES(wasp PRIVATE readline) # => libreadline
    endif ()
endif ()

if (SDL)
    target_link_libraries(wasp SDL2::SDL2)
endif ()

#set(CMAKE_SUPPRESS_REGENERATION)
#set(CMAKE_SKIP_RULE_DEPENDENCY TRUE) # incremental build
#set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "/INCREMENTAL:YES ${CMAKE_CXX_FLAGS_RELEASE}")

# CLion SET cidr.asynchronous.refresh.after.build = false (in Find-Action -> Registry )

# Please either delete it manually or select another generation directory => delete CMakeCache.txt

