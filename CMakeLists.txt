cmake_minimum_required(VERSION 3.16)
project(wasp)

set(CMAKE_SYSROOT "")
#SET(CMAKE_SYSROOT /opt/wasm/wasi-sdk/share/wasi-sysroot/)


set(CMAKE_VERBOSE_MAKEFILE OFF) # DEBUG

# If <variable> is already set as a normal or cache variable, then the command does nothing!!
set(RUNTIME_ONLY 1) # no Angle eval emit merge etc!
#set(DEBUG 1)
set(RELEASE 1)

#set(WASM 1)
#set(WASI 1)

if (!WASM)
if (!RUNTIME_ONLY)
    # TO EXECUTE wasm:
    #set(VM 1) #  wasm-micro-runtime => wasm_runner_micro.cpp
    #set(WABT 1) #  wasm_runner_wabt.cpp
#    set(WASM3 1) # wasm_runner_wasm3.cpp
#    set(WASMX 1)
endif ()
endif ()

if (WASM)
    ADD_COMPILE_DEFINITIONS(MY_WASM) # use log() panic() ... !
    if (!WASI)
    endif ()
endif ()

if(WASI)
    set(WASM 1)
endif()

set(CMAKE_CXX_STANDARD 20)

#set(CMAKE_CXX_FLAGS_RELEASE "/INCREMENTAL:YES ${CMAKE_CXX_FLAGS_RELEASE}")
#set(CMAKE_SKIP_RULE_DEPENDENCY TRUE) # incremental build

if(RUNTIME_ONLY)
    ADD_COMPILE_DEFINITIONS(RUNTIME_ONLY)
endif()


FUNCTION(ADD_COMPILE_FLAG value)
    MESSAGE(STATUS "Building with ${value}")
    FOREACH (variable CMAKE_C_FLAGS CMAKE_CXX_FLAGS)
        SET(${variable} "${${variable}} ${value}" PARENT_SCOPE)
    ENDFOREACH (variable)
ENDFUNCTION()

include_directories(SYSTEM /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/usr/include)
# ^^ fixes <cmath> mac errors!

# enable:
if (!WASM)
    ADD_COMPILE_FLAG("-Werror") # WARNINGS AS ERRORS!  use -Wno-error=… for exceptions
endif ()
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
ADD_COMPILE_FLAG(" -g") # DEBUG
if(DEBUG)
#    ADD_COMPILE_FLAG(" -O0 -v ") # DEBUG lld
else()
    ADD_COMPILE_FLAG("-w") # hide warnings
    ADD_COMPILE_FLAG("-Wno-unused-command-line-argument") # -mmacosx-version-min=11.0
    ADD_COMPILE_FLAG(" -Os -Oz ") # size should be ok? -O1 removes symbol names https://bugs.llvm.org/show_bug.cgi?id=45602 !
endif ()

ADD_COMPILE_DEFINITIONS(NO_CPP_DEMANGLE)

set(SOURCE_FILES String.cpp Node.cpp Map.cpp Wasp.cpp Angle.cpp wasm_helpers.cpp) # COMMON FOR ALL
if (RUNTIME_ONLY)
else ()
    message(NOT RUNTIME_ONLY)
    set(SOURCE_FILES ${SOURCE_FILES} Angle.cpp Code.cpp wasm_emitter.cpp wasm_reader.cpp wasm_merger.cpp wasm_merger.h)
endif ()

#    TO EXECUTE wasm, add ONE of these: wasm_runner_console.cpp  wasm_runner_wasm3.cpp wasm_runner_wabt.cpp wasm_runner_wabt.cpp  wasm_runner_micro.cpp
if(WASMX)
    set(SOURCE_FILES ${SOURCE_FILES} wasm_runner_console.cpp)
endif ()

if (WASM) # OR WASI)
    message(WASM)
    SET(CMAKE_CXX_COMPILER /opt/wasm/wasi-sdk/bin/clang)
    ADD_COMPILE_DEFINITIONS(WASM)
    if (WASI)
        ADD_COMPILE_DEFINITIONS(WASI) # same as -DWASI=1
        SET(CMAKE_SYSROOT /opt/wasm/wasi-sdk/share/wasi-sysroot/)
        set(CMAKE_SYSROOT "")
        ADD_COMPILE_FLAG("--sysroot=/opt/wasi-sdk/share/wasi-sysroot")
    else ()
        ADD_COMPILE_FLAG("--target=wasm32-unknown-unknown-wasm") # default:wasi ok
        ADD_COMPILE_FLAG("-nostdlib -march=wasm") #-v
    endif ()
    #    ADD_COMPILE_FLAG("-fvisibility=hidden -static-libsan -isystem ... -Oz -c ") # BREAKS BUILD! ???
    #    ADD_COMPILE_FLAG("-fvisibility=hidden -static-libsan ")
    ADD_COMPILE_FLAG("-o wasp.wasm") # DIRECT OUTPUT!
    #    ADD_COMPILE_FLAG("-fno-builtin")
    #    ADD_COMPILE_FLAG("-nostdlib -fno-builtin") # use our OWN memcpy (and printf?)
    #    ADD_COMPILE_FLAG("-fno-exceptions") # cannot use 'throw' with exceptions disabled. ok better than typeinfo for char const

    #    ADD_COMPILE_FLAG("--target=wasm32-wasi") # default # IF USING OTHER CLANG ... as before switch
    # AVOID   -search_paths_first BUG! : if ( NOT WASM ) in Darwin.cmake (homebrew) OR  remove from CMAKE_CXX_LINK_FLAGS
    #-lwasmer
    ADD_COMPILE_FLAG("-Wl,--entry=main,--demangle,--allow-undefined,--no-check-features,-t") # wasm-ld flags forwarded from clang
    #   #--allow-undefined-file= ,--export-all FUCKS UP __class_type_info!
    add_definitions(-DHEAP_OFFSET=65536)
    #--import-memory        Import memory from the environment
    ADD_COMPILE_FLAG("-Wl,--initial-memory=11796480") # stack size plus some
    add_definitions(-DMEMORY_SIZE=11796480)
    ADD_COMPILE_FLAG("-Wl,-z,stack-size=10485760") # 1MB*10  pointers will START from high and go lower!
    #    ADD_COMPILE_FLAG("-Wl,-z,heap-size=1048576") # 1MB*10
    #    ADD_COMPILE_FLAG("-Wl,-z,stack-size=1073741824") # Set maximum stack size to 1GB / 8MiB 1073741824
    #    -flto \ # Add metadata for link-time optimizations
    if (DEBUG)
        ADD_COMPILE_FLAG("-Wl,-t,--verbose,--export-table") # ,-O0  VERBOSE!
        #    ADD_COMPILE_FLAG("-Wl,--emit-relocs")
    endif ()
    if (RELEASE)
        ADD_COMPILE_FLAG("-Oz -flto -Qn -Ws,--lto-O3,-O4,--gc-sections") # RELEASE optimize size 130kb as module, 180kb all tests+emitter OK
        ADD_COMPILE_FLAG("-fno-inline") # helps https://developers.google.com/web/updates/2020/12/webassembly
        #       ADD_COMPILE_FLAG("-gseparate-dwarf=debug.wasm")
    endif()
    add_executable(wasp.wasm ${SOURCE_FILES})
    add_custom_target(BuildWasm ALL DEPENDS wasp.wasm)
elseif (WASI)
    message(WASI)
    ADD_COMPILE_DEFINITIONS(WASI) # same as -DWASI=1
    SET(CMAKE_CXX_COMPILER /opt/wasm/wasi-sdk/bin/clang)
    SET(CMAKE_SYSROOT /opt/wasm/wasi-sdk/share/wasi-sysroot/)
    set(CMAKE_SYSROOT "")
    ADD_COMPILE_FLAG("--sysroot=/opt/wasi-sdk/share/wasi-sysroot")
    #  ADD_COMPILE_FLAG("-Wl,--entry=main,--allow-undefined") # __cxa_throw ?
    #${WAMR_ROOT_DIR}/wamr-sdk/app/libc-builtin-sysroot/include/ # stdbool :(
    add_executable(wasp.wasm ${SOURCE_FILES})
    add_custom_target(BuildWasi ALL DEPENDS wasp.wasm)
elseif (VM) # wasm-micro-vm:
    if (WASI)
        message(WASI and wasm-micro-vm no good!?)
        return()
    endif ()
    message("USING INTERNAL WASM MICRO RUNTIME TO DEBUG")
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

    set(WAMR_BUILD_TARGET "X86_64")
    set(WAMR_BUILD_LIBC_BUILTIN 1)

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
    #    ADD_COMPILE_DEFINITIONS(WASM_ENABLE_FAST_INTERP)
    #    ADD_COMPILE_FLAG("-w -Oz") # maximal optimization, CANT DEBUG!
    ADD_COMPILE_FLAG("-dM -ferror-limit=5 -w -g -O0")#debug
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
elseif (WABT) # NORMAL:
    message(WABT)
    ADD_COMPILE_DEFINITIONS(WABT)
    ADD_COMPILE_FLAG("-dM -ferror-limit=5 -g -w")
    include(wabt/cmake_install.cmake)
    add_subdirectory(wabt)
    set(SOURCE_FILES ${SOURCE_FILES} wasm_runner_wabt.cpp)
    add_executable(angle ${SOURCE_FILES})
    target_include_directories(angle PUBLIC wabt/)
    target_include_directories(angle PUBLIC wabt/src/)
    target_include_directories(angle PUBLIC wabt/src/interp)
    target_link_libraries(angle PRIVATE libwabt) # libwabt.a 20 MB only for local stuff!
    add_custom_target(BuildWabtc ALL DEPENDS angle) #.exe
elseif (WASM3) # NORMAL:
    message(WASM3 VM)
    ADD_COMPILE_FLAG("-dM -ferror-limit=5 -g -w")
    include(wasm3/cmake_install.cmake)
    set(SOURCE_FILES ${SOURCE_FILES} wasm_runner_wasm3.cpp)
    add_subdirectory(wasm3/source ${CMAKE_BINARY_DIR}/m3)
    add_executable(angle ${SOURCE_FILES})
    target_include_directories(angle PUBLIC wasm3/source/)
    target_link_libraries(angle PRIVATE m3)
    add_custom_target(BuildWasm3 ALL DEPENDS angle) #.exe
else () # NORMAL:
    message(NO WASM "${WASM}")
    ADD_COMPILE_FLAG("-fsanitize=address -fsanitize-recover=address -fno-omit-frame-pointer  -shared-libasan ") # NOT FOR WASM
    #    ADD_COMPILE_FLAG("-std=c++2a") #  -stdlib=libc++

    # just for Backtrace.cpp :
    #    ADD_COMPILE_FLAG("-g -rdynamic -Wl,--no-as-needed -ldl")

    #smart_types.cpp wasm_runner3.h NodeTypes.h wasm_merger.cpp
    add_executable(angle ${SOURCE_FILES})
    TARGET_LINK_LIBRARIES(angle curl)
    add_custom_target(BuildWasp ALL DEPENDS angle)
endif ()

#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY out)

# DISABLED WARNINGS
# -Wliteral-suffix -Wno-deprecated  -Wno-write-strings")  -Wunused-variable -Wall vs -w # NO WARNINGS -Werror") # all warnings are errors!
#ADD_COMPILE_FLAG(" -Wundefined-bool-conversion -Wno-writable-strings -Wno-dangling-else -Wundefined-bool-conversion -Wno-format-security")
#ADD_COMPILE_FLAG(" -Werror=sizeof-array-argument") # DOES NOT work to treat size-of-array-argument as array:(

set(CMAKE_SUPPRESS_REGENERATION)
