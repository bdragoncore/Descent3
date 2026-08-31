#!/bin/bash
# Standalone fix tests

BUILD_DIR="$(cd "$(dirname "$0")/../build/tests" && pwd)"

mkdir -p "$BUILD_DIR"

echo "Building standalone fix tests..."

clang++ -g --coverage -std=c++17 \
    /run/host/home/bperris/base/dev/flare.rs/d3src/tests/fix_tests_standalone.cpp \
    -o "$BUILD_DIR/fix_tests" \
    -lgtest -lgtest_main -lpthread \
    2>&1

if [ $? -eq 0 ]; then
    echo "Running tests..."
    "$BUILD_DIR/fix_tests"
else
    echo "Build failed!"
    exit 1
fi
