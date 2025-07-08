# doesn't really work?

# HOW TO SOLVE?
#System is unknown to cmake, create:
#Platform/wasm to use this system, please post your config file on discourse.cmake.org so it can be added to cmake
#Your CMakeCache.txt file was copied to CopyOfCMakeCache.txt. Please post that file on discourse.cmake.org.


#set (CMAKE_SYSTEM_NAME "Generic")
set(CMAKE_SYSTEM_NAME wasm)
set(CMAKE_SYSTEM_PROCESSOR wasm)

set(SDK /opt/wasm/wasi-sdk/)
set(CMAKE_C_COMPILER ${SDK}/bin/clang)
set(CMAKE_CXX_COMPILER ${SDK}/bin/clang)

set(CMAKE_SYSROOT /opt/wasm/wasi-sdk/share/wasi-sysroot/)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

set(HAVE_FLAG_SEARCH_PATHS_FIRST 0)
set(CMAKE_C_LINK_FLAGS "")
set(CMAKE_CXX_LINK_FLAGS "")
