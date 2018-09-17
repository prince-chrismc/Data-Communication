cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
project(Data-Comm-Assignments)

# You can tweak some common (for all subprojects) stuff here. For example:

set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
set(CMAKE_DISABLE_SOURCE_CHANGES  ON)

if ("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
  message(SEND_ERROR "In-source builds are not allowed.")
endif ()

# Remove 'lib' prefix for shared libraries on Windows
if (WIN32)
  set(CMAKE_SHARED_LIBRARY_PREFIX "")
endif ()

# When done tweaking common stuff, configure the components (subprojects).
# NOTE: The order matters! The most independent ones should go first.
add_subdirectory(Simple-Sockets)
add_subdirectory(Cli-Parser)

include_directories("Simple-Sockets/examples/http/")

if(UNIX)
    ADD_EXECUTABLE(Curl.run Curl/Curl.cpp)
    TARGET_LINK_LIBRARIES(Curl.run simple-socket cli-parser pthread)
elseif(WIN32)
    ADD_EXECUTABLE(Curl Curl/Curl.cpp)
    TARGET_LINK_LIBRARIES(Curl simple-socket cli-parser)
endif()
