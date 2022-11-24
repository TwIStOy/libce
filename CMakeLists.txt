cmake_minimum_required(VERSION 3.12)

include(CheckCXXCompilerFlag)

# Fallback for using newer policies on CMake <3.12.
if(${CMAKE_VERSION} VERSION_LESS 3.12)
  cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()

if(NOT DEFINED CE_MASTER_PROJECT)
  set(CE_MASTER_PROJECT OFF)
  if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(CE_MASTER_PROJECT ON)
    message(STATUS "CMake Version: ${CMAKE_VERSION}")
    message(STATUS "MASTER PROJECT NOW")
  endif()
endif()

option(CE_BUILD_TESTS "Build tests" ${CE_MASTER_PROJECT})
option(CE_BUILD_EXAMPLES "Build examples" ${CE_MASTER_PROJECT})
option(CE_BUILD_BENCHMARKS "Build benchmarks" ${CE_MASTER_PROJECT})
option(CE_EXTERNAL_LIBEVENT "Use system libevent" OFF)
option(CE_INSTALL "Generate install targets" ${CE_MASTER_PROJECT})

project(libce LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# add custom cmake scripts
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

add_library(ce)
target_include_directories(ce PUBLIC ${PROJECT_SOURCE_DIR}/include)

file(GLOB_RECURSE CE_SOURCE_FILES ./src/*.cc)
target_sources(ce PRIVATE ${CE_SOURCE_FILES})

