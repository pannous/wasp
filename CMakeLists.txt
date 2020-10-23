cmake_minimum_required(VERSION 3.16)
project(wasp)

set(CMAKE_VERBOSE_MAKEFILE OFF) # DEBUG

# If <variable> is already set as a normal or cache variable, then the command does nothing!!
set(WASM 0) # OR :
set(WASI 0)
set(VM 0)
set(CMAKE_CXX_STANDARD 20)

#set(CMAKE_CXX_FLAGS_RELEASE "/INCREMENTAL:YES ${CMAKE_CXX_FLAGS_RELEASE}")
#set(CMAKE_SKIP_RULE_DEPENDENCY TRUE) # incremental build

FUNCTION(ADD_COMPILE_FLAG value)
    MESSAGE(STATUS "Building with ${value}")
    FOREACH(variable CMAKE_C_FLAGS CMAKE_CXX_FLAGS)
        SET(${variable} "${${variable}} ${value}" PARENT_SCOPE)
    ENDFOREACH(variable)
ENDFUNCTION()

#ADD_COMPILE_FLAG(" -w") # NO WARNINGS
#ADD_COMPILE_FLAG("-std=c++2a") #  -stdlib=libc++
if ( WASM )
    message(WASM)
    SET(CMAKE_CXX_COMPILER /opt/wasm/wasi-sdk/bin/clang)
    ADD_COMPILE_FLAG("-DWASM=1")
    ADD_COMPILE_FLAG("-nostdlib -march=wasm -w -g -O0") #-v
    ADD_COMPILE_FLAG("-o wasp.wasm") # DIRECT OUTPUT!
#    ADD_COMPILE_FLAG("-fvisibility=hidden -static-libsan -isystem -Oz -c ") # ???

#    ADD_COMPILE_FLAG("    --target=wasm32-unknown-unknown-wasm") # default:wasi ok
#    ADD_COMPILE_FLAG("    --target=wasm32-wasi") # default # IF USING OTHER CLANG ... as before switch

# AVOID   -search_paths_first BUG! : if ( NOT WASM ) in Darwin.cmake (homebrew) OR  remove from CMAKE_CXX_LINK_FLAGS

    ADD_COMPILE_FLAG("-Wl,--entry=main,--allow-undefined,--no-check-features,--demangle,-t") # wasm-ld flags forwarded from clang

#    ADD_COMPILE_FLAG("-Wl,-t,--verbose") # ,-O0  VERBOSE!
    ADD_COMPILE_FLAG("-Wl,--export-table,--emit-relocs") # DEBUG info and symbols!
#    ADD_COMPILE_FLAG("-Ws,-O4,--gc-sections") RELEASE strip maximum!

#   #--allow-undefined-file= ,--export-all FUCKS UP __class_type_info!

    set(SOURCE_FILES String.cpp Node.cpp Wasp.cpp WasmHelpers.cpp)
    add_executable(wasp.wasm ${SOURCE_FILES})
    add_custom_target(BuildWasm ALL DEPENDS wasp.wasm)
elseif(WASI)
    message(WASI)
    add_compile_definitions(WASI) # same as -DWASI=1
    SET(CMAKE_CXX_COMPILER /opt/wasm/wasi-sdk/bin/clang)
    SET(CMAKE_SYSROOT /opt/wasm/wasi-sdk/share/wasi-sysroot/)
    set(CMAKE_OSX_SYSROOT "")
#    SET(FIND_PATH /opt/wasm/wasi-sdk/share/wasi-sysroot/)
#    SET(CMAKE_FIND_ROOT_PATH /opt/wasm/wasi-sdk/share/wasi-sysroot/)
#    SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM ONLY)
#    SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
#    SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
#    set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

#    ADD_COMPILE_FLAG("-fno-exceptions") # cannot use 'throw' with exceptions disabled. ok better than typeinfo for char const
#    ADD_COMPILE_FLAG("-Wl,--entry=main,--allow-undefined") # __cxa_throw ?

    ADD_COMPILE_FLAG("-w -g -O0 -v ") # -nostdlib
    set(SOURCE_FILES String.cpp Node.cpp Wasp.cpp WasmHelpers.cpp exceptions.cpp)
    add_executable(wasp.wasm ${SOURCE_FILES})
    add_custom_target(BuildWasi ALL DEPENDS wasp.wasm)
elseif(VM) # NORMAL:
    add_compile_definitions(WASM_ENABLE_INTERP)
    add_compile_definitions(WASM_ENABLE_FAST_INTERP)
    message("USING INTERNAL WASM RUNTIME TO DEBUG")
    ADD_COMPILE_FLAG("-w -Oz")
#    ADD_COMPILE_FLAG("-dM -ferror-limit=5 -w -g -O0")#debug
    SET(WASMVM wasm_runtime/src/)
    SET(WASMVM_UTILS wasm_runtime/src/shared/utils/)
    SET(WASMVM_UTILS2 wasm_runtime/src/shared/utils/uncommon/)

    set(SOURCE_FILES String.cpp Node.cpp NodeTypes.h
            WasmHelpers.cpp wasm-emitter.cpp wasm_runner.cpp
            Wasp.cpp
            ${WASMVM}/wasm_runtime_common.c
            ${WASMVM}/wasm_native.c
            ${WASMVM}/wasm_runtime.c
#            ${WASMVM}/wasm_interp_classic.c
            ${WASMVM}/wasm_interp_fast.c
            ${WASMVM}/wasm_exec_env.c
            wasm_runtime/src/shared/platform/darwin/platform_init.c # vprintf
            wasm_runtime/src/wasm_runtime.c
#            core/iwasm/interpreter/wasm_interp.h
            ${WASMVM}/wasm_memory.c
            ${WASMVM}/wasm_loader.c
            ./wasm_runtime/core/shared/platform/common/posix/posix_malloc.c
            ./wasm_runtime/core/shared/platform/common/posix/posix_time.c
            ./wasm_runtime/core/shared/platform/common/posix/posix_memmap.c
            ./wasm_runtime/core/shared/platform/common/posix/posix_thread.c
            ./wasm_runtime/core/shared/utils/runtime_timer.c
            ./wasm_runtime/core/shared/utils/bh_common.c
            ./wasm_runtime/core/shared/mem-alloc/mem_alloc.c
            ./wasm_runtime/core/shared/mem-alloc/ems/ems_kfc.c
            ./wasm_runtime/core/shared/mem-alloc/ems/ems_alloc.c
            ./wasm_runtime/core/iwasm/common/arch/invokeNative_general.c
            ${WASMVM_UTILS}/bh_log.c
            ${WASMVM_UTILS2}/bh_read_file.c wasm_runner.h)
    add_executable(debug_wasm_emitter ${SOURCE_FILES})
    add_custom_target(BuildWart ALL DEPENDS debug_wasm_emitter) #.exe
    target_sources(debug_wasm_emitter PUBLIC wasm_runtime/src/shared/utils/uncommon/ wasm_runtime/src/shared/utils/ wasm_runtime/src/)
    target_include_directories(debug_wasm_emitter PUBLIC
            wasm_runtime/include wasm_runtime/core/iwasm/ wasm_runtime/core/iwasm/include/
            wasm_runtime/core/shared/utils wasm_runtime/core/shared/utils/uncommon
            wasm_runtime/core/shared/platform/include/
            wasm_runtime/core/shared/platform/darwin/ wasm_runtime/core/shared/mem-alloc)

else() # NORMAL:
    message(NO WASM "${WASM}")
    ADD_COMPILE_FLAG("-dM -ferror-limit=5 -g -w")
#    ADD_COMPILE_FLAG("-Wno-tautological-undefined-compare -Wno-user-defined-literals")
    # just for Backtrace.cpp :
#    ADD_COMPILE_FLAG("-g -rdynamic -Wl,--no-as-needed -ldl")
    #find_package( Readline)    #find_package( READLINE )
    #    ADD_COMPILE_FLAG(" -lreadline -ldl -lcurl -lz") # extra libraries
    set(SOURCE_FILES String.cpp Node.cpp NodeTypes.h Wasp.cpp WasmHelpers.cpp wasm-emitter.cpp Map.cpp )

#    set(SOURCE_FILES Node.h String.h String.cpp Node.cpp NodeTypes.h Wasp.cpp wasm-emitter.cpp Map.cpp)
    add_executable(angle ${SOURCE_FILES})
    TARGET_LINK_LIBRARIES(angle curl)
#    set(CMAKE_EXECUTABLE_SUFFIX ".exe")
    add_custom_target(BuildWasp ALL DEPENDS angle) #.exe
endif ()

#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY out)

# DISABLED WARNINGS
# -Wliteral-suffix -Wno-deprecated  -Wno-write-strings")  -Wunused-variable -Wall vs -w # NO WARNINGS -Werror") # all warnings are errors!
#ADD_COMPILE_FLAG(" -Wundefined-bool-conversion -Wno-writable-strings -Wno-dangling-else -Wundefined-bool-conversion -Wno-format-security")
#ADD_COMPILE_FLAG(" -Werror=sizeof-array-argument") # DOES NOT work to treat size-of-array-argument as array:(

set (CMAKE_SUPPRESS_REGENERATION)
