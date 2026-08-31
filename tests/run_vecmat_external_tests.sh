#!/bin/bash
# Run vecmat_external coverage tests

BUILD_DIR="$(cd "$(dirname "$0")/../build/tests" && pwd)"
mkdir -p "$BUILD_DIR"

cd "$(dirname "$0")"

g++ -std=c++17 -O2 -g -pthread \
    -I../lib \
    -I../Descent3 \
    -I/usr/include \
    vecmat_external_tests_standalone.cpp \
    -lgtest -lgtest_main -lpthread \
    -o "$BUILD_DIR/vecmat_external_tests"

if [ $? -eq 0 ]; then
    echo "Running vecmat_external tests..."
    "$BUILD_DIR/vecmat_external_tests"
else
    echo "Compilation failed!"
    exit 1
fi
