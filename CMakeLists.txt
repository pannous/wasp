cmake_minimum_required(VERSION 3.16)
project(wasp)

set(CMAKE_VERBOSE_MAKEFILE OFF) # DEBUG

# If <variable> is already set as a normal or cache variable, then the command does nothing!!
set(RUNTIME_ONLY 0) # no Angle eval emit merge etc!
#set(VERBOSE 1)
set(DEBUG 1) # SET via Preferences… Build->CMake->Profiles
#set(RELEASE 1)
#set(WASM 1)
#set(WASI 1)
if (DEBUG)
    MESSAGE("DEBUG!")
    ADD_COMPILE_DEFINITIONS(DEBUG)
endif ()

#ADD_COMPILE_DEFINITIONS(NO_TESTS) # CAREFUL: most unused functions get optimized away

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
        #                set(MICRO 1) # NOT Mac M1 Arm64 ready!  wasm-micro-runtime => wasm_runner_micro.cpp
        #                set(WABT 1) # SLOOOW on Mac M1!  wasm_runner_wabt.cpp
        set(WASM3 1) # GOOD (backtrace…) wasm_runner_wasm3.cpp
        #        set(wasmx 1)	// doesn't work: cannot insert imports or function types!
    endif ()
endif ()


if (RELEASE)
    MESSAGE(RELEASE)
    set(RUNTIME_ONLY 1)
endif ()

set(CMAKE_CXX_STANDARD 20)


FUNCTION(ADD_COMPILE_FLAG value)
    MESSAGE(STATUS "Building ${TARGET} with ${value}")
    FOREACH (variable CMAKE_C_FLAGS CMAKE_CXX_FLAGS)
        SET(${variable} "${${variable}} ${value}" PARENT_SCOPE)
    ENDFOREACH (variable)
ENDFUNCTION()
ADD_COMPILE_FLAG("-Wno-unknown-pragmas")
ADD_COMPILE_FLAG("-Wno-typedef-redefinition")

if (NOT RELEASE)
    ADD_COMPILE_FLAG("-fno-inline")
    ADD_COMPILE_FLAG(" -g3") # DEBUG
    if (NOT WASM)
        ADD_COMPILE_FLAG("-Werror") # WARNINGS AS ERRORS!  use -Wno-error=… for exceptions
    endif ()
    # enable:
    ADD_COMPILE_FLAG("-Wformat") # essential !! print("%s",string) => memory corruption!
    ADD_COMPILE_FLAG("-Wreturn-type") # VERY USEFUL : non-void function does not return a value
    ADD_COMPILE_FLAG("-Wreturn-stack-address") # VERY USEFUL : disallow stack memory associated return!
    # disable:
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
    ADD_COMPILE_FLAG("-w") # hide warnings
    if (NOT WASM)
        ADD_COMPILE_FLAG("-Os -Oz") # NO WASM GENERATED wth. size should be ok? -O1 removes symbol names https://bugs.llvm.org/show_bug.cgi?id=45602 !
    endif ()
endif ()

ADD_COMPILE_DEFINITIONS(NO_CPP_DEMANGLE)

#target_include_directories(angle PUBLIC source)

# COMMON FOR ALL
set(SOURCE_FILES ${SOURCE_FILES} source/String.cpp source/Node.cpp source/Map.cpp source/Wasp.cpp source/Angle.cpp source/wasm_helpers.cpp)

if (RUNTIME_ONLY)
    ADD_COMPILE_DEFINITIONS(RUNTIME_ONLY)
else ()
    message(NOT RUNTIME_ONLY)
    set(SOURCE_FILES ${SOURCE_FILES} source/Angle.cpp source/Code.cpp source/wasm_emitter.cpp source/wasm_reader.cpp source/wasm_merger.cpp)
    set(SOURCE_FILES ${SOURCE_FILES} source/Interpret.cpp)
endif ()

#    TO EXECUTE wasm, add ONE of these: wasm_runner_console.cpp  wasm_runner_wasm3.cpp wasm_runner_wabt.cpp wasm_runner_wabt.cpp  wasm_runner_micro.cpp
if (WASMX)
    set(SOURCE_FILES ${SOURCE_FILES} source/wasm_runner_console.cpp)
endif ()

if (WASM) # OR WASI)
    message(WASM)
    ADD_COMPILE_DEFINITIONS(WASM)
    #    set(CMAKE_SYSROOT "") // via -DCMAKE_TOOLCHAIN_FILE=wasm.toolchain.cmake
    if (WASI)
        ADD_COMPILE_DEFINITIONS(WASI) # same as -DWASI=1
        set(CMAKE_ISYSROOT "")
        ADD_COMPILE_FLAG("--target=wasm32-wasi") # default # IF USING OTHER CLANG ... as before switch
    else ()
        ADD_COMPILE_FLAG("--target=wasm32-unknown-unknown-wasm") # default:wasi ok
        ADD_COMPILE_FLAG("-nostdlib -march=wasm") #-v
#        ADD_LINK_OPTIONS("--no-entry")
    endif ()
    #    ADD_COMPILE_FLAG("-fvisibility=hidden -static-libsan -isystem ... -Oz -c ") # BREAKS BUILD! ???
    #    ADD_COMPILE_FLAG("-fvisibility=hidden -static-libsan ")
    ADD_COMPILE_FLAG("-o wasp.wasm") # DIRECT OUTPUT!
    #    ADD_COMPILE_FLAG("-fno-builtin")
    #    ADD_COMPILE_FLAG("-nostdlib -fno-builtin") # use our OWN memcpy (and printf?)
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
        if (EMCC)
            ADD_COMPILE_FLAG("-gseparate-dwarf=debug.wasm")
        endif ()
    endif ()
    #    add_library(wasp.wasm SHARED ${SOURCE_FILES}) #
    add_executable(wasp.wasm ${SOURCE_FILES}) # same as add_library, with or without --export-all
    add_custom_target(BuildWasm ALL DEPENDS wasp.wasm)
elseif (WASI)
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

    add_custom_target(BuildEmitter ALL DEPENDS wasp) #.exe
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
    include(wabt/cmake_install.cmake)
    add_subdirectory(wabt)
    set(SOURCE_FILES ${SOURCE_FILES} source/wasm_runner_wabt.cpp)
    add_executable(angle ${SOURCE_FILES})
    target_include_directories(angle PUBLIC wabt)
    target_include_directories(angle PUBLIC wabt/src)
    target_include_directories(angle PUBLIC wabt/src/interp)
    target_link_libraries(angle PRIVATE libwabt) # libwabt.a 20 MB only for local stuff!
    #    add_custom_target(BuildWabtc ALL DEPENDS angle) #.exe
elseif (WASM3)
    message(wasm3 VM)
    ADD_COMPILE_FLAG("-dM -ferror-limit=5 -g")
    include(wasm3/cmake_install.cmake)
    set(SOURCE_FILES ${SOURCE_FILES} source/wasm_runner_wasm3.cpp)
    add_subdirectory(wasm3/source ${CMAKE_BINARY_DIR}/m3)
    add_executable(angle ${SOURCE_FILES})
    target_include_directories(angle PUBLIC wasm3/source)
    target_link_libraries(angle PRIVATE m3)
    #    add_custom_target(BuildWasm3 ALL DEPENDS angle) #.exe
else () # NORMAL:
    message(NO WASM "${WASM}")
    if (DEBUG)
        ADD_COMPILE_FLAG("-fsanitize=address -fsanitize-recover=address -fno-omit-frame-pointer  -shared-libasan ") # NOT FOR WASM
        #    ADD_COMPILE_FLAG("-rdynamic -Wl,--no-as-needed -ldl") # just for backtrace
    endif ()
    add_executable(angle ${SOURCE_FILES})
    #    TARGET_LINK_LIBRARIES(angle curl)
    #    add_custom_target(BuildWasp ALL DEPENDS angle)
endif ()

#set(CMAKE_CXX_FLAGS_RELEASE "/INCREMENTAL:YES ${CMAKE_CXX_FLAGS_RELEASE}")
set(CMAKE_SKIP_RULE_DEPENDENCY TRUE) # incremental build
set(CMAKE_SUPPRESS_REGENERATION)
