cmake_minimum_required(VERSION 3.10)
project(tests)

# Include directories from the game project
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../game/src)

# Find source files in the tests directory
file(GLOB ROOT_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
file(GLOB SUBDIR_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*/*.c")

set(NON_TEST_SOURCES "")
set(TEST_SOURCES "")

# Separate test and non-test sources
foreach(sourcefile ${ROOT_SOURCES} ${SUBDIR_SOURCES})
    get_filename_component(filename ${sourcefile} NAME)
    if(filename STREQUAL "testmanager.c" OR filename MATCHES "^src.*")
        list(APPEND NON_TEST_SOURCES ${sourcefile})
    else()
        list(APPEND TEST_SOURCES ${sourcefile})
    endif()
endforeach()

# Create test executables and link them with the game library
foreach(testsourcefile ${TEST_SOURCES})
    get_filename_component(testname ${testsourcefile} NAME_WE)
    set(testname "${testname}.test")
    message(STATUS "BUILDING TEST : ${testname}")

    add_executable(${testname} ${testsourcefile} ${NON_TEST_SOURCES})
    target_link_libraries(${testname} PRIVATE game_lib)
endforeach()

