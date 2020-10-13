cmake_minimum_required(VERSION 3.16)
project(wasp)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_FLAGS_RELEASE "/INCREMENTAL:YES ${CMAKE_CXX_FLAGS_RELEASE}")

set(CMAKE_SKIP_RULE_DEPENDENCY TRUE) # incremental build

FUNCTION(ADD_COMPILE_FLAG value)
    MESSAGE(STATUS "Building with ${value}")
    FOREACH(variable CMAKE_C_FLAGS CMAKE_CXX_FLAGS)
        SET(${variable} "${${variable}} ${value}" PARENT_SCOPE)
    ENDFOREACH(variable)
ENDFUNCTION()


#set (CMAKE_SUPPRESS_REGENERATION)
#define_property(WASM )
if ( WASM )
    message(WASM)
    set(CMAKE_CXX_FLAGS "-DWASM=1 -Wno-main -fvisibility=hidden ${CMAKE_CXX_FLAGS}")
    set(CMAKE_CXX_FLAGS "-w -isystem --target=wasm32-unknown-unknown-wasm -emit-llvm -Oz -c -o wasp.bc ${CMAKE_CXX_FLAGS}")
    ADD_COMPILE_FLAG("--target=wasm32-unknown-unknown-wasm")
    ADD_COMPILE_FLAG("-emit-llvm")
    ADD_COMPILE_FLAG("-o wasp.bc")
else()
    # NORMAL:
    message(NO WASM "${WASM}")
    set(CMAKE_CXX_FLAGS " -dM -ferror-limit=5 -g ${CMAKE_CXX_FLAGS}")


    #    set(CMAKE_CXX_FLAGS " -lreadline -ldl -lcurl -lz ${CMAKE_CXX_FLAGS}") # extra libraries
endif ()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mmacosx-version-min=10.15" ) #initializer_list
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mmacosx-version-min=10.15" ) #initializer_list

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++2a  -stdlib=libc++ -I /usr/include/")

#find_package( Readline)
#find_package( READLINE )

# WASM:


# -I /usr/include/" # -nostdinc -nostdlib

# DISABLED WARNINGS
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w -Wliteral-suffix -Wno-deprecated  -Wno-write-strings")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wundefined-bool-conversion -Wno-writable-strings -Wno-dangling-else -Wundefined-bool-conversion -Wno-format-security")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mmacosx-version-min=10.7")

# LIBRARIES
include_directories(/usr/include/)
#include_directories(/usr/local/Cellar/llvm/10.0.0_3/include/c++/v1/)
#include_directories(/Library/Developer/CommandLineTools/usr/include/c++/v1/) # error: "No thread API"
#include_directories(/Library/Developer/CommandLineTools/SDKs/MacOSX10.15.sdk/usr/include/)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY out)
# just for Backtrace.cpp :
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -rdynamic -Wl,--no-as-needed -ldl")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -rdynamic -Wl -ldl")

# WASM:
#set(SOURCE_FILES String.h NodeTypes.h Wasp.cpp)
set(SOURCE_FILES Node.h String.h String.cpp Node.cpp NodeTypes.h Wasp.cpp wasm-emitter.cpp Map.cpp )
#String.h
#not WASM: ErrorHandler.cpp  ErrorHandler.h
#via Angle.cpp:
#mark.cpp
#String.cpp
#String.h
#Node.cpp Node.h

add_executable(wasp ${SOURCE_FILES})

TARGET_LINK_LIBRARIES(wasp curl)

#include_directories(.)
