#!/bin/bash
# Run all D3 unit tests
# Usage: ./run_tests.sh [test_name]
# If no test_name is provided, all tests are run

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

TEST_NAME="$1"

# ============================================
# CORE FOUNDATION TESTS
# ============================================

if [ "$TEST_NAME" = "fix" ] || [ -z "$TEST_NAME" ]; then
    echo ""
    echo "=========================================="
    echo "Running fix tests..."
    echo "=========================================="
    "$SCRIPT_DIR/run_fix_tests.sh"
fi

if [ "$TEST_NAME" = "byteswap" ] || [ -z "$TEST_NAME" ]; then
    echo ""
    echo "=========================================="
    echo "Running byteswap tests..."
    echo "=========================================="
    "$SCRIPT_DIR/run_byteswap_tests.sh"
fi

if [ "$TEST_NAME" = "md5" ] || [ -z "$TEST_NAME" ]; then
    echo ""
    echo "=========================================="
    echo "Running MD5 tests..."
    echo "=========================================="
    "$SCRIPT_DIR/run_md5_tests.sh"
fi

# psglob tests disabled - PSGlobHasPattern function doesn't exist in codebase
# if [ "$TEST_NAME" = "psglob" ] || [ -z "$TEST_NAME" ]; then
#     echo ""
#     echo "=========================================="
#     echo "Running psglob tests..."
#     echo "=========================================="
#     "$SCRIPT_DIR/run_psglob_tests.sh"
# fi

if [ "$TEST_NAME" = "pstring" ] || [ -z "$TEST_NAME" ]; then
    echo ""
    echo "=========================================="
    echo "Running pstring tests..."
    echo "=========================================="
    "$SCRIPT_DIR/run_pstring_tests.sh"
fi

if [ "$TEST_NAME" = "psrand" ] || [ -z "$TEST_NAME" ]; then
    echo ""
    echo "=========================================="
    echo "Running psrand tests..."
    echo "=========================================="
    "$SCRIPT_DIR/run_psrand_tests.sh"
fi

if [ "$TEST_NAME" = "psclass" ] || [ -z "$TEST_NAME" ]; then
    echo ""
    echo "=========================================="
    echo "Running psclass tests..."
    echo "=========================================="
    "$SCRIPT_DIR/run_psclass_tests.sh"
fi

if [ "$TEST_NAME" = "macros" ] || [ -z "$TEST_NAME" ]; then
    echo ""
    echo "=========================================="
    echo "Running macros tests..."
    echo "=========================================="
    "$SCRIPT_DIR/run_macros_tests.sh"
fi

# ============================================
# MATH & PHYSICS TESTS
# ============================================

if [ "$TEST_NAME" = "vecmat" ] || [ -z "$TEST_NAME" ]; then
    echo ""
    echo "=========================================="
    echo "Running vecmat tests..."
    echo "=========================================="
    "$SCRIPT_DIR/run_vecmat_tests.sh"
fi

if [ "$TEST_NAME" = "vecmat_external" ] || [ -z "$TEST_NAME" ]; then
    echo ""
    echo "=========================================="
    echo "Running standalone vecmat_external tests..."
    echo "=========================================="
    "$SCRIPT_DIR/run_vecmat_external_tests.sh"
fi

if [ "$TEST_NAME" = "physics" ] || [ -z "$TEST_NAME" ]; then
    echo ""
    echo "=========================================="
    echo "Running physics tests..."
    echo "=========================================="
    "$SCRIPT_DIR/run_physics_tests.sh"
fi

# ============================================
# DESCENT3/ SUB-MODULE TESTS - PART 4
# ============================================

if [ "$TEST_NAME" = "args" ] || [ -z "$TEST_NAME" ]; then
    echo ""
    echo "=========================================="
    echo "Running args tests..."
    echo "=========================================="
    "$SCRIPT_DIR/run_args_tests.sh"
fi

if [ "$TEST_NAME" = "difficulty" ] || [ -z "$TEST_NAME" ]; then
    echo ""
    echo "=========================================="
    echo "Running difficulty tests..."
    echo "=========================================="
    "$SCRIPT_DIR/run_difficulty_tests.sh"
fi

if [ "$TEST_NAME" = "weapon_external" ] || [ -z "$TEST_NAME" ]; then
    echo ""
    echo "=========================================="
    echo "Running weapon_external tests..."
    echo "=========================================="
    "$SCRIPT_DIR/run_weapon_external_tests.sh"
fi

if [ "$TEST_NAME" = "damage_external" ] || [ -z "$TEST_NAME" ]; then
    echo ""
    echo "=========================================="
    echo "Running damage_external tests..."
    echo "=========================================="
    "$SCRIPT_DIR/run_damage_external_tests.sh"
fi

if [ "$TEST_NAME" = "levelgoal_external" ] || [ -z "$TEST_NAME" ]; then
    echo ""
    echo "=========================================="
    echo "Running levelgoal_external tests..."
    echo "=========================================="
    "$SCRIPT_DIR/run_levelgoal_external_tests.sh"
fi

if [ "$TEST_NAME" = "room_external" ] || [ -z "$TEST_NAME" ]; then
    echo ""
    echo "=========================================="
    echo "Running room_external tests..."
    echo "=========================================="
    "$SCRIPT_DIR/run_room_external_tests.sh"
fi

if [ "$TEST_NAME" = "object_external" ] || [ -z "$TEST_NAME" ]; then
    echo ""
    echo "=========================================="
    echo "Running object_external tests..."
    echo "=========================================="
    "$SCRIPT_DIR/run_object_external_tests.sh"
fi

# ============================================
# PLATFORM I/O TESTS
# ============================================

if [ "$TEST_NAME" = "cfile" ] || [ -z "$TEST_NAME" ]; then
    echo ""
    echo "=========================================="
    echo "Running cfile tests..."
    echo "=========================================="
    "$SCRIPT_DIR/run_cfile_tests.sh"
fi

if [ "$TEST_NAME" = "bitmap" ] || [ -z "$TEST_NAME" ]; then
    echo ""
    echo "=========================================="
    echo "Running bitmap allocation tests..."
    echo "=========================================="
    "$SCRIPT_DIR/run_bitmap_tests.sh"
fi

if [ "$TEST_NAME" = "manage" ] || [ -z "$TEST_NAME" ]; then
    echo ""
    echo "=========================================="
    echo "Running manage tracklock tests..."
    echo "=========================================="
    "$SCRIPT_DIR/run_manage_tests.sh"
fi

# ============================================
# RENDERING TESTS (Mesa llvmpipe headless)
# ============================================

if [ "$TEST_NAME" = "text_render" ] || [ -z "$TEST_NAME" ]; then
    echo ""
    echo "=========================================="
    echo "Running text render tests (Mesa llvmpipe)..."
    echo "=========================================="
    "$SCRIPT_DIR/run_text_render_tests.sh"
fi

# ============================================
# USAGE INFO
# ============================================

if [ "$TEST_NAME" = "help" ] || [ "$TEST_NAME" = "--help" ] || [ "$TEST_NAME" = "-h" ]; then
    echo "Usage: ./run_tests.sh [test_name]"
    echo ""
    echo "Available test names:"
    echo "  Core Foundation:"
    echo "    fix, byteswap, md5, psglob, pstring, psrand, psclass, macros"
    echo ""
    echo "  Math & Physics:"
    echo "    vecmat, vecmat_external, physics"
    echo ""
    echo "  Descent3/ Sub-modules (Part 4):"
    echo "    args, difficulty, weapon_external, damage_external,"
    echo "    levelgoal_external, room_external, object_external"
    echo ""
    echo "  Platform I/O:"
    echo "    cfile, bitmap, manage"
    echo ""
    echo "  Rendering (Mesa llvmpipe headless):"
    echo "    text_render"
    echo ""
    echo "  Run all tests by providing no argument"
    exit 0
fi

echo ""
