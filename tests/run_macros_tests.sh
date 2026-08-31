#!/bin/bash
# Run macros tests

BUILD_DIR="$(cd "$(dirname "$0")/../build/tests" && pwd)"
mkdir -p "$BUILD_DIR"

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$SCRIPT_DIR/.."

echo "Building standalone macros tests..."

g++ -std=c++17 -I. -I/usr/include \
    tests/macros_tests_standalone.cpp \
    -lgtest -lgtest_main -pthread \
    -o "$BUILD_DIR/macros_tests"

echo "Running tests..."
"$BUILD_DIR/macros_tests" "$@"
