cmake_minimum_required(VERSION 3.16)
project(wasp)

set(CMAKE_VERBOSE_MAKEFILE OFF) # DEBUG

#LINK_DIRECTORIES(lib)
LINK_DIRECTORIES(Frameworks)
LINK_DIRECTORIES(/usr/local/lib)
LINK_DIRECTORIES(/opt/homebrew/Cellar/v8/9.3.345.19/libexec/)

# SET these configurations via Preferences… Build->CMake->Profiles
# MANUALLY force set these only for urgent debugging,
#set(RUNTIME_ONLY 1) # no Angle eval emit merge etc! ≠ NO_TESTS
#set(VERBOSE 1)
#set(DEBUG 1)
#set(RELEASE 0) # no tests
#set(SDL 1) # Graphics
#set(WASM 1) # no WebView, duh
#set(WABT_MERGE 0) # not the whole WABT though

#ADD_COMPILE_DEFINITIONS(DEBUG_WASM3) # WASM3 best for development!
#ADD_COMPILE_DEFINITIONS(NO_TESTS) # CAREFUL: unused functions may get optimized away, see -Wl,export-all -Wl,no-gc…

if (WASM)
    ADD_COMPILE_DEFINITIONS(RUNTIME_ONLY) # no wasm reader != NO_TESTS
    if (NOT WASI)
        ADD_COMPILE_DEFINITIONS(MY_WASM) # use log() panic() ... !
    endif ()
endif ()

if (NOT WASM)
    #    include_directories(SYSTEM /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/usr/include)
    # ^^ fixes <cmath> mac errors!
    ADD_COMPILE_DEFINITIONS(_Backtrace_)
    set(SOURCE_FILES ${SOURCE_FILES} source/Backtrace.cpp)
    if (NOT RUNTIME_ONLY)
        # TO EXECUTE wasm:
        # https://github.com/WebAssembly/wasm-c-api implemented by V8 Wabt Wasmtime Wasmer
        #        SET(V8 1)
                        SET(WASMTIME 1) # FASTEST!
        #                set(WEBAPP 1) # FAST!! WebView for electron like standalone apps.
        #        set(WASM3 1) # GOOD (backtrace…) wasm_runner_wasm3.cpp
        #            set(MICRO 1) # SLOW on Mac M1 wasm-micro-runtime => wasm_runner_micro.cpp TODO: use AOT/JIT mode!?
        #                        set(WABT 1) # EXTREMELY SLOW on Mac M1!  wasm_runner_wabt.cpp
        #            set(WASMX 1)  # doesn't work: cannot insert imports or function types!
        #            set(WASMER 1) # only good for calling tested code, otherwise it gives ZERO info on what went wrong!
    endif ()
endif ()


if (RELEASE)
    MESSAGE(RELEASE)
    set(RUNTIME_ONLY 1)
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
    ADD_COMPILE_DEFINITIONS(WEBAPP)
    ADD_COMPILE_DEFINITIONS(GRAFIX)
    SET(WASM 0)
    set(WASI 0)
    #    SET(RUNTIME_ONLY 1) # we can emit to V8, right??
    set(SOURCE_FILES ${SOURCE_FILES} source/ErrorHandler.cpp) # no SIGSEGV mac popup
endif ()
if (WASM)
    ADD_COMPILE_DEFINITIONS(WASM)
    SET(WASM3 0) # No wasm-VM inside wasm
    REMOVE_DEFINITIONS(WASM3)
    #    multi-value referenceTypes bulkMemory works in FF
    #    ADD_COMPILE_FLAG("-O0 -s WASM_BIGINT") should work in WebKit but we only use standard MVP features for now!
    SET(WEBAPP 0) # just be sure
    SET(SDL 0)
    # ^^ UNSET, or better don't set manually at all!
endif ()

if (SDL)
    ADD_COMPILE_DEFINITIONS(GRAFIX)
    find_package(SDL2 REQUIRED)
    ADD_COMPILE_DEFINITIONS(SDL)
    remove_definitions(SERVER)
    include_directories(${SDL2_INCLUDE_DIRS})
    set(SOURCE_FILES ${SOURCE_FILES} source/Paint.cpp) # if SDL
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
    ADD_COMPILE_DEFINITIONS(DEBUG)
    SET(GCC_GENERATE_DEBUGGING_SYMBOLS YES)
    SET(GCC_DEBUGGING_SYMBOLS full)
    SET(DEBUG_INFORMATION_FORMAT dwarf-with-dsym)
    SET(DWARF_DSYM_FILE_NAME "$(TARGET_NAME).dSYM")
    SET(DWARF_DSYM_FOLDER_PATH "$(CONFIGURATION_BUILD_DIR)/dSyms")
    SET(DEPLOYMENT_POSTPROCESSING YES)
endif ()


if (NOT RELEASE)
    ADD_COMPILE_FLAG("-fno-inline")

    ADD_COMPILE_FLAG(" -g3") # DEBUG
    if (NOT WASM)
        ADD_COMPILE_FLAG("-Werror") # WARNINGS AS ERRORS!  use -Wno-error=… for exceptions
    endif ()
    # enable:
    #    ADD_COMPILE_FLAG("-Wall")
    ADD_COMPILE_FLAG("-Wformat") # essential !! print("%s",string) => memory corruption!
    ADD_COMPILE_FLAG("-Wreturn-type") # VERY USEFUL : non-void function does not return a value
    ADD_COMPILE_FLAG("-Wreturn-stack-address") # VERY USEFUL : disallow stack memory associated return!
    ADD_COMPILE_FLAG("-Wunused-result") # VERY USEFUL : check for nodiscard, e.g. in non-self-modifying replace()
    # disable:
    ADD_COMPILE_FLAG("-Wno-unknown-pragmas")
    ADD_COMPILE_FLAG("-Wno-unused-function")
    ADD_COMPILE_FLAG("-Wno-unused-variable")
    ADD_COMPILE_FLAG("-Wno-unused-private-field")
    ADD_COMPILE_FLAG("-Wno-comment") # allow /* an old nested /* comment */ */
    ADD_COMPILE_FLAG("-Wno-typedef-redefinition")
    ADD_COMPILE_FLAG("-Wno-error=#warnings")
    ADD_COMPILE_FLAG("-Wno-writable-strings") # allow conversion from string literal to 'char *'
    ADD_COMPILE_FLAG("-Wno-unknown-attributes") # wasi
    ADD_COMPILE_FLAG("-Wno-ambiguous-reversed-operator") # useless
    ADD_COMPILE_FLAG("-Wno-undefined-bool-conversion") # if (!this)return false
    ADD_COMPILE_FLAG("-Wno-user-defined-literals") # allow "hi"s for string("hi")
    ADD_COMPILE_FLAG("-Wno-tautological-undefined-compare") # harmless if this == 0 check
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

ADD_COMPILE_DEFINITIONS(NO_CPP_DEMANGLE)

# COMMON FOR ALL
set(SOURCE_FILES ${SOURCE_FILES} source/String.cpp source/Node.cpp source/Map.cpp source/Wasp.cpp source/wasm_helpers.cpp source/Util.cpp)


if (RUNTIME_ONLY)
    ADD_COMPILE_DEFINITIONS(RUNTIME_ONLY)
    ADD_COMPILE_DEFINITIONS(NO_TESTS)
else ()
    message("NOT RUNTIME_ONLY")
    set(SOURCE_FILES ${SOURCE_FILES} source/Angle.cpp)
    set(SOURCE_FILES ${SOURCE_FILES} source/Interpret.cpp)
    set(SOURCE_FILES ${SOURCE_FILES} source/Code.cpp source/wasm_emitter.cpp)
    if (WABT_MERGE)
        set(SOURCE_FILES ${SOURCE_FILES} ./Frameworks/wabt/src/ ./source/wasm_merger_wabt.cpp ./source/wasm_reader_wabt.cpp)
        set(SOURCE_FILES ${SOURCE_FILES} ./Frameworks/wabt_merge/binary-reader-linker.cc)
    else ()
        set(SOURCE_FILES ${SOURCE_FILES} source/wasm_reader.cpp source/wasm_merger.cpp)
    endif ()

endif ()


#
# END OF GENERAL CONFIGURATION
#
# DIFFERENT TARGETS :
#
if (WASM) # OR WASI)
    message(WASM)
    ADD_COMPILE_DEFINITIONS(WASM)
    #    set(CMAKE_SYSROOT "") // via -DCMAKE_TOOLCHAIN_FILE=wasm.toolchain.cmake
    if (WASI)
        ADD_COMPILE_DEFINITIONS(WASI) # same as -DWASI=1
        set(CMAKE_ISYSROOT "") # typo? =>
        set(CMAKE_SYSROOT "")
        ADD_COMPILE_FLAG("--target=wasm32-wasi") # default # IF USING OTHER CLANG ... as before switch
    else ()
        ADD_COMPILE_FLAG("--target=wasm32-unknown-unknown-wasm") # default:wasi ok
        ADD_COMPILE_FLAG("-nostdlib -march=wasm") #-v
        #        ADD_LINK_OPTIONS("--no-entry")
    endif ()
    #    ADD_COMPILE_FLAG("-fvisibility=hidden -static-libsan -isystem ... -Oz -c ") # BREAKS BUILD! ???
    #    ADD_COMPILE_FLAG("-fvisibility=hidden -static-libsan ")
    ADD_COMPILE_FLAG("-o wasp.wasm") # DIRECT OUTPUT!
    #    ADD_COMPILE_FLAG("-nostdlib") # use our OWN memcpy (and printf?)
    #    ADD_COMPILE_FLAG("-fno-builtin")
    #    ADD_COMPILE_FLAG("-fno-exceptions") # cannot use 'throw' with exceptions disabled. ok better than typeinfo for char const
    # AVOID   -search_paths_first BUG! : if ( NOT WASM ) in Darwin.cmake (homebrew) or /Applications/CLion.app/Contents/bin/cmake/...
    #    OR  remove from CMAKE_CXX_LINK_FLAGS how
    #-lwasmer --entry=main,

    ADD_COMPILE_FLAG("-Wl,--demangle,--allow-undefined,--no-check-features") # wasm-ld flags forwarded from clang
    #   #--allow-undefined-file= ,--export-all FUCKS UP __class_type_info!
    #--import-memory        Import memory from the environment
    add_definitions(-DHEAP_OFFSET=65536)
    add_definitions(-DMEMORY_SIZE=117964800)
    ADD_COMPILE_FLAG("-Wl,--initial-memory=117964800") # stack size plus some
    ADD_COMPILE_FLAG("-Wl,-z,stack-size=104857600") # 1MB*10  pointers will START from high and go lower!
    ADD_COMPILE_FLAG("-Qn") # no producer metadata
    if (VERBOSE)
        ADD_COMPILE_FLAG("-Wl,-t,--verbose,--export-table -O0") # ,-O0  VERBOSE!
        #    ADD_COMPILE_FLAG("-Wl,--emit-relocs")
    endif ()
    if (DEBUG)
        #        ADD_COMPILE_FLAG("-O0")
    endif ()
    ADD_COMPILE_FLAG("-Wl,--whole-archive") #    Force load of all members in a static library
    ADD_COMPILE_FLAG("-Wl,--export-all") # preserve functions even if optimized
    ADD_COMPILE_FLAG("-Wl,--no-gc-sections") # keep name section?
    if (RELEASE)
        ADD_COMPILE_DEFINITIONS(NO_TESTS)
        #        ADD_COMPILE_FLAG("-Wl,--export-dynamic") #       Put symbols in the dynamic symbol table
        ADD_COMPILE_FLAG("-v")
        #        ADD_COMPILE_FLAG("-Wl,-vvv")
        #        ADD_COMPILE_FLAG("-save-temps=all") # keep intermediates   c stuff not wasm yet
        #          --reproduce=<value>    Dump linker invocation and input files for debugging

        # No support for creating shared libraries. The spec for shared libraries in WebAssembly is still in flux
        # https://github.com/rust-lang/rust/issues/60231#issuecomment-654645065
        #        ADD_COMPILE_FLAG("-Wl,--shared")
        #        ADD_COMPILE_FLAG("-Wl,--pie") #  position independent executable, does it help? NO
        #relocation R_WASM_MEMORY_ADDR_SLEB cannot be used against symbol nil_name; recompile with -fPIC
        #        ADD_COMPILE_FLAG("-fPIC") # does not help
        ADD_COMPILE_FLAG("-Wl,--compress-relocations,--strip-debug") # incompatible with output debug information. Please pass --strip-debug
        set(SOURCE_FILES ${SOURCE_FILES} source/exceptions.cpp)
        #        ADD_COMPILE_FLAG("-flto") # Add metadata for link-time optimizations => wasp.wasm only 30 kb (without tests) YAY
        #        --no-validation   --debug
        ADD_COMPILE_FLAG("-Oz -Qn -Ws,--lto-O3,-O4,--gc-sections") # RELEASE optimize size 130kb as module, 180kb all tests+emitter OK
        ADD_COMPILE_FLAG("-fno-inline") # helps https://developers.google.com/web/updates/2020/12/webassembly
        if (EMCC) # emscripten
            ADD_COMPILE_FLAG("-gseparate-dwarf=debug.wasm")
        endif ()
    endif ()
    #    add_library(wasp.wasm SHARED ${SOURCE_FILES}) # libwasp.wasm.so todo: rename
    add_executable(wasp.wasm ${SOURCE_FILES}) # same as add_library, with or without --export-all
elseif (WASI)
    #    TODO "System is unknown to cmake"
    message(WASI)
    ADD_COMPILE_DEFINITIONS(WASI) # same as -DWASI=1
    #    only works with wasm.toolchain.cmake
    SET(CMAKE_CXX_COMPILER /opt/wasm/wasi-sdk/bin/clang)
    SET(CMAKE_SYSROOT /opt/wasm/wasi-sdk/share/wasi-sysroot/)
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
    ADD_COMPILE_DEFINITIONS(WASM_ENABLE_INTERP)
    set(WAMR_BUILD_INTERP 1)
    SET(WAMR_ROOT_DIR wasm-micro-runtime)
    if (APPLE)
        add_definitions(-DBH_PLATFORM_DARWIN)
        SET(WAMR_BUILD_PLATFORM darwin)
    endif ()

    # Reset default linker flags
    set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")
    set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "")

    #    set(WAMR_BUILD_TARGET "X86_64")
    set(WAMR_BUILD_TARGET "AARCH64")
    set(WAMR_BUILD_LIBC_BUILTIN 1)

    ################  wasm-micro-runtime  ################
    include(${WAMR_ROOT_DIR}/build-scripts/runtime_lib.cmake)
    ################  application related  ################
    include(${SHARED_DIR}/utils/uncommon/shared_uncommon.cmake)

    add_library(vmlib ${WAMR_RUNTIME_LIB_SOURCE})
    # Build as X86_32 by default in 32-bit platform
    #    set (WAMR_BUILD_TARGET "X86_32")
    #    ADD_COMPILE_DEFINITIONS(WASM_ENABLE_FAST_INTERP)
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
            #            VERSUS /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk
            )
elseif (WABT)
    message(WABT)
    ADD_COMPILE_DEFINITIONS(WABT)
    ADD_COMPILE_FLAG("-dM -ferror-limit=5 -g")
    #    include(wabt/cmake_install.cmake)
    #    add_subdirectory(wabt)
    set(SOURCE_FILES ${SOURCE_FILES} source/wasm_runner_wabt.cpp)
    add_executable(wasp ${SOURCE_FILES})
    target_include_directories(wasp PUBLIC Frameworks/wabt)
    target_include_directories(wasp PUBLIC Frameworks/wabt/src)
    target_include_directories(wasp PUBLIC Frameworks/wabt/src/interp)
    target_link_libraries(wasp PRIVATE wasm)     #    libwabt is called libwasm.dylib DANGER!
elseif (WASMER)
    set(SOURCE_FILES ${SOURCE_FILES} source/wasmer_runner.cpp)
    add_executable(wasp ${SOURCE_FILES})
    target_include_directories(wasp PUBLIC Frameworks/wasmer-c-api)
    target_link_libraries(wasp PRIVATE wasmer)
elseif (WASMTIME)
    add_compile_definitions(WASMTIME)
    set(SOURCE_FILES ${SOURCE_FILES} source/wasmtime_runner.cpp)
    add_executable(wasp ${SOURCE_FILES})
    target_include_directories(wasp PUBLIC Frameworks/wasmtime-c-api/include/)
    target_link_libraries(wasp PRIVATE wasmtime)
elseif (WEBAPP)
    add_compile_definitions(WEBAPP)
    include(source/WebView.cmake)
    set(SOURCE_FILES ${SOURCE_FILES} source/WebView.cpp source/WebServer.cpp)
    add_executable(wasp ${SOURCE_FILES})
    target_link_libraries(wasp PUBLIC webview)
elseif (WASM3)
    MESSAGE(wasm3 VM)
    add_compile_definitions(WASM3)
    ADD_COMPILE_FLAG("-dM -ferror-limit=5 -g")
    include(Frameworks/wasm3/cmake_install.cmake)
    set(SOURCE_FILES ${SOURCE_FILES} source/wasm_runner_wasm3.cpp source/types/Number.cpp source/types/Number.h)
    add_subdirectory(Frameworks/wasm3/source ${CMAKE_BINARY_DIR}/m3)
    add_executable(wasp ${SOURCE_FILES})
    target_include_directories(wasp PUBLIC Frameworks/wasm3/source)
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
    set(SOURCE_FILES ${SOURCE_FILES} source/wasm_runner_v8.cpp source/Util.cpp source/Util.h source/wasm_patcher.cpp source/wasm_patcher.h)
    add_executable(wasp ${SOURCE_FILES})
    target_include_directories(wasp PUBLIC Frameworks/wasm-c-api/include/)
    target_include_directories(wasp PUBLIC Frameworks/wasm-c-api/v8/include/)
    target_link_libraries(wasp PRIVATE v8 v8_libplatform wasm)
else () # NORMAL:
    message(NO WASM "${WASM}")
    if (DEBUG)
        ADD_COMPILE_FLAG("-fsanitize=address -fsanitize-recover=address -fno-omit-frame-pointer  -shared-libasan ") # NOT FOR WASM
        #    ADD_COMPILE_FLAG("-rdynamic -Wl,--no-as-needed -ldl") # just for backtrace
    endif ()
    if (RELEASE)
        add_library(wasp SHARED ${SOURCE_FILES})
    else ()
        add_executable(wasp ${SOURCE_FILES})
    endif ()
    #    TARGET_LINK_LIBRARIES(wasp curl)
    #    add_custom_target(BuildWasp ALL DEPENDS angle)
endif ()


if (NOT WASM)
    if (WABT_MERGE)
        target_include_directories(wasp PUBLIC Frameworks/wabt)
        target_include_directories(${PROJECT_NAME} PUBLIC Frameworks/wabt/src)
        target_include_directories(${PROJECT_NAME} PUBLIC Frameworks/wabt_merge/)
        target_include_directories(${PROJECT_NAME} PUBLIC Frameworks/wabt_merge/tools)
        target_link_libraries(wasp PRIVATE wasm)     #    libwabt is called libwasm.dylib DANGER!

        #    target_include_directories(wasp PUBLIC Frameworks/wabt/src/interp)
    endif ()
endif ()


#set(CMAKE_CXX_FLAGS_RELEASE "/INCREMENTAL:YES ${CMAKE_CXX_FLAGS_RELEASE}")
set(CMAKE_SKIP_RULE_DEPENDENCY TRUE) # incremental build
set(CMAKE_SUPPRESS_REGENERATION)
