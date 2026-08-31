#!/bin/bash
# Run args coverage tests

BUILD_DIR="$(cd "$(dirname "$0")/../build/tests" && pwd)"
mkdir -p "$BUILD_DIR"

cd "$(dirname "$0")"

g++ -std=c++17 -O2 -g -pthread \
    -I../lib \
    -I../Descent3 \
    -I/usr/include \
    args_tests_standalone.cpp \
    -lgtest -lgtest_main -lpthread \
    -o "$BUILD_DIR/args_tests"

if [ $? -eq 0 ]; then
    echo "Running args tests..."
    "$BUILD_DIR/args_tests"
else
    echo "Compilation failed!"
    exit 1
fi
