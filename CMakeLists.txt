cmake_minimum_required(VERSION 3.5)
project(mark)

set(CMAKE_CXX_STANDARD 11)

#set(CMAKE_CXX_FLAGS_RELEASE "/INCREMENTAL:YES ${CMAKE_CXX_FLAGS_RELEASE}")

set(CMAKE_SKIP_RULE_DEPENDENCY TRUE) # incremental build
#set (CMAKE_SUPPRESS_REGENERATION)


#find_package( Readline)
#find_package( READLINE )

#-std=c99 -Wall -j8
#set(CMAKE_C_FLAGS "-lreadline -lz -Wno-write-strings ")
set(CMAKE_CXX_FLAGS "-lreadline -lcurl -lz -Wno-write-strings -dM -Wno-deprecated ${CMAKE_CXX_FLAGS}")

set(SOURCE_FILES mark.cpp Node.cpp Node.h)
#String.cpp
#String.h


add_executable(mark ${SOURCE_FILES})

TARGET_LINK_LIBRARIES(mark curl)
