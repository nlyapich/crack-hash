cmake_minimum_required(VERSION 3.20)

if(EXISTS "${CMAKE_BINARY_DIR}/build/Release/generators/conan_toolchain.cmake ")
    include("${CMAKE_BINARY_DIR}/build/Release/generators/conan_toolchain.cmake ")
endif()

project(CrackHashCpp VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/common")
    add_subdirectory(common)
endif()
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/manager")
    add_subdirectory(manager)
endif()
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/worker")
    add_subdirectory(worker)
endif()
