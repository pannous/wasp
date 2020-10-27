cmake_minimum_required(VERSION 3.16)
project(wasp)

set(CMAKE_VERBOSE_MAKEFILE OFF) # DEBUG

# If <variable> is already set as a normal or cache variable, then the command does nothing!!
set(WASM 0) # OR :
#if($ENV{WASI})
set(WASI 0)
set(VM 1)
set(CMAKE_CXX_STANDARD 20)

#set(CMAKE_CXX_FLAGS_RELEASE "/INCREMENTAL:YES ${CMAKE_CXX_FLAGS_RELEASE}")
#set(CMAKE_SKIP_RULE_DEPENDENCY TRUE) # incremental build

FUNCTION(ADD_COMPILE_FLAG value)
    MESSAGE(STATUS "Building with ${value}")
    FOREACH (variable CMAKE_C_FLAGS CMAKE_CXX_FLAGS)
        SET(${variable} "${${variable}} ${value}" PARENT_SCOPE)
    ENDFOREACH (variable)
ENDFUNCTION()

#ADD_COMPILE_FLAG(" -w") # NO WARNINGS
#ADD_COMPILE_FLAG("-std=c++2a") #  -stdlib=libc++
if (WASM)
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
elseif (WASI)
    message(WASI)
    add_compile_definitions(WASI) # same as -DWASI=1
    SET(CMAKE_CXX_COMPILER /opt/wasm/wasi-sdk/bin/clang)
    SET(CMAKE_SYSROOT /opt/wasm/wasi-sdk/share/wasi-sysroot/)
    set(CMAKE_OSX_SYSROOT "")
    set(CMAKE_SYSROOT "")
#    sd mv MacOSX.sdk.x ! WHY is CMAKE_OSX_SYSROOT still found???

    #    SET(FIND_PATH /opt/wasm/wasi-sdk/share/wasi-sysroot/)
    #    SET(CMAKE_FIND_ROOT_PATH /opt/wasm/wasi-sdk/share/wasi-sysroot/)
    #    SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM ONLY)
    #    SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
    #    SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
    #    set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

    #    ADD_COMPILE_FLAG("-fno-exceptions") # cannot use 'throw' with exceptions disabled. ok better than typeinfo for char const
    #    ADD_COMPILE_FLAG("-Wl,--entry=main,--allow-undefined") # __cxa_throw ?
#${WAMR_ROOT_DIR}/wamr-sdk/app/libc-builtin-sysroot/include/ # stdbool :(

    ADD_COMPILE_FLAG("-w -g -O0 -v ") # -nostdlib
    set(SOURCE_FILES String.cpp Node.cpp Wasp.cpp WasmHelpers.cpp exceptions.cpp)
    add_executable(wasp.wasm ${SOURCE_FILES})
    add_custom_target(BuildWasi ALL DEPENDS wasp.wasm)
elseif (VM) # NORMAL:
    if($ENV{WASI})
        return()
    endif()
    message("USING INTERNAL WASM RUNTIME TO DEBUG")
    add_compile_definitions(WASM_ENABLE_INTERP)
    set(WAMR_BUILD_INTERP 1)
    SET(WAMR_ROOT_DIR wasm-micro-runtime)
    if (APPLE)
        add_definitions(-DBH_PLATFORM_DARWIN)
        SET(WAMR_BUILD_PLATFORM darwin)
    endif ()

    # Reset default linker flags
    set (CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")
    set (CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "")

    set(WAMR_BUILD_TARGET "X86_64")
    set (WAMR_BUILD_LIBC_BUILTIN 1)

    ################  wasm-micro-runtime  ################
    include(${WAMR_ROOT_DIR}/build-scripts/runtime_lib.cmake)
    ################  application related  ################
    #    include_directories(${CMAKE_CURRENT_LIST_DIR}/src)
    include(${SHARED_DIR}/utils/uncommon/shared_uncommon.cmake)
    # why extra?
#    include(${WAMR_ROOT_DIR}/core/iwasm/libraries/libc-builtin/libc_builtin.cmake)

    add_library(vmlib ${WAMR_RUNTIME_LIB_SOURCE})
    # Build as X86_32 by default in 32-bit platform
    #    set (WAMR_BUILD_TARGET "X86_32")
    #    add_compile_definitions(WASM_ENABLE_FAST_INTERP)
#    ADD_COMPILE_FLAG("-w -Oz") # maximal optimization, CANT DEBUG!
    ADD_COMPILE_FLAG("-dM -ferror-limit=5 -w -g -O0")#debug
    set(SOURCE_FILES Wasp.cpp String.cpp Node.cpp NodeTypes.h
            WasmHelpers.cpp wasm-emitter.cpp wasm_runner.cpp
            )
    add_executable(wasp ${SOURCE_FILES} ${UNCOMMON_SHARED_SOURCE})

    if (APPLE)
        target_link_libraries(wasp vmlib -lm -ldl -lpthread)
    else ()
        target_link_libraries(wasp vmlib -lm -ldl -lpthread -lrt)
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
else () # NORMAL:
    message(NO WASM "${WASM}")
    ADD_COMPILE_FLAG("-dM -ferror-limit=5 -g -w")
    #    ADD_COMPILE_FLAG("-Wno-tautological-undefined-compare -Wno-user-defined-literals")
    # just for Backtrace.cpp :
    #    ADD_COMPILE_FLAG("-g -rdynamic -Wl,--no-as-needed -ldl")
    #find_package( Readline)    #find_package( READLINE )
    #    ADD_COMPILE_FLAG(" -lreadline -ldl -lcurl -lz") # extra libraries
    set(SOURCE_FILES String.cpp Node.cpp NodeTypes.h Wasp.cpp WasmHelpers.cpp wasm-emitter.cpp Map.cpp)

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

set(CMAKE_SUPPRESS_REGENERATION)
