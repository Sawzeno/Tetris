cmake_minimum_required(VERSION 3.10)
project(game)

# Set the compiler to Clang
set(CMAKE_C_COMPILER "clang")
set(CMAKE_CXX_COMPILER "clang++")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror=vla")

message(STATUS "Compiler being used: ${CMAKE_C_COMPILER}")

# Automatically add all source files from the src directory
file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")

# Create a static library from the source files
add_library(game_lib STATIC ${SOURCES})

# Include directories for the game library
target_include_directories(game_lib
PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Create an executable for the game using the main source file
add_executable(game ${CMAKE_CURRENT_SOURCE_DIR}/src/main.c)
target_link_libraries(game PRIVATE game_lib)
