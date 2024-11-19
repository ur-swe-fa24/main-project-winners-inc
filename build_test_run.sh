#!/bin/bash

# Exit on any error
set -e

# Print commands before executing them
set -x

# Get the directory of the script
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# Delete build directory if it exists
rm -rf "$SCRIPT_DIR/build"

# Create build directory
mkdir -p "$SCRIPT_DIR/build"

# Copy resources directory to build directory
cp -r "$SCRIPT_DIR/resources" "$SCRIPT_DIR/build/"

# Run CMake
cmake -B "$SCRIPT_DIR/build" -S "$SCRIPT_DIR"

# Build all targets
cmake --build "$SCRIPT_DIR/build" -j8

# Run tests
cd "$SCRIPT_DIR/build" && ctest --output-on-failure

# Run the wxWidgets app
"$SCRIPT_DIR/build/app/wx_robot_test"
