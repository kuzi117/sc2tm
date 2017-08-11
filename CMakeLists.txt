cmake_minimum_required(VERSION 3.8)
project(sc2tm)

set(CMAKE_CXX_STANDARD 11)

# Determine our install directory
if (IS_DIRECTORY ${SC2TM_INSTALL_DIR})
  set(bin_dir ${SC2TM_INSTALL_DIR})
  message(STATUS "Using custom bin dir: ${bin_dir}")
else()
  set(bin_dir "${CMAKE_SOURCE_DIR}/bin")
  message(STATUS "Using default bin dir: ${bin_dir}")
endif()
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin )

find_package(Boost REQUIRED COMPONENTS system)
message(STATUS "Found Boost version: ${Boost_VERSION}")

include_directories("${CMAKE_SOURCE_DIR}/include")
include_directories(${Boost_INCLUDE_DIRS})

add_subdirectory(src)
