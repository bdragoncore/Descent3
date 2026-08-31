#!/bin/bash
# Standalone vecmat tests - truly standalone version
# Uses GoogleTest with pure C++ vector math (no D3 dependencies)

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$(cd "$(dirname "$0")/../build/tests" && pwd)"

mkdir -p "$BUILD_DIR"

echo "Building standalone vecmat tests..."

clang++ -g --coverage -std=c++17 \
    -I/usr/include \
    "$SCRIPT_DIR/vecmat_tests_standalone.cpp" \
    -o "$BUILD_DIR/vecmat_tests" \
    -lgtest -lgtest_main -lpthread \
    2>&1

if [ $? -eq 0 ]; then
    echo "Running tests..."
    "$BUILD_DIR/vecmat_tests"
else
    echo "Build failed!"
    exit 1
fi
