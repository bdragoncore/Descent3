#!/bin/bash
# Standalone physics tests

BUILD_DIR="$(cd "$(dirname "$0")/../build/tests" && pwd)"

mkdir -p "$BUILD_DIR"

echo "Building standalone physics tests..."

clang++ -g --coverage -std=c++17 \
    /run/host/home/bperris/base/dev/flare.rs/d3src/tests/physics_tests_standalone.cpp \
    -o "$BUILD_DIR/physics_tests" \
    -lgtest -lgtest_main -lpthread \
    2>&1

if [ $? -eq 0 ]; then
    echo "Running tests..."
    "$BUILD_DIR/physics_tests"
else
    echo "Build failed!"
    exit 1
fi
