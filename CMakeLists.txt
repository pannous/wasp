cmake_minimum_required(VERSION 3.16)
project(wasp)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_FLAGS_RELEASE "/INCREMENTAL:YES ${CMAKE_CXX_FLAGS_RELEASE}")

set(CMAKE_SKIP_RULE_DEPENDENCY TRUE) # incremental build
#set (CMAKE_SUPPRESS_REGENERATION)


#find_package( Readline)
#find_package( READLINE )

# WASM:
#set(CMAKE_CXX_FLAGS "-DWASM -Wno-main -fvisibility=hidden ${CMAKE_CXX_FLAGS}")
#set(CMAKE_CXX_FLAGS "-w -isystem --target=wasm32-unknown-unknown-wasm -emit-llvm -Oz -c -o wasp.bc ${CMAKE_CXX_FLAGS}")

# NORMAL:
#set(CMAKE_CXX_FLAGS "-Wall -std=c++2a ${CMAKE_CXX_FLAGS} -I /usr/include/")


# -I /usr/include/" # -nostdinc -nostdlib

# DISABLED WARNINGS
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w -Wliteral-suffix -Wno-deprecated  -Wno-write-strings")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wundefined-bool-conversion -Wno-writable-strings -Wno-dangling-else -Wundefined-bool-conversion")

# LIBRARIES
#include_directories(/usr/include/)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY out)
set(CMAKE_CXX_FLAGS " -dM -ferror-limit=5 -g ${CMAKE_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS " -lreadline -ldl -lcurl -lz ${CMAKE_CXX_FLAGS}") # extra libraries
# just for Backtrace.cpp :
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -rdynamic -Wl,--no-as-needed -ldl")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -rdynamic -Wl -ldl")

# WASM:
#set(SOURCE_FILES String.h NodeTypes.h Wasp.cpp)
set(SOURCE_FILES NodeTypes.h Fetch.cpp String.h Wasp.cpp)
#not WASM: ErrorHandler.cpp  ErrorHandler.h
#via Angle.cpp:
#mark.cpp
#String.cpp
#String.h
#Node.cpp Node.h

add_executable(wasp ${SOURCE_FILES})

TARGET_LINK_LIBRARIES(wasp curl)

#include_directories(.)
