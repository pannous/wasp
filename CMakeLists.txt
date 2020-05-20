cmake_minimum_required(VERSION 3.5)
project(mark)

set(CMAKE_CXX_STANDARD 20)

#set(CMAKE_CXX_FLAGS_RELEASE "/INCREMENTAL:YES ${CMAKE_CXX_FLAGS_RELEASE}")

set(CMAKE_SKIP_RULE_DEPENDENCY TRUE) # incremental build
#set (CMAKE_SUPPRESS_REGENERATION)


#find_package( Readline)
#find_package( READLINE )

#-std=c99 -Wall -j8 -std=c++2a

# DISABLED WARNINGS
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w -Wliteral-suffix -Wno-deprecated  -Wno-write-strings")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wundefined-bool-conversion -Wno-writable-strings -Wno-dangling-else -Wundefined-bool-conversion")

# LIBRARIES
set(CMAKE_CXX_FLAGS " -dM -g ${CMAKE_CXX_FLAGS}")
#set(CMAKE_CXX_FLAGS " -lreadline -ldl -lcurl -lz ${CMAKE_CXX_FLAGS}") # extra libraries
# just for Backtrace.cpp :
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -rdynamic -Wl,--no-as-needed -ldl")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -rdynamic -Wl -ldl")

set(SOURCE_FILES ErrorHandler.cpp ErrorHandler.h String.h NodeTypes.h Angle.cpp)
#via Angle.cpp:
#mark.cpp
#String.cpp
#String.h
#Node.cpp Node.h


add_executable(mark ${SOURCE_FILES})

TARGET_LINK_LIBRARIES(mark curl)
