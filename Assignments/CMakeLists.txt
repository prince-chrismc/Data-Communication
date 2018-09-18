cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
project(Data-Comm-Assignments)

# You can tweak some common (for all subprojects) stuff here. For example:

set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
set(CMAKE_DISABLE_SOURCE_CHANGES  ON)

if ("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
  message(SEND_ERROR "In-source builds are not allowed.")
endif ()

IF(CMAKE_BUILD_TYPE MATCHES Debug)
  message("Building 'Debug' configuration.")
ELSEIF(CMAKE_BUILD_TYPE MATCHES Release)
  message("Building 'Release' configuration.")
ELSE()
  set(CMAKE_BUILD_TYPE Release)
  message("Defaulting to 'Release' configuration.")
ENDIF()

# When done tweaking common stuff, configure the components (subprojects).
# NOTE: The order matters! The most independent ones should go first.
add_subdirectory(Simple-Sockets)
add_subdirectory(Cli-Parser)

include_directories("Simple-Sockets/examples/http/")
include_directories("Simple-Sockets/src/")
include_directories("Cli-Parser/src/")

FILE(GLOB HTTP "Simple-Sockets/examples/http/*")

if(UNIX)
    ADD_EXECUTABLE(Curl.run Curl/Curl.cpp ${HTTP})
    TARGET_LINK_LIBRARIES(Curl.run simple-socket cli-parser pthread)
elseif(WIN32)
    ADD_EXECUTABLE(Curl Curl/Curl.cpp ${HTTP})
    TARGET_LINK_LIBRARIES(Curl simple-socket cli-parser)
endif()
