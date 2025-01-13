# Required CMake version
cmake_minimum_required(VERSION 3.28)

# Project name and languages
project(puzzles C)

# Disable in-place builds
if (${CMAKE_BINARY_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    MESSAGE(FATAL_ERROR "Error:  In-place builds are not supported. Please create a separate build directory")
endif (${CMAKE_BINARY_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})

# Find all the common sources recursively
file(GLOB_RECURSE SRC_CORE src/core/*.c)

# Define individual puzzle executables
file(GLOB SRC_PUZZLES src/*.c)
foreach(puzzle_file ${SRC_PUZZLES})
    # get puzzle name by removing file extension and directory path
    get_filename_component(puzzle_name ${puzzle_file} NAME_WE)

    add_executable(${puzzle_name} ${puzzle_file})
    target_sources(${puzzle_name} PRIVATE ${SRC_CORE})
endforeach()
