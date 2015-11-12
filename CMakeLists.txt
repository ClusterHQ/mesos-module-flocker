project(mesos-module-flocker)

cmake_minimum_required(VERSION 2.8.4)
cmake_policy(SET CMP0042 NEW)

# Override output folder for conveniency.
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake-modules)

# Check that one of the officially supported compilers is used.
if(MSVC)
  message(STATUS "Supported compiler used: MSVC")
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  message(STATUS "Supported compiler used: GCC")
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  message(STATUS "Supported compiler used: Clang")
else()
  message("Unsupported compiler used: ${CMAKE_CXX_COMPILER_ID}")
endif(MSVC)

# Locate and enable Mesos, stout, and libprocess.
# NOTE(rukletsov): We do not regenerate protobufs, but reuse those from
# Mesos build folder. We also re-use boost bundled with Mesos to avoid
# funky issues.
# TODO(rukletsov): Improve FindMesos.cmake so that it supports both installed
# Mesos and dev build.
find_package(Mesos REQUIRED)
include_directories(${Mesos_INCLUDE_DIR} ${Stout_INCLUDE_DIR} ${Libprocess_INCLUDE_DIR})
include_directories(${Mesos_Boost_INCLUDE_DIR})
include_directories(${Mesos_BUILD_DIR}/include)
message(STATUS "Mesos library used: \"${Mesos_LIBRARIES}\"")

# Enable C++11 for Clang on MacOS X.
if(APPLE AND ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang"))
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++")
endif(APPLE AND ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang"))

# Isolator
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/isolator")