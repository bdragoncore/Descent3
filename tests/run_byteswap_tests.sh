#!/bin/bash
# Run byteswap tests

BUILD_DIR="$(cd "$(dirname "$0")/../build/tests" && pwd)"
mkdir -p "$BUILD_DIR"

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$SCRIPT_DIR/.."

g++ -std=c++17 -I. -I/usr/include \
    tests/byteswap_tests_standalone.cpp \
    -lgtest -lgtest_main -pthread \
    -o "$BUILD_DIR/byteswap_tests"

"$BUILD_DIR/byteswap_tests" "$@"
