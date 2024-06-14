cmake_minimum_required(VERSION 3.10)
project(TETRIS)

# For C code
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")

# Define options
option(LOG_WARN  "Enable warn  logs"  ON)
option(LOG_INFO  "Enable info  logs"  ON)
option(LOG_TEST  "Enable debug logs"  ON)
option(LOG_TRACE "Enable trace logs"  ON)
option(LOG_DEBUG "Enable debug logs"  ON)

# Print the options values for debugging
message(STATUS "LOG_WARN_ENABLED:   ${LOG_WARN}")
message(STATUS "LOG_INFO_ENABLED:   ${LOG_INFO}")
message(STATUS "LOG_TEST_ENABLED:   ${LOG_TEST}")
message(STATUS "LOG_TRACE_ENABLED:  ${LOG_TRACE}")
message(STATUS "LOG_DEBUG_ENABLED:  ${LOG_DEBUG}")

# Use the option values directly in add_compile_definitions
add_compile_definitions(
  LOG_WARN_ENABLED=$<IF:$<BOOL:${LOG_WARN}>,1,0>
  LOG_TEST_ENABLED=$<IF:$<BOOL:${LOG_TEST}>,1,0>
  LOG_INFO_ENABLED=$<IF:$<BOOL:${LOG_INFO}>,1,0>
  LOG_TRACE_ENABLED=$<IF:$<BOOL:${LOG_TRACE}>,1,0>
  LOG_DEBUG_ENABLED=$<IF:$<BOOL:${LOG_DEBUG}>,1,0>
)

# Include subdirectories
message(STATUS "GAME")
add_subdirectory(game)

message(STATUS "TESTS")
add_subdirectory(tests)
