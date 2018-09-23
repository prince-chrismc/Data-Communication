cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
project(Data-Comm-Assignments)

# You can tweak some common (for all subprojects) stuff here. For example:
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
set(CMAKE_DISABLE_SOURCE_CHANGES  ON)

if ("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
  message(SEND_ERROR "In-source builds are not allowed.")
endif ()

# OS and compiler checks.
if(UNIX)
    # linux / normal unix
    add_definitions(-D_LINUX)
    set(CMAKE_CXX_STANDARD 17)

    if(CMAKE_BUILD_TYPE MATCHES Debug)
      message("Building 'Debug' configuration.")
    elseif(CMAKE_BUILD_TYPE MATCHES Release)
      message("Building 'Release' configuration.")
    else()
      set(CMAKE_BUILD_TYPE Release)
      message("Defaulting to 'Release' configuration.")
    endif()

    if(CYGWIN)
        # Special Cygwin stuff here
    elseif(APPLE)
        # Special Apple stuff here
        remove_definitions(-D_LINUX)
        add_definitions(-D_DARWIN)
    endif()
elseif(WIN32)
    add_definitions(-D_WIN32)
    if(MINGW)
        # Special MINGW stuff here
    elseif(MSVC)
        # Special MSVC stuff here
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
    else()
        # No idea what it is, but there's a good chance it's too weird.
        MESSAGE( FATAL_ERROR "Using unknown WIN32 compiler... NOT. Please add to build system." )
    endif()
endif()

# When done tweaking common stuff, configure the components (subprojects).
set(SIMPLE_SOCKET_DEP_ONLY true CACHE BOOL "" FORCE)
set(SIMPLE_SOCKET_EXAMPLES false CACHE BOOL "" FORCE)
set(CLI_PARSER_DEP_ONLY true CACHE BOOL "" FORCE)

# NOTE: The order matters! The most independent ones should go first.
add_subdirectory(Simple-Sockets)
add_subdirectory(Cli-Parser)

include_directories("Simple-Sockets/examples/http/")
include_directories("Simple-Sockets/src/")
include_directories("Cli-Parser/src/")
include_directories("Curl/src/")

FILE(GLOB HTTP "Simple-Sockets/examples/http/*")
FILE(GLOB CURL "Curl/src/*")

if(UNIX)
    ADD_EXECUTABLE(Curl.run Curl ${CURL} ${HTTP})
    TARGET_LINK_LIBRARIES(Curl.run simple-socket Cli-Parser pthread)
elseif(WIN32)
    ADD_EXECUTABLE(Curl Curl/Curl.cpp ${CURL} ${HTTP})
    TARGET_LINK_LIBRARIES(Curl simple-socket Cli-Parser)
endif()
