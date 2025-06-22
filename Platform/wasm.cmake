set(CMAKE_SYSTEM_NAME WASM)
set(CMAKE_SYSTEM_PROCESSOR wasm)
set(CMAKE_CROSSCOMPILING TRUE)

# Specify the compiler
set(CMAKE_C_COMPILER_ID Clang)
set(CMAKE_CXX_COMPILER_ID Clang)
set(CMAKE_ASM_COMPILER_ID Clang)

# Configure the target environment
set(WASM 1)
set(CMAKE_EXECUTABLE_SUFFIX ".wasm")
set(CMAKE_STATIC_LIBRARY_PREFIX "")
set(CMAKE_STATIC_LIBRARY_SUFFIX ".wasm")
set(CMAKE_SHARED_LIBRARY_PREFIX "")
set(CMAKE_SHARED_LIBRARY_SUFFIX ".wasm")

# Set compile and link flags for WASM
set(CMAKE_C_FLAGS_INIT "-target wasm32-unknown-unknown")
set(CMAKE_CXX_FLAGS_INIT "-target wasm32-unknown-unknown")
set(CMAKE_EXE_LINKER_FLAGS_INIT "-target wasm32-unknown-unknown")

# Prevent CMake from testing the compiler
set(CMAKE_C_COMPILER_FORCED TRUE)
set(CMAKE_CXX_COMPILER_FORCED TRUE)

# Set search paths
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)