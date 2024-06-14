#!/bin/bash

# Define color constants
RED='\x1b[91m'
GREEN='\x1b[92m'
YELLOW='\x1b[93m'
BLUE='\x1b[94m'
MAGENTA='\x1b[95m'
CYAN='\x1b[96m'
WHITE='\x1b[97m'
NC='\x1b[0m'  # No Color

PREFIX="[❯]"

# Function to print colored text
color_print() {
  local color="$1"
  local message="$2"
  echo -e "${PREFIX}  ${color}${message}${NC}"
}

cmake_options="-DLOG_WARN=ON
               -DLOG_INFO=ON
               -DLOG_TEST=OFF
               -DLOG_TRACE=ON
               -DLOG_DEBUG=ON"

run_cmake() {
    pushd build || { echo "Failed to enter build directory"; exit 1; }
    cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON $cmake_options .. || { echo "CMake configuration failed"; popd; exit 1; }
    popd
}

run_make() {
    pushd build || { echo "Failed to enter build directory"; exit 1; }
    make || { echo "Compilation with make failed"; popd; exit 1; }
    popd
}

run_tests() {
    pushd build || { echo "Failed to enter build directory"; exit 1; }
    for test_executable in ../tests/*.test; do
        color_print $BLUE "RUNNING TEST FILE ${test_executable}" && ${test_executable} || { color_print $RED "Execution of ${test_executable} failed"; popd; exit 1; }
    done
    popd
}

while [[ $# -gt 0 ]]; do
    case $1 in
        -c)
            run_cmake
            ;;
        -m)
            run_make
            ;;
        -r)
            run_cmake
            run_make
            ;;
        -t)
            cmake_options="$cmake_options -DLOG_TEST=ON"
            run_make
            run_tests
            ;;
        -nw)
            cmake_options="$cmake_options -DLOG_WARN=OFF"
            ;;
        -ni)
            cmake_options="$cmake_options -DLOG_INFO=OFF"
            ;;
        -nt)
            cmake_options="$cmake_options -DLOG_TRACE=OFF"
            ;;
        -nd)
            cmake_options="$cmake_options -DLOG_DEBUG=OFF"
            ;;
        *)
            echo "Invalid option: $1" >&2
            exit 1
            ;;
    esac
    shift
done

