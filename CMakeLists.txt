# Copyright (c) Christopher Di Bella.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#

# Modified by Henry Zhou - 22 September 2021
# Reorganised the scripts to remove vcpkg dependencies.

cmake_minimum_required(VERSION 3.13)

project("GDWG-GRAPH-CPP" CXX)
add_library(${PROJECT_NAME} INTERFACE)

# Options
option(GRAPH_USE_LIBCXX "Build and test using libc++ as the standard library." ON)
option(GRAPH_ENABLE_TESTING "Enable testing of the gdwg library." ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/toolchains")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Set C++ Compiler
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS Off)
set(CMAKE_CXX_STANDARD_REQUIRED On)
set(CMAKE_EXPORT_COMPILE_COMMANDS On)
if (GRAPH_USE_LIBCXX)
    include(clang-libcxx)
else()
    include(gcc-libstdcxx)
endif()

# Import third-party packages
find_package(Catch2 CONFIG REQUIRED)

# Configure project
include(add-targets)
include_directories(include)
#add_subdirectory(src)

if (GRAPH_ENABLE_TESTING)
    enable_testing()
    include(CTest)
    add_subdirectory(test)
endif()
