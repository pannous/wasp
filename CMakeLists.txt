cmake_minimum_required(VERSION 3.10)
project(wasp)

set(CMAKE_VERBOSE_MAKEFILE OFF) # DEBUG CMAKE

if (X86_64)
    LINK_DIRECTORIES(bin/mac/lib.x86)
else ()
    #    LINK_DIRECTORIES(Frameworks/wasmtime-dev-aarch64-macos-c-api/lib)
    LINK_DIRECTORIES(bin/mac/lib)
    #    LINK_DIRECTORIES(bin/linux/arm64/lib/)
endif ()

# SET these configurations via Preferences… Build->CMake->Profiles
# MANUALLY force set these only for urgent debugging,
#set(RUNTIME_ONLY 1) # no Angle eval emit merge etc! ≠ NO_TESTS
#set(RELEASE 0) # no tests todo VS:
#set(NO_TESTS 1) # no tests
#set(VERBOSE 1)
#set(DEBUG 1)
#set(TRACE 1)
#set(SDL 1) # Graphics
#set(WABT_MERGE 1) # not the whole WABT though, heavy
#set(INCLUDE_MERGER 0)
#set(WASM 1) # no WebView, duh SET VIA TOOLCHAIN!
#ADD_DEFINITIONS(-DMULTI_VALUE)
#ADD_DEFINITIONS(-DDEBUG_WASM3) # WASM3 best for development!

# binaryen_wasm target : https://github.com/WebAssembly/binaryen/blob/main/CMakeLists.txt#L407.

if (NOT WASM)
    set(CONSOLE 1)
    if (NOT RUNTIME_ONLY)
        set(INCLUDE_MERGER 1) # works with WASI
        # TO EXECUTE wasm:
        # https://github.com/WebAssembly/wasm-c-api implemented by V8 Wabt Wasmtime Wasmer
        # as of 2022-10 wasm-c-api encompasses multiple return values and reference types, but not yet threads.
        SET(WASMTIME 1) # FASTEST!
        #    SET(WASMEDGE 1) # todo smart multi-value returns
        #    set(WEBAPP 1) # FAST!! WebView for electron like standalone apps.
        #        set(WASM3 1) # GOOD (backtrace…) wasm_runner_wasm3.cpp BEST API but dying 2022-12: last release on Jun 2, 2021
        #            set(MICRO 1) # SLOW on Mac M1 wasm-micro-runtime => wasm_runner_micro.cpp TODO: use AOT/JIT mode!?
        #        SET(V8 1) # 2022-10 Exception: EXC_BAD_ACCESS (code=1, address=0x0) :( and VERY SLOW !?
        #            set(WABT 1) # 2022-12 broken! EXTREMELY SLOW on Mac M1!  wasm_runner_wabt.cpp
        #    set(WASMX 1)  # doesn't work: cannot insert imports or function types!
        #    set(WASMER 1) # only good for calling tested code, otherwise it gives ZERO info on what went wrong!
    endif ()
    #    include_directories(SYSTEM /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/usr/include)
    # ^^ fixes <cmath> mac errors!
    ADD_DEFINITIONS(-D_Backtrace_)
    set(SOURCE_FILES ${SOURCE_FILES} source/Backtrace.cpp)
endif () # else :
#
#if(WASP_FULL)
#    # experiment: whole runtime included!
#    SET(WASM3 1)
#endif()

if (INCLUDE_MERGER)
    message(INCLUDE_MERGER)
    ADD_DEFINITIONS(-DINCLUDE_MERGER)
    set(SOURCE_FILES ${SOURCE_FILES} source/wasm_merger_own.cpp)
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


if (WASM)
    set(WASI 0) # increases size by factor 10!! 100873 wasp.wasm => 1074221 wasp.wasm_10x_with_WASI
    set(MY_WASI 1) # manually write via fd_write, NOT via wasi-sdk printf=>fd_write
    ADD_DEFINITIONS(-DMY_WASI)
    ADD_DEFINITIONS(-DNO_CPP_DEMANGLE)
    #    set(RUNTIME_ONLY 1) NO, we want compiler version too!
    #    ADD_DEFINITIONS(-DRUNTIME_ONLY) # no wasm reader != NO_TESTS
    if (NOT WASI AND NOT MY_WASI)
        ADD_DEFINITIONS(-DPURE_WASM)
        ADD_DEFINITIONS(-DMY_WASM) # use put() panic() ... !
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
    #    SET(RUNTIME_ONLY 1) # we can emit to V8, right??
    set(SOURCE_FILES ${SOURCE_FILES} source/ErrorHandler.cpp) # no SIGSEGV mac popup
endif ()
if (WASM)
    ADD_DEFINITIONS(-DWASM)
    SET(WASM3 0) # No wasm-VM inside wasm
    REMOVE_DEFINITIONS(WASM3)
    #    multi-value referenceTypes bulkMemory works in FF
    #    ADD_COMPILE_FLAG("-O0 -s WASM_BIGINT") should work in WebKit but we only use standard MVP features for now!
    SET(WEBAPP 0) # just be sure
    SET(SDL 0)
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


FUNCTION(ADD_COMPILE_FLAG value)
    MESSAGE(STATUS "Building ${TARGET} with ${value}")
    FOREACH (variable CMAKE_C_FLAGS CMAKE_CXX_FLAGS)
        SET(${variable} "${${variable}} ${value}" PARENT_SCOPE)
    ENDFOREACH (variable)
ENDFUNCTION()

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
    ADD_COMPILE_FLAG(" -g3") # DEBUG
endif ()

if (TRACE)
    message("TRACE => tracing=true")
    #    ADD_COMPILE_FLAG("-Werror") # WARNINGS AS ERRORS!  use -Wno-error=… for exceptions
    #    add_compile_options(-Wall)
    #    add_compile_options(-Wextra)
endif ()

ADD_COMPILE_FLAG("-fno-inline") # why not? debug?

# ENABLE:
#ADD_COMPILE_FLAG("-Werror") # WARNINGS AS ERRORS!  use -Wno-error=… for exceptions
ADD_COMPILE_FLAG("-Wformat") # essential !! print("%s",string) => memory corruption!
ADD_COMPILE_FLAG("-Wreturn-type") # VERY USEFUL : non-void function does not return a value
ADD_COMPILE_FLAG("-Wno-writable-strings") # ALWAYS allow conversion from string literal to 'char *'
ADD_COMPILE_FLAG("-Wno-ambiguous-reversed-operator") # useless
ADD_COMPILE_FLAG("-Wunused-result") # VERY USEFUL : check for nodiscard, e.g. in non-self-modifying replace()
ADD_COMPILE_FLAG("-Wno-unused-command-line-argument") # may conflict between builds, so disable freely!
ADD_COMPILE_FLAG("-Wno-tautological-undefined-compare") # harmless if this == 0 check
ADD_COMPILE_FLAG("-Wno-non-pod-varargs") #  usually only Plain Old Data structures!
ADD_COMPILE_FLAG("-Wno-varargs") # contra: "passing an object that undergoes default argument promotion to 'va_start' has undefined behavior" ??
# todo Just use initiator lists? ^^
if (APPLE)
    ADD_COMPILE_FLAG("-Wreturn-stack-address") # VERY USEFUL : disallow stack memory associated return!
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
        ADD_COMPILE_FLAG("-Wno-unknown-attributes") # wasi
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
endif ()

if (VERBOSE)
    ADD_COMPILE_FLAG("-v") # DEBUG lld
endif ()

#if (DEBUG)
#else ()
if (RELEASE)
    #    ADD_COMPILE_FLAG("-w") # hide warnings
    if (NOT WASM)
        ADD_COMPILE_FLAG("-Os -Oz") # NO WASM GENERATED wth. size should be ok? -O1 removes symbol names https://bugs.llvm.org/show_bug.cgi?id=45602 !
    endif ()
endif ()

#ADD_DEFINITIONS(-DNO_CPP_DEMANGLE)

# COMMON FOR ALL
set(SOURCE_FILES ${SOURCE_FILES} source/String.cpp source/Node.cpp source/Map.cpp source/Util.cpp source/List.cpp source/Wasp.cpp source/wasm_helpers.cpp source/Code.cpp source/NodeTypes.cpp)

if (CONSOLE)
    message(CONSOLE)
    ADD_DEFINITIONS(-DCONSOLE)
    ADD_DEFINITIONS(-DUSE_READLINE)
    set(SOURCE_FILES ${SOURCE_FILES} source/console.cpp) # Needs to come after, because conflicting Function symbol
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


add_compile_options(-Werror=deprecated-enum-compare) # check(Primitive::charp == (Valtype) Primitive::charp) FAILS!!!
add_compile_options(-Wdeprecated-enum-compare)
#add_compile_options(-Wall)
add_compile_options(-Wno-tautological-constant-out-of-range-compare) # todo remove
ADD_COMPILE_FLAG("-Wno-unknown-pragmas") # always allow
ADD_COMPILE_FLAG("-Wno-unused-parameter") # always
ADD_COMPILE_FLAG("-Wno-deprecated-copy-with-user-provided-copy") # what's this?
add_compile_options(-Werror)

#set(SANITIZE 1)
if (SANITIZE OR TRACE) # or TRACE
    # Often gdb would be a better tool to debug this problem. Run `gdb wasp -ex r` until it crashes.
    # Then print stacktrace with bt to see what was going wrong.
    MESSAGE("ASAN sanitize=address …")
    #    if(APPLE)
    #    ADD_COMPILE_FLAG("-static-libsan ")
    #    else()
    if (GCC)
        ADD_COMPILE_FLAG("-static-libasan")
    else ()
        ADD_COMPILE_FLAG("-static-libsan")
    endif ()
    set(ASAN_OPTIONS verbosity=3)
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
    endif ()

    #   add_compile & add_link NEEDS TO BE IN SYNC!
    #    link_libraries("-fsanitize=address")
    add_link_options(-fsanitize=address)  # currently crashes with AsanDie so don't use :(
    add_link_options(-fsanitize=undefined)
    add_link_options(-fsanitize-trap=undefined)
    add_link_options(-fno-omit-frame-pointer)
    add_link_options(-fsanitize-recover=address)
endif ()

if (WASM)
    set(SOURCE_FILES ${SOURCE_FILES} source/wasm_helpers_wasm.cpp)
else ()
    set(SOURCE_FILES ${SOURCE_FILES} source/wasm_helpers_host.cpp)
endif ()

#
# END OF GENERAL CONFIGURATION
#
# DIFFERENT TARGETS :
if (WASM) # OR WASI)
    message(WASM)
    ADD_DEFINITIONS(-DWASM)
    include(wasm.toolchain.cmake)
    if (WASI)
        ADD_DEFINITIONS(-DWASI) # same as -DWASI=1
        set(CMAKE_ISYSROOT "") # typo? =>
        set(CMAKE_SYSROOT "") # via -DCMAKE_TOOLCHAIN_FILE=wasm.toolchain.cmake
        ADD_COMPILE_FLAG("--target=wasm32-wasi") # default # IF USING OTHER CLANG ... as before switch
    else ()
        ADD_COMPILE_FLAG("--target=wasm32-unknown-unknown-wasm") # default:wasi ok
        ADD_COMPILE_FLAG("-nostdlib -march=wasm") #-v
        ADD_LINK_OPTIONS("-Wl,--no-entry") # if as library
    endif ()
    SET(CMAKE_SYSROOT /opt/wasm/wasi-sdk/share/wasi-sysroot) # also for WASM include!

    #    ADD_COMPILE_FLAG("-fvisibility=hidden") # ??
    ADD_COMPILE_FLAG("-o wasp.wasm") # DIRECT OUTPUT!
#    ADD_COMPILE_FLAG("-Wl,-Clink-arg=--export=__heap_base") # wasm-ld: error: unknown argument:
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
    add_definitions(-DHEAP_OFFSET=65536)
    add_definitions(-DMEMORY_SIZE=117964800)
    ADD_COMPILE_FLAG("-Wl,--initial-memory=117964800") # stack size plus some
    ADD_COMPILE_FLAG("-Wl,-z,stack-size=104857600") # 1MB*10  pointers will START from high and go lower!
    ADD_COMPILE_FLAG("-Qn") # no producer metadata
    ADD_COMPILE_FLAG("-Wl,--whole-archive") #    Force load of all members in a static library
    ADD_COMPILE_FLAG("-Wl,--no-gc-sections") # keep name section?
    #    ADD_COMPILE_FLAG("-Wl,--pie") #  position independent executable, does it help? NO
    #    ADD_COMPILE_FLAG("-fPIC") # does not help creating PIEs, with -pie, is not yet stable

    if (VERBOSE)
        ADD_COMPILE_FLAG("-Wl,-t,--verbose,--export-table -O0") # ,-O0  VERBOSE!
        ADD_COMPILE_FLAG("-Wl,--emit-relocs")
        #    ADD_COMPILE_FLAG("-Wl,-vvv")
    endif ()
    if (DEBUG)
        ADD_COMPILE_FLAG("-O0")
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
    #    TODO "System is unknown to cmake"
    message(WASI)
    ADD_DEFINITIONS(-DWASI) # same as -DWASI=1
    #    only works with wasm.toolchain.cmake
    SET(CMAKE_CXX_COMPILER /opt/wasm/wasi-sdk/bin/clang)
    SET(CMAKE_SYSROOT /opt/wasm/wasi-sdk/share/wasi-sysroot) # also for WASM include!
    ADD_COMPILE_FLAG("--sysroot=/opt/wasm/wasi-sdk/share/wasi-sysroot")
    #  ADD_COMPILE_FLAG("-Wl,--entry=main,--allow-undefined") # __cxa_throw ?
    #${WAMR_ROOT_DIR}/wamr-sdk/app/libc-builtin-sysroot/include/ # stdbool :(
    add_executable(wasp.wasm ${SOURCE_FILES})
    add_custom_target(BuildWasi ALL DEPENDS wasp.wasm)
elseif (MICRO) # wasm-micro-vm:
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
    if (APPLE)
        target_link_libraries(wasp vmlib -ldl -lpthread) # -lm
    else ()
        target_link_libraries(wasp vmlib -ldl -lpthread -lrt)
    endif ()
    if (SDL)
        target_link_libraries(wasp SDL2::SDL2)
    endif ()

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
    set(SOURCE_FILES ${SOURCE_FILES} source/wasmer_runner.cpp)
    add_executable(wasp ${SOURCE_FILES})
    target_include_directories(wasp PUBLIC Frameworks/wasmer-c-api)
    target_link_libraries(wasp PRIVATE wasmer)
elseif (WASMEDGE)
    MESSAGE("USING WASMEDGE")
    ADD_DEFINITIONS(-DWASMEDGE)
    set(SOURCE_FILES ${SOURCE_FILES} source/wasmedge_runner.cpp)
    set(SOURCE_FILES source/wasmedge_runner.cpp ${SOURCE_FILES})
    add_executable(wasp ${SOURCE_FILES})
    target_include_directories(wasp PUBLIC Frameworks/wasmedge)
    target_link_libraries(wasp PRIVATE wasmedge) # libwasmedge.dylib 88MB holy fuck!!
elseif (WASMTIME)
    ADD_DEFINITIONS(-DWASMTIME)
    set(SOURCE_FILES ${SOURCE_FILES} source/wasmtime_runner.cpp)
    add_executable(wasp ${SOURCE_FILES})
    #    target_include_directories(wasp PUBLIC Frameworks/wasmtime-c-api/include/)
    #    target_include_directories(wasp PUBLIC Frameworks/wasmtime-dev-x86_64-macos-c-api/include/)
    #    LINK_DIRECTORIES(Frameworks/wasmtime-dev-x86_64-macos-c-api/lib/)
    target_include_directories(wasp PUBLIC Frameworks/wasmtime-dev-aarch64-macos-c-api/include)
    LINK_DIRECTORIES(Frameworks/wasmtime-dev-aarch64-macos-c-api/lib)
    target_link_libraries(wasp PRIVATE wasm)
    target_link_libraries(wasp PRIVATE wasmtime)
elseif (WEBAPP)
    ADD_DEFINITIONS(-DWEBAPP)
    include(source/WebView.cmake)
    set(SOURCE_FILES ${SOURCE_FILES} source/WebApp.cpp source/WebServer.cpp)
    add_executable(wasp ${SOURCE_FILES})
    target_link_libraries(wasp PUBLIC webview)
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
    set(SOURCE_FILES source/wasm_runner_v8.cpp source/Util.cpp source/Util.h source/List.cpp source/List.h ${SOURCE_FILES})
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


if (CONSOLE)
    find_package(Readline)
    #    find_library(Readline)
    #    link_libraries(wasp readline)
    TARGET_LINK_LIBRARIES(wasp PRIVATE readline) # => libreadline
endif ()

if (WASM) # OR WASI) common to all!
    #    target_include_directories(wasp.wasm PUBLIC Frameworks/wasm-c-api/include/)
    #else ()
    #    target_include_directories(wasp PUBLIC Frameworks/wasm-c-api/include/)
endif ()

#set(CMAKE_SUPPRESS_REGENERATION)
#set(CMAKE_SKIP_RULE_DEPENDENCY TRUE) # incremental build
#set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "/INCREMENTAL:YES ${CMAKE_CXX_FLAGS_RELEASE}")

# Please either delete it manually or select another generation directory => delete CMakeCache.txt
