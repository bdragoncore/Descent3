#!/bin/bash
# Run PCX loader tests (requires cmake build)

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$SCRIPT_DIR/.."

BUILD_DIR="../build_test"

if [ ! -d "$BUILD_DIR" ]; then
    echo "CMake build directory not found. Running cmake..."
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"
    cmake -DBUILD_TESTING=ON -DENABLE_COVERAGE=ON ..
    cd "$SCRIPT_DIR/.."
fi

if [ ! -f "$BUILD_DIR/d3src/tests/coverage/pcx_loader_tests" ]; then
    echo "Building PCX loader tests..."
    cd "$BUILD_DIR"
    make pcx_loader_tests
    cd "$SCRIPT_DIR/.."
fi

"$BUILD_DIR/d3src/tests/coverage/pcx_loader_tests" "$@"
