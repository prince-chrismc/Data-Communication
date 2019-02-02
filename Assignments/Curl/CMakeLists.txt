cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
project(Curl)

include_directories("../Simple-Sockets/src/")
include_directories("../Cli-Parser/src/")

# HTTP Library
FILE(GLOB HTTP "../http/*")

# Curl - Assingment 1
FILE(GLOB CURL "src/*")

ADD_EXECUTABLE(Curl Curl.cpp ${CURL} ${HTTP})
target_include_directories(Curl PRIVATE src/ ../http/)
TARGET_LINK_LIBRARIES(Curl Simple-Socket Cli-Parser)
