cmake_minimum_required(VERSION 3.16)
#SET(CMAKE_SYSROOT /opt/wasm/wasi-sdk/share/wasi-sysroot/)
set(CMAKE_SYSROOT "")

project(wasp)

set(CMAKE_VERBOSE_MAKEFILE OFF) # DEBUG

# If <variable> is already set as a normal or cache variable, then the command does nothing!!
set(DEBUG 1)
add_compile_definitions(DEBUG)
add_compile_definitions(NO_CPP_DEMANGLE)
#add_compile_definitions(MY_WASM) # use log() !
#set(WASM 1)
#set(WASI 1)
#set(VM 1)
set(WABT 1)
#set(WASM3 1)
set(CMAKE_CXX_STANDARD 20)

#    ADD_COMPILE_FLAG("-fsanitize=address -fno-omit-frame-pointer")
#set(CMAKE_CXX_FLAGS_RELEASE "/INCREMENTAL:YES ${CMAKE_CXX_FLAGS_RELEASE}")
#set(CMAKE_SKIP_RULE_DEPENDENCY TRUE) # incremental build

FUNCTION(ADD_COMPILE_FLAG value)
    MESSAGE(STATUS "Building with ${value}")
    FOREACH (variable CMAKE_C_FLAGS CMAKE_CXX_FLAGS)
        SET(${variable} "${${variable}} ${value}" PARENT_SCOPE)
    ENDFOREACH (variable)
ENDFUNCTION()

include_directories(SYSTEM /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/usr/include)
# ^^ fixes <cmath> mac errors!

#ADD_COMPILE_FLAG(" -w") # NO WARNINGS
#ADD_COMPILE_FLAG("-std=c++2a") #  -stdlib=libc++

ADD_COMPILE_FLAG("-Wno-writable-strings")
ADD_COMPILE_FLAG("-Wno-unused-command-line-argument") # -mmacosx-version-min=11.0
ADD_COMPILE_FLAG("-Wno-undefined-bool-conversion") # if (!this)return false
ADD_COMPILE_FLAG("-Wno-user-defined-literals") # allow "hi"s for string("hi")
ADD_COMPILE_FLAG("-Wreturn-type") # VERY USEFUL : non-void function does not return a value

#ADD_COMPILE_FLAG("-fsanitize=address -fno-omit-frame-pointer")
ADD_COMPILE_FLAG("-fsanitize=address -fsanitize-recover=address -fno-omit-frame-pointer  -shared-libasan ")
#add_compile_definitions(ASAN_OPTIONS=halt_on_error=0)
#ASAN_OPTIONS=strict_string_checks=1:detect_stack_use_after_return=1:check_initialization_order=1:strict_init_order=1
#add_compile_definitions(_FORTIFY_SOURCE)
#A: Doesn't work if  you link with -Wl,-z,defs or -Wl,--no-undefined.
# These flags don't work with ASan unless you also use -shared-libasan (which is the default mode for GCC, but not for Clang).


if (WASM) # OR WASI)
    message(WASM)
    SET(CMAKE_CXX_COMPILER /opt/wasm/wasi-sdk/bin/clang)
    ADD_COMPILE_FLAG("-DWASM=1")
    add_compile_definitions(WASM)
    if(WASI)
    add_compile_definitions(WASI) # same as -DWASI=1
    SET(CMAKE_SYSROOT /opt/wasm/wasi-sdk/share/wasi-sysroot/)
    set(CMAKE_SYSROOT "")
    ADD_COMPILE_FLAG("--sysroot=/opt/wasi-sdk/share/wasi-sysroot")
    else()
        ADD_COMPILE_FLAG("--target=wasm32-unknown-unknown-wasm") # default:wasi ok
    ADD_COMPILE_FLAG("-nostdlib -march=wasm -w") #-v
endif()
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
    #ADD_COMPILE_FLAG("-Wl,-t,--verbose,--export-table") # ,-O0  VERBOSE!
    ADD_COMPILE_FLAG(" -g -O0") # DEBUG
#    -Wl,-z,stack-size=$[8 * 1024 * 1024] \ # Set maximum stack size to 8MiB
#    -flto \ # Add metadata for link-time optimizations
#    ADD_COMPILE_FLAG("-Oz -flto -Qn -Ws,--lto-O3,-O4,--gc-sections") # RELEASE optimize size 130kb as module, 180kb all tests+emitter OK
#    ADD_COMPILE_FLAG("-Wl,--emit-relocs")
#    set(SOURCE_FILES String.cpp Node.cpp Wasp.cpp wasm_helpers.cpp) # module only
#    set(SOURCE_FILES String.cpp Node.cpp Wasp.cpp wasm_helpers.cpp Angle.cpp) # with interpreter
    set(SOURCE_FILES String.cpp Node.cpp Wasp.cpp wasm_helpers.cpp Angle.cpp wasm_emitter.cpp) # with compiler
    add_executable(wasp.wasm ${SOURCE_FILES})
    add_custom_target(BuildWasm ALL DEPENDS wasp.wasm)
elseif (WASI)
    message(WASI)
    add_compile_definitions(WASI) # same as -DWASI=1
    SET(CMAKE_CXX_COMPILER /opt/wasm/wasi-sdk/bin/clang)
    SET(CMAKE_SYSROOT /opt/wasm/wasi-sdk/share/wasi-sysroot/)
    set(CMAKE_SYSROOT "")
    ADD_COMPILE_FLAG("--sysroot=/opt/wasi-sdk/share/wasi-sysroot")
#    ADD_COMPILE_FLAG("-Wl,--entry=main,--allow-undefined") # __cxa_throw ?
#${WAMR_ROOT_DIR}/wamr-sdk/app/libc-builtin-sysroot/include/ # stdbool :(

    ADD_COMPILE_FLAG("-w -g -O0 -v ") # -nostdlib
    set(SOURCE_FILES String.cpp Node.cpp Wasp.cpp wasm_helpers.cpp exceptions.cpp )
    # smart_types.cpp
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
    set(SOURCE_FILES Angle.cpp Wasp.cpp String.cpp Node.cpp ErrorHandler.cpp Backtrace.cpp NodeTypes.h
            wasm_helpers.cpp wasm_emitter.cpp wasm_runner.cpp
            smart_types.cpp)
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
    ADD_COMPILE_FLAG("-dM -ferror-limit=5 -g -w")
    include(wabt/cmake_install.cmake)

    add_subdirectory(wabt)
    set(SOURCE_FILES
            Angle.cpp Wasp.cpp String.cpp Node.cpp ErrorHandler.cpp Backtrace.cpp NodeTypes.h
            wasm_helpers.cpp wasm_emitter.cpp wasm_reader.cpp )
    add_executable(angle ${SOURCE_FILES})
    target_include_directories(angle PUBLIC wabt/)
    target_include_directories(angle PUBLIC wabt/src/)
    target_link_libraries(angle PRIVATE libwabt) # libwabt.a 20 MB only for local stuff!
    add_custom_target(BuildWabtc ALL DEPENDS angle) #.exe

elseif (WASM3) # NORMAL:
    message(WASM3 VM)
    ADD_COMPILE_FLAG("-dM -ferror-limit=5 -g -w")
    include(wasm3/cmake_install.cmake)

    add_subdirectory(wasm3/source ${CMAKE_BINARY_DIR}/m3)
    set(SOURCE_FILES
            Angle.cpp Wasp.cpp String.cpp Node.cpp ErrorHandler.cpp Backtrace.cpp NodeTypes.h
            wasm_helpers.cpp wasm_emitter.cpp wasm3_runner.cpp
            smart_types.cpp wasm_reader.cpp)
    add_executable(angle ${SOURCE_FILES})
    target_include_directories(angle PUBLIC wasm3/source/)
    target_link_libraries(angle PRIVATE m3)
    add_custom_target(BuildWasm3 ALL DEPENDS angle) #.exe
else () # NORMAL:
    message(NO WASM "${WASM}")
    ADD_COMPILE_FLAG("-dM -ferror-limit=5 -g -w")
    #    ADD_COMPILE_FLAG("-Wno-tautological-undefined-compare -Wno-user-defined-literals")
    # just for Backtrace.cpp :
    #    ADD_COMPILE_FLAG("-g -rdynamic -Wl,--no-as-needed -ldl")
    #find_package( Readline)    #find_package( READLINE )
    #    ADD_COMPILE_FLAG(" -lreadline -ldl -lcurl -lz") # extra libraries
    set(SOURCE_FILES String.cpp Node.cpp NodeTypes.h Wasp.cpp wasm_helpers.cpp wasm_emitter.cpp Map.cpp smart_types.cpp wasm3_runner.cpp wasm3_runner.h wasm_reader.cpp wasm_reader.h)

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
