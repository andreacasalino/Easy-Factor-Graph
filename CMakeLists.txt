cmake_minimum_required(VERSION 3.0 FATAL_ERROR)

option(LIB_OPT "Compile shared libraries (ON) or static (OFF)" OFF)
option(BUILD_SAMPLES "Build the samples showing how to use EFG" ON)
option(BUILD_TESTS "" OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# set macro-directory and find scripts
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/")

# include cmake custom functions
set(WITH_SOURCE_TREE ON)
include(GroupSources)
include(AutoCollect)
include(MakeLibrary)

set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

project(EFG-Core)
option(THREAD_POOL_OPT "thread pool enabled (ON) or disabled (OFF)" ON)
add_subdirectory(Lib)

if(BUILD_SAMPLES)
	project(EFG-Samples)
    add_subdirectory(Samples)
endif()

if(BUILD_TESTS)
	project(EFG-Tests)
    add_subdirectory(Tests)
endif()

