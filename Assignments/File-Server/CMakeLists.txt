cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
project(File-Server)

include_directories("../Simple-Sockets/src/")
include_directories("../Cli-Parser/src/")

# HTTP Library
FILE(GLOB HTTP "../Simple-Sockets/examples/http/*")

# File Server - Assingment 2
FILE(GLOB FS "src/*")

ADD_EXECUTABLE(File-Server File-Server.cpp ${FS} ${HTTP})
target_include_directories(File-Server PRIVATE src ../Simple-Sockets/examples/http/)
if(UNIX)
    TARGET_LINK_LIBRARIES(File-Server Simple-Socket Cli-Parser stdc++fs pthread)
else()
    TARGET_LINK_LIBRARIES(File-Server Simple-Socket Cli-Parser)
endif()
