cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
project(Data-Comm-Assignments)

# CMake Settings
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
set(CMAKE_DISABLE_SOURCE_CHANGES  ON)

if ("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
  message(SEND_ERROR "In-source builds are not allowed.")
endif ()

#set(CMAKE_VERBOSE_MAKEFILE ON)

# OS and compiler checks.
if(UNIX)
    add_definitions(-D_LINUX)
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_EXECUTABLE_SUFFIX .run)
    set(THREAD_LIB pthread)
    set(FILESYSTEM_LIB stdc++fs)

    if(CMAKE_BUILD_TYPE MATCHES Debug)
      add_compile_options(-g -D_DEBUG)
    elseif(CMAKE_BUILD_TYPE MATCHES Release)
      add_compile_options(-O3)
    else()
      set(CMAKE_BUILD_TYPE Release)
      message("Defaulting to 'Release' configuration.")
    endif()

    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        add_definitions(-D_CLANG)
    endif()

    #add_compile_options(-Wall -Wextra)

    if(APPLE)
        remove_definitions(-D_LINUX)
        add_definitions(-D_DARWIN)
    endif()
elseif(WIN32)
    add_definitions(-D_WIN32)

    if(MSVC)
        add_compile_options(/std:c++17 -W4)
    else()
        message( FATAL_ERROR "Using unknown WIN32 compiler... NOT. Please add to build system." )
    endif()
endif()

# When done tweaking common stuff, configure the components (subprojects).

# NOTE: The order matters! The most independent ones should go first.
add_subdirectory(Simple-Sockets EXCLUDE_FROM_ALL)
add_subdirectory(Cli-Parser)

include_directories("Simple-Sockets/src/")
include_directories("Cli-Parser/src/")

# HTTP Library
FILE(GLOB HTTP "Simple-Sockets/examples/http/*")

# Curl - Assingment 1
FILE(GLOB CURL "Curl/src/*")

# File Server - Assingment 2
FILE(GLOB FS "File-Server/src/*")

# httpc && httpfs - Assingment 3
FILE(GLOB TP_SOURCE "Text-Protocol/src/*")
FILE(GLOB TP_CLIENT "Text-Protocol/Client/src/*")
FILE(GLOB TP_SERVER "Text-Protocol/Server/src/*")
FILE(GLOB SERVLETS "File-Server/src/*Servlet*")

ADD_LIBRARY(Text-Protocol STATIC ${TP_SOURCE})
target_include_directories(Text-Protocol PRIVATE Text-Protocol/src/)

ADD_EXECUTABLE(Curl Curl/Curl.cpp ${CURL} ${HTTP})
target_include_directories(Curl PRIVATE Curl/src/ Simple-Sockets/examples/http/)
TARGET_LINK_LIBRARIES(Curl Simple-Socket Cli-Parser)

ADD_EXECUTABLE(File-Server File-Server/File-Server.cpp ${FS} ${HTTP})
target_include_directories(File-Server PRIVATE File-Server/src Simple-Sockets/examples/http/)
TARGET_LINK_LIBRARIES(File-Server Simple-Socket Cli-Parser ${FILESYSTEM_LIB} ${THREAD_LIB})

ADD_EXECUTABLE(Text-Protocol-Client Text-Protocol/Client/Main.cpp Curl/src/Href.cpp Curl/src/Href.h ${TP_CLIENT} ${HTTP})
target_include_directories(Text-Protocol-Client PRIVATE Text-Protocol/Client/src/ Text-Protocol/src Simple-Sockets/examples/http/)
TARGET_LINK_LIBRARIES(Text-Protocol-Client Text-Protocol Simple-Socket Cli-Parser)

ADD_EXECUTABLE(Text-Protocol-Server Text-Protocol/Server/Main.cpp ${TP_SERVER} ${SERVLETS} ${HTTP})
target_include_directories(Text-Protocol-Server PRIVATE Text-Protocol/Server/src Text-Protocol/src File-Server/src/ Simple-Sockets/examples/http/)
TARGET_LINK_LIBRARIES(Text-Protocol-Server Text-Protocol Simple-Socket Cli-Parser ${FILESYSTEM_LIB} ${THREAD_LIB})
