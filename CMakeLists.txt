# Required CMake version
cmake_minimum_required(VERSION 3.28)

set(CMAKE_VERBOSE_MAKEFILE ON)

# Project name and languages
project(puzzles C)

# Disable in-place builds
if (${CMAKE_BINARY_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    MESSAGE(FATAL_ERROR "Error:  In-place builds are not supported. Please create a separate build directory")
endif (${CMAKE_BINARY_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})

# Fix for homebrew install of notcurses on macos
if (APPLE)
    include_directories("/opt/homebrew/opt/notcurses/include")
    link_directories("/opt/homebrew/opt/notcurses/lib")
endif ()

# Required libraries
find_package(Notcurses CONFIG REQUIRED)

# Find all the common sources recursively
file(GLOB_RECURSE SRC_CORE src/core/*.c)

# Define DEBUG macro for debug builds
add_compile_definitions("LOGGING=$<CONFIG:Debug>")

# Define individual puzzle executables
file(GLOB SRC_PUZZLES src/*.c)
foreach (puzzle_file ${SRC_PUZZLES})
    # get puzzle name by removing file extension and directory path
    get_filename_component(puzzle_name ${puzzle_file} NAME_WE)

    add_executable(${puzzle_name} ${puzzle_file})
    target_sources(${puzzle_name} PUBLIC ${SRC_CORE})
    target_link_libraries(${puzzle_name} PUBLIC Notcurses Notcurses-core)
endforeach ()

# Set up testing
enable_testing()
add_subdirectory(tests)