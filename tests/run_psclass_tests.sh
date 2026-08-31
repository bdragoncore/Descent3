#!/bin/bash
# Run psclass tests

BUILD_DIR="$(cd "$(dirname "$0")/../build/tests" && pwd)"
mkdir -p "$BUILD_DIR"

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$SCRIPT_DIR/.."

echo "Building standalone psclass tests..."

g++ -std=c++17 -I. -I/usr/include \
    tests/psclass_tests_standalone.cpp \
    -lgtest -lgtest_main -pthread \
    -o "$BUILD_DIR/psclass_tests"

echo "Running tests..."
"$BUILD_DIR/psclass_tests" "$@"
