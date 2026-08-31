#!/bin/bash
# Run psrand tests

BUILD_DIR="$(cd "$(dirname "$0")/../build/tests" && pwd)"
mkdir -p "$BUILD_DIR"

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$SCRIPT_DIR/.."

echo "Building standalone psrand tests..."

g++ -std=c++17 -I. -I/usr/include \
    tests/psrand_tests_standalone.cpp \
    -lgtest -lgtest_main -pthread \
    -o "$BUILD_DIR/psrand_tests"

echo "Running tests..."
"$BUILD_DIR/psrand_tests" "$@"
