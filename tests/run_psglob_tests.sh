#!/bin/bash
# Run psglob tests

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$SCRIPT_DIR/.."

echo "Building standalone psglob tests..."

g++ -std=c++17 -fpermissive -I. -Ilib -I/usr/include \
    tests/psglob_tests_standalone.cpp \
    misc/psglob.cpp \
    -lgtest -lgtest_main -pthread \
    -o tests/psglob_tests

echo "Running tests..."
./tests/psglob_tests "$@"
