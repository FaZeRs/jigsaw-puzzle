cmake_minimum_required(VERSION 3.28)

# This warns about missing CMakeLists.txt file. We handle that ourselves if possible
cmake_policy(SET CMP0014 NEW)

# This warns about missing FindOpenMP.cmake (built-in cmake module) having "TRUE" string in if().
cmake_policy(SET CMP0012 NEW)

project(cpp_task)

# Enable C++23
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Options that are configurable through cmake
SET(ENABLE_PROFILE FALSE CACHE BOOL "Enable profiling flags (-pg)")
set(SANITIZE_MEMORY FALSE CACHE BOOL "Enable sanitizer for memory - cannot be used with thread sanitizer")
set(SANITIZE_THREAD FALSE CACHE BOOL "Enable sanitizer for thread - cannot be used with memory sanitizer")

if(SANITIZE_MEMORY)
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address -fsanitize=leak -fsanitize=undefined")
endif()

if(SANITIZE_THREAD)
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=thread")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -ltsan")
endif()

if(SANITIZE_THREAD AND SANITIZE_MEMORY)
  message(FATAL_ERROR "Cannot enable both SANITIZE_THREAD and SANITIZE_MEMORY!")
endif()

# Set output directory
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

#
# FLAGS START
#
# Set compilation flags
# Enable optimizations
# Default to release if user didn't change it
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "RELEASE")
endif()

string(TOLOWER ${CMAKE_BUILD_TYPE} BUILD_TYPE)

if(BUILD_TYPE STREQUAL "release")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -march=native -Wall -O3 -flto")
else()
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Og -ggdb -Wall")
endif()

if(ENABLE_PROFILE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg -g -fno-omit-frame-pointer")
  set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS} -pg -g -fno-omit-frame-pointer")
endif()

# Add OpenCV
find_package(OpenCV REQUIRED)
include_directories(SYSTEM ${OpenCV_INCLUDE_DIRS})

# Add directories for compilation - this has the compilation target
add_subdirectory(src)

# Print out status
message(STATUS " ")

message(STATUS "OpenCV library status:")
message(STATUS "    version: ${OpenCV_VERSION}")
message(STATUS "    libraries: ${OpenCV_LIBS}")
message(STATUS "    libraries path: ${OpenCV_LIBRARY_DIRS}")
message(STATUS "    include path: ${OpenCV_INCLUDE_DIRS}")
message(STATUS " ")

message(STATUS " ")

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Release cxx: ${CMAKE_CXX_FLAGS_RELEASE}")
message(STATUS "Debug cxx: ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS " ")

if(BUILD_TYPE STREQUAL "release")
  message(STATUS "Complete cxx: ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE}")
  message(STATUS "Complete ld: ${CMAKE_EXE_LINKER_FLAGS} ${CMAKE_EXE_LINKER_FLAGS_RELEASE}")
else()
  message(STATUS "Complete cxx: ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG}")
  message(STATUS "Complete ld: ${CMAKE_EXE_LINKER_FLAGS} ${CMAKE_EXE_LINKER_FLAGS_DEBUG}")
endif()