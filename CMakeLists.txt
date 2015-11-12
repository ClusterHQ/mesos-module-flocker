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
  add_definitions(-std=gnu++0x)
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  message(STATUS "Supported compiler used: Clang")
else()
  message("Unsupported compiler used: ${CMAKE_CXX_COMPILER_ID}")
endif(MSVC)

find_package(Mesos REQUIRED)
include_directories(${Mesos_INCLUDE_DIR} ${Stout_INCLUDE_DIR} ${Libprocess_INCLUDE_DIR})
include_directories(${Mesos_Boost_INCLUDE_DIR})
include_directories(${Mesos_protobuf_INCLUDE_DIR})
include_directories(${Mesos_glog_INCLUDE_DIR})
include_directories(${Mesos_picojson_INCLUDE_DIR})
include_directories(${Mesos_BUILD_DIR}/include)
include_directories(${Mesos_BUILD_DIR}/src)
include_directories(${Mesos_SRC_DIR})
message(STATUS "Mesos library used: \"${Mesos_LIBRARIES}\"")

# Enable C++11 for Clang on MacOS X.
if(APPLE AND ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang"))
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++")
endif(APPLE AND ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang"))

# Isolator
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/isolator")