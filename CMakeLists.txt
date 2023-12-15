cmake_minimum_required(VERSION 3.0 FATAL_ERROR)

option(LIB_OPT "Compile shared libraries (ON) or static (OFF)" OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/MakeLibrary.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/Fetch.cmake)

set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

project(EFG-Core)

set(EFG_LIB_NAME EFG-Core)
add_subdirectory(src)

option(BUILD_EFG_SAMPLES "Build the samples showing how to use EFG" ON)
if(BUILD_EFG_SAMPLES)
    add_subdirectory(samples)
endif()

option(BUILD_EFG_TESTS "" OFF)
if(BUILD_EFG_TESTS)
    add_subdirectory(tests)
endif()

