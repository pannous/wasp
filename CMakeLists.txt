cmake_minimum_required(VERSION 3.16)
project(wasp)


# If <variable> is already set as a normal or cache variable, then the command does nothing!!
set(WASM 1)
#add_compile_definitions(WASM)
#target_compile_definitions(my_target WASM WASM)
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_FLAGS_RELEASE "/INCREMENTAL:YES ${CMAKE_CXX_FLAGS_RELEASE}")
set(CMAKE_SKIP_RULE_DEPENDENCY TRUE) # incremental build

FUNCTION(ADD_COMPILE_FLAG value)
    MESSAGE(STATUS "Building with ${value}")
    FOREACH(variable CMAKE_C_FLAGS CMAKE_CXX_FLAGS)
        SET(${variable} "${${variable}} ${value}" PARENT_SCOPE)
    ENDFOREACH(variable)
ENDFUNCTION()

#ADD_COMPILE_FLAG(" -w") # NO WARNINGS
ADD_COMPILE_FLAG("-std=c++2a") #  -stdlib=libc++
#set (CMAKE_SUPPRESS_REGENERATION)
if ( WASM )
    message(WASM)
    ADD_COMPILE_FLAG("-DWASM=1 -Wno-main -fvisibility=hidden")
    ADD_COMPILE_FLAG("-w -isystem --target=wasm32-unknown-unknown-wasm -emit-llvm -Oz -c -o wasp.bc ")
    set(CMAKE_C_FLAGS "-s STANDALONE_WASM" )
    set(SOURCE_FILES String.cpp Node.cpp NodeTypes.h Wasp.cpp WasmHelpers.cpp)
#    set(SOURCE_FILES Node.h String.h String.cpp Node.cpp NodeTypes.h Wasp.cpp wasm-emitter.cpp Map.cpp)
    add_executable(wasp ${SOURCE_FILES})
    set(CMAKE_EXECUTABLE_SUFFIX ".wasm")
    add_custom_command ( OUTPUT wasp COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/build-wasm.sh WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/)
    add_custom_command ( OUTPUT wasp.wasm COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/build-wasm.sh WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/)
    add_custom_target(BuildWasm ALL DEPENDS wasp.wasm)

    #    set_target_properties(wasp PROPERTIES LINK_FLAGS "-s WASM=1 -s EXPORTED_FUNCTIONS='[_main]'")
else() # NORMAL:
    message(NO WASM "${WASM}")
    ADD_COMPILE_FLAG("-dM -ferror-limit=5 -g")
    # just for Backtrace.cpp :
    ADD_COMPILE_FLAG("-Wno-tautological-undefined-compare -Wno-user-defined-literals")
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

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY out)

# DISABLED WARNINGS
# -Wliteral-suffix -Wno-deprecated  -Wno-write-strings")  -Wunused-variable -Wall vs -w # NO WARNINGS -Werror") # all warnings are errors!
ADD_COMPILE_FLAG(" -Wundefined-bool-conversion -Wno-writable-strings -Wno-dangling-else -Wundefined-bool-conversion -Wno-format-security")
ADD_COMPILE_FLAG(" -Werror=sizeof-array-argument") # DOES NOT work to treat size-of-array-argument as array:(


# LIBRARIES
include_directories(/usr/include/)

# WASM:
#set(SOURCE_FILES String.h NodeTypes.h Wasp.cpp)
#not WASM: ErrorHandler.cpp  ErrorHandler.h
