#!/bin/bash
# Run weapon_external coverage tests

BUILD_DIR="$(cd "$(dirname "$0")/../build/tests" && pwd)"
mkdir -p "$BUILD_DIR"

cd "$(dirname "$0")"

g++ -std=c++17 -O2 -g -pthread \
    -I../lib \
    -I../Descent3 \
    -I/usr/include \
    weapon_external_tests_standalone.cpp \
    -lgtest -lgtest_main -lpthread \
    -o "$BUILD_DIR/weapon_external_tests"

if [ $? -eq 0 ]; then
    echo "Running weapon_external tests..."
    "$BUILD_DIR/weapon_external_tests"
else
    echo "Compilation failed!"
    exit 1
fi
