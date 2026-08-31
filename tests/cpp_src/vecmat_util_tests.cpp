/**
 * @file vecmat_util_tests.cpp
 * @brief vecmat utility tests — cover vm_* functions not previously tested.
 *
 * @details
 * Links against the real vecmat library (vecmat/vector.cpp) plus its
 * dependencies (fix, misc, ddebug). Uses row-vector / row-major conventions:
 * v' = v * M.
 *
 * This harness validates the behavior of `vecmat/vector.cpp, lib/vecmat_external.h, fix/fix.h`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `vecmat/vector.cpp, lib/vecmat_external.h, fix/fix.h`
 * @par Harness
 * `vecmat_util_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see vecmat/vector.cpp
 * @see lib/vecmat_external.h
 * @see fix/fix.h
 */

#include <gtest/gtest.h>
#include "vecmat.h"
#include "fix.h"
#include <cmath>
#include <limits>

static constexpr float TOL = 1e-4f;

// The fix trig tables (sincos/asin/acos) are only initialized by the game's
// startup code (Descent3/init.cpp); initialize them here for library use.
static struct MathTablesInit {
    MathTablesInit() { InitMathTables(); }
} math_tables_init;

// ============================================================================
// vm_ClearMatrix
// ============================================================================

/**
 * @test VecmatUtil.vm_ClearMatrix_ClearsAllElements
 * @brief Verifies vm Clear Matrix Clears All Elements.
 *
 * @details
 * Exercises the VecmatUtil code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see vecmat/vector.cpp, lib/vecmat_external.h, fix/fix.h
 * @ingroup descent3_tests
 */
TEST(VecmatUtil, vm_ClearMatrix_ClearsAllElements) {
    matrix m;
    m.rvec = {1.0f, 2.0f, 3.0f};
    m.uvec = {4.0f, 5.0f, 6.0f};
    m.fvec = {7.0f, 8.0f, 9.0f};

    vm_ClearMatrix(&m);

    EXPECT_FLOAT_EQ(m.rvec.x(), 0.0f);
    EXPECT_FLOAT_EQ(m.rvec.y(), 0.0f);
    EXPECT_FLOAT_EQ(m.rvec.z(), 0.0f);
    EXPECT_FLOAT_EQ(m.uvec.x(), 0.0f);
    EXPECT_FLOAT_EQ(m.uvec.y(), 0.0f);
    EXPECT_FLOAT_EQ(m.uvec.z(), 0.0f);
    EXPECT_FLOAT_EQ(m.fvec.x(), 0.0f);
    EXPECT_FLOAT_EQ(m.fvec.y(), 0.0f);
    EXPECT_FLOAT_EQ(m.fvec.z(), 0.0f);
}

// ============================================================================
// vm_AverageVector
// ============================================================================

/**
 * @test VecmatUtil.vm_AverageVector_DividesByCount
 * @brief Verifies vm Average Vector Divides By Count.
 *
 * @details
 * Exercises the VecmatUtil code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see vecmat/vector.cpp, lib/vecmat_external.h, fix/fix.h
 * @ingroup descent3_tests
 */
TEST(VecmatUtil, vm_AverageVector_DividesByCount) {
    vector v = {2.0f, 4.0f, 6.0f};
    vm_AverageVector(&v, 2);
    EXPECT_FLOAT_EQ(v.x(), 1.0f);
    EXPECT_FLOAT_EQ(v.y(), 2.0f);
    EXPECT_FLOAT_EQ(v.z(), 3.0f);
}

/**
 * @test VecmatUtil.vm_AverageVector_ZeroVectorStaysZero
 * @brief Verifies vm Average Vector Zero Vector Stays Zero.
 *
 * @details
 * Exercises the VecmatUtil code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see vecmat/vector.cpp, lib/vecmat_external.h, fix/fix.h
 * @ingroup descent3_tests
 */
TEST(VecmatUtil, vm_AverageVector_ZeroVectorStaysZero) {
    vector v = {0.0f, 0.0f, 0.0f};
    vm_AverageVector(&v, 5);
    EXPECT_FLOAT_EQ(v.x(), 0.0f);
    EXPECT_FLOAT_EQ(v.y(), 0.0f);
    EXPECT_FLOAT_EQ(v.z(), 0.0f);
}

// ============================================================================
// vm_ScaleAddVector
// ============================================================================

/**
 * @test VecmatUtil.vm_ScaleAddVector_CombinesPointAndScaledVector
 * @brief Verifies vm Scale Add Vector Combines Point And Scaled Vector.
 *
 * @details
 * Exercises the VecmatUtil code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see vecmat/vector.cpp, lib/vecmat_external.h, fix/fix.h
 * @ingroup descent3_tests
 */
TEST(VecmatUtil, vm_ScaleAddVector_CombinesPointAndScaledVector) {
    vector p = {1.0f, 2.0f, 3.0f};
    vector v = {2.0f, 3.0f, 4.0f};
    vector d;
    vm_ScaleAddVector(&d, &p, &v, 2.0f);
    EXPECT_FLOAT_EQ(d.x(), 5.0f);
    EXPECT_FLOAT_EQ(d.y(), 8.0f);
    EXPECT_FLOAT_EQ(d.z(), 11.0f);
}

/**
 * @test VecmatUtil.vm_ScaleAddVector_ZeroScaleEqualsPoint
 * @brief Verifies vm Scale Add Vector Zero Scale Equals Point.
 *
 * @details
 * Exercises the VecmatUtil code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see vecmat/vector.cpp, lib/vecmat_external.h, fix/fix.h
 * @ingroup descent3_tests
 */
TEST(VecmatUtil, vm_ScaleAddVector_ZeroScaleEqualsPoint) {
    vector p = {1.0f, 2.0f, 3.0f};
    vector v = {2.0f, 3.0f, 4.0f};
    vector d;
    vm_ScaleAddVector(&d, &p, &v, 0.0f);
    EXPECT_FLOAT_EQ(d.x(), p.x());
    EXPECT_FLOAT_EQ(d.y(), p.y());
    EXPECT_FLOAT_EQ(d.z(), p.z());
}

// ============================================================================
// vm_DivVector
// ============================================================================

/**
 * @test VecmatUtil.vm_DivVector_DividesComponents
 * @brief Verifies vm Div Vector Divides Components.
 *
 * @details
 * Exercises the VecmatUtil code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see vecmat/vector.cpp, lib/vecmat_external.h, fix/fix.h
 * @ingroup descent3_tests
 */
TEST(VecmatUtil, vm_DivVector_DividesComponents) {
    vector src = {2.0f, 4.0f, 6.0f};
    vector d;
    vm_DivVector(&d, &src, 2.0f);
    EXPECT_FLOAT_EQ(d.x(), 1.0f);
    EXPECT_FLOAT_EQ(d.y(), 2.0f);
    EXPECT_FLOAT_EQ(d.z(), 3.0f);
}

/**
 * @test VecmatUtil.vm_DivVector_DoesNotAliasInput
 * @brief Verifies vm Div Vector Does Not Alias Input.
 *
 * @details
 * Exercises the VecmatUtil code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see vecmat/vector.cpp, lib/vecmat_external.h, fix/fix.h
 * @ingroup descent3_tests
 */
TEST(VecmatUtil, vm_DivVector_DoesNotAliasInput) {
    vector src = {1.0f, 2.0f, 3.0f};
    vector d;
    vm_DivVector(&d, &src, 10.0f);
    EXPECT_FLOAT_EQ(src.x(), 1.0f);
    EXPECT_FLOAT_EQ(src.y(), 2.0f);
    EXPECT_FLOAT_EQ(src.z(), 3.0f);
}

// ============================================================================
// vm_Dot3Product / vm_Dot3Vector
// ============================================================================

/**
 * @test VecmatUtil.vm_Dot3Product_MatchesManualDot
 * @brief Verifies vm Dot3Product Matches Manual Dot.
 *
 * @details
 * Exercises the VecmatUtil code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see vecmat/vector.cpp, lib/vecmat_external.h, fix/fix.h
 * @ingroup descent3_tests
 */
TEST(VecmatUtil, vm_Dot3Product_MatchesManualDot) {
    vector a = {1.0f, 2.0f, 3.0f};
    vector b = {4.0f, -5.0f, 6.0f};
    scalar expected = 1.0f * 4.0f + 2.0f * -5.0f + 3.0f * 6.0f;
    EXPECT_FLOAT_EQ(vm_Dot3Product(a, b), expected);
}

/**
 * @test VecmatUtil.vm_Dot3Product_OrthogonalVectors
 * @brief Verifies vm Dot3Product Orthogonal Vectors.
 *
 * @details
 * Exercises the VecmatUtil code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see vecmat/vector.cpp, lib/vecmat_external.h, fix/fix.h
 * @ingroup descent3_tests
 */
TEST(VecmatUtil, vm_Dot3Product_OrthogonalVectors) {
    vector a = {1.0f, 0.0f, 0.0f};
    vector b = {0.0f, 1.0f, 0.0f};
    EXPECT_FLOAT_EQ(vm_Dot3Product(a, b), 0.0f);
}

/**
 * @test VecmatUtil.vm_Dot3Vector_MatchesDotProduct
 * @brief Verifies vm Dot3Vector Matches Dot Product.
 *
 * @details
 * Exercises the VecmatUtil code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see vecmat/vector.cpp, lib/vecmat_external.h, fix/fix.h
 * @ingroup descent3_tests
 */
TEST(VecmatUtil, vm_Dot3Vector_MatchesDotProduct) {
    vector v = {1.0f, 2.0f, 3.0f};
    EXPECT_FLOAT_EQ(vm_Dot3Vector(4.0f, -5.0f, 6.0f, &v), 12.0f);
}

// ============================================================================
// vm_Cross3Product
// ============================================================================

/**
 * @test VecmatUtil.vm_Cross3Product_MatchesManualCross
 * @brief Verifies vm Cross3Product Matches Manual Cross.
 *
 * @details
 * Exercises the VecmatUtil code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see vecmat/vector.cpp, lib/vecmat_external.h, fix/fix.h
 * @ingroup descent3_tests
 */
TEST(VecmatUtil, vm_Cross3Product_MatchesManualCross) {
    vector a = {1.0f, 0.0f, 0.0f};
    vector b = {0.0f, 1.0f, 0.0f};
    vector c = vm_Cross3Product(a, b);
    EXPECT_FLOAT_EQ(c.x(), 0.0f);
    EXPECT_FLOAT_EQ(c.y(), 0.0f);
    EXPECT_FLOAT_EQ(c.z(), 1.0f);
}

/**
 * @test VecmatUtil.vm_Cross3Product_Anticommutative
 * @brief Verifies vm Cross3Product Anticommutative.
 *
 * @details
 * Exercises the VecmatUtil code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see vecmat/vector.cpp, lib/vecmat_external.h, fix/fix.h
 * @ingroup descent3_tests
 */
TEST(VecmatUtil, vm_Cross3Product_Anticommutative) {
    vector a = {1.0f, 2.0f, 3.0f};
    vector b = {4.0f, 5.0f, 6.0f};
    vector ab = vm_Cross3Product(a, b);
    vector ba = vm_Cross3Product(b, a);
    EXPECT_NEAR(ab.x(), -ba.x(), TOL);
    EXPECT_NEAR(ab.y(), -ba.y(), TOL);
    EXPECT_NEAR(ab.z(), -ba.z(), TOL);
}

/**
 * @test VecmatUtil.vm_Cross3Product_ParallelVectorsZero
 * @brief Verifies vm Cross3Product Parallel Vectors Zero.
 *
 * @details
 * Exercises the VecmatUtil code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see vecmat/vector.cpp, lib/vecmat_external.h, fix/fix.h
 * @ingroup descent3_tests
 */
TEST(VecmatUtil, vm_Cross3Product_ParallelVectorsZero) {
    vector a = {2.0f, 4.0f, 6.0f};
    vector b = {1.0f, 2.0f, 3.0f};
    vector c = vm_Cross3Product(a, b);
    EXPECT_FLOAT_EQ(c.x(), 0.0f);
    EXPECT_FLOAT_EQ(c.y(), 0.0f);
    EXPECT_FLOAT_EQ(c.z(), 0.0f);
}

// ============================================================================
// vm_VectorDistanceQuick
// ============================================================================

/**
 * @test VecmatUtil.vm_VectorDistanceQuick_MatchesExactDistance
 * @brief Verifies vm Vector Distance Quick Matches Exact Distance.
 *
 * @details
 * Exercises the VecmatUtil code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see vecmat/vector.cpp, lib/vecmat_external.h, fix/fix.h
 * @ingroup descent3_tests
 */
TEST(VecmatUtil, vm_VectorDistanceQuick_MatchesExactDistance) {
    vector a = {0.0f, 0.0f, 0.0f};
    vector b = {3.0f, 4.0f, 0.0f};
    EXPECT_NEAR(vm_VectorDistanceQuick(&a, &b), 5.0f, TOL);
}

/**
 * @test VecmatUtil.vm_VectorDistanceQuick_ZeroForSamePoint
 * @brief Verifies vm Vector Distance Quick Zero For Same Point.
 *
 * @details
 * Exercises the VecmatUtil code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see vecmat/vector.cpp, lib/vecmat_external.h, fix/fix.h
 * @ingroup descent3_tests
 */
TEST(VecmatUtil, vm_VectorDistanceQuick_ZeroForSamePoint) {
    vector a = {1.0f, 2.0f, 3.0f};
    EXPECT_FLOAT_EQ(vm_VectorDistanceQuick(&a, &a), 0.0f);
}

// ============================================================================
// vm_GetMagnitudeFast
// ============================================================================

/**
 * @test VecmatUtil.vm_GetMagnitudeFast_CloseToExact
 * @brief Verifies vm Get Magnitude Fast Close To Exact.
 *
 * @details
 * Exercises the VecmatUtil code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see vecmat/vector.cpp, lib/vecmat_external.h, fix/fix.h
 * @ingroup descent3_tests
 */
TEST(VecmatUtil, vm_GetMagnitudeFast_CloseToExact) {
    // 3-4-5 triangle
    vector v = {3.0f, 4.0f, 0.0f};
    EXPECT_NEAR(vm_GetMagnitudeFast(&v), 5.0f, 5.0f * 0.1f);
    // Unit cube diagonal
    vector w = {1.0f, 1.0f, 1.0f};
    EXPECT_NEAR(vm_GetMagnitudeFast(&w), std::sqrt(3.0f), std::sqrt(3.0f) * 0.15f);
}

/**
 * @test VecmatUtil.vm_GetMagnitudeFast_AxisAlignedIsExact
 * @brief Verifies vm Get Magnitude Fast Axis Aligned Is Exact.
 *
 * @details
 * Exercises the VecmatUtil code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see vecmat/vector.cpp, lib/vecmat_external.h, fix/fix.h
 * @ingroup descent3_tests
 */
TEST(VecmatUtil, vm_GetMagnitudeFast_AxisAlignedIsExact) {
    vector v = {0.0f, 0.0f, 7.0f};
    EXPECT_NEAR(vm_GetMagnitudeFast(&v), 7.0f, TOL);
}

/**
 * @test VecmatUtil.vm_GetMagnitudeFast_ZeroVector
 * @brief Verifies vm Get Magnitude Fast Zero Vector.
 *
 * @details
 * Exercises the VecmatUtil code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see vecmat/vector.cpp, lib/vecmat_external.h, fix/fix.h
 * @ingroup descent3_tests
 */
TEST(VecmatUtil, vm_GetMagnitudeFast_ZeroVector) {
    vector v = {0.0f, 0.0f, 0.0f};
    EXPECT_FLOAT_EQ(vm_GetMagnitudeFast(&v), 0.0f);
}

// ============================================================================
// vm_NormalizeVectorFast
// ============================================================================

/**
 * @test VecmatUtil.vm_NormalizeVectorFast_ReturnsApproximatelyUnitVector
 * @brief Verifies vm Normalize Vector Fast Returns Approximately Unit Vector.
 *
 * @details
 * Exercises the VecmatUtil code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see vecmat/vector.cpp, lib/vecmat_external.h, fix/fix.h
 * @ingroup descent3_tests
 */
TEST(VecmatUtil, vm_NormalizeVectorFast_ReturnsApproximatelyUnitVector) {
    vector v = {3.0f, 4.0f, 0.0f};
    scalar mag = vm_NormalizeVectorFast(&v);
    float len = std::sqrt(v.x() * v.x() + v.y() * v.y() + v.z() * v.z());
    // Fast normalization divides by the approximated magnitude (5.125 vs 5),
    // so the result is close to but not exactly unit length.
    EXPECT_NEAR(len, 1.0f, 0.05f);
    EXPECT_NEAR(mag, 5.0f, 5.0f * 0.1f);
}

/**
 * @test VecmatUtil.vm_NormalizeVectorFast_PreservesDirection
 * @brief Verifies vm Normalize Vector Fast Preserves Direction.
 *
 * @details
 * Exercises the VecmatUtil code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see vecmat/vector.cpp, lib/vecmat_external.h, fix/fix.h
 * @ingroup descent3_tests
 */
TEST(VecmatUtil, vm_NormalizeVectorFast_PreservesDirection) {
    vector v = {1.0f, 2.0f, 2.0f};
    vm_NormalizeVectorFast(&v);
    EXPECT_NEAR(v.x() / v.z(), 0.5f, TOL);
    EXPECT_NEAR(v.y() / v.z(), 1.0f, TOL);
}

/**
 * @test VecmatUtil.vm_NormalizeVectorFast_ZeroVector
 * @brief Verifies vm Normalize Vector Fast Zero Vector.
 *
 * @details
 * Exercises the VecmatUtil code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see vecmat/vector.cpp, lib/vecmat_external.h, fix/fix.h
 * @ingroup descent3_tests
 */
TEST(VecmatUtil, vm_NormalizeVectorFast_ZeroVector) {
    vector v = {0.0f, 0.0f, 0.0f};
    scalar mag = vm_NormalizeVectorFast(&v);
    EXPECT_FLOAT_EQ(mag, 0.0f);
    EXPECT_FLOAT_EQ(v.x(), 0.0f);
    EXPECT_FLOAT_EQ(v.y(), 0.0f);
    EXPECT_FLOAT_EQ(v.z(), 0.0f);
}

// ============================================================================
// vm_GetNormalizedDirFast
// ============================================================================

/**
 * @test VecmatUtil.vm_GetNormalizedDirFast_ApproximatelyUnitDirection
 * @brief Verifies vm Get Normalized Dir Fast Approximately Unit Direction.
 *
 * @details
 * Exercises the VecmatUtil code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see vecmat/vector.cpp, lib/vecmat_external.h, fix/fix.h
 * @ingroup descent3_tests
 */
TEST(VecmatUtil, vm_GetNormalizedDirFast_ApproximatelyUnitDirection) {
    vector start = {1.0f, 1.0f, 1.0f};
    vector end = {4.0f, 5.0f, 1.0f}; // delta = (3,4,0), length 5
    vector dest;
    vm_GetNormalizedDirFast(&dest, &end, &start);
    float len = std::sqrt(dest.x() * dest.x() + dest.y() * dest.y() + dest.z() * dest.z());
    // Normalized by the fast (approximate) magnitude, so tolerances are loose.
    EXPECT_NEAR(len, 1.0f, 0.05f);
    EXPECT_NEAR(dest.x(), 0.6f, 0.05f);
    EXPECT_NEAR(dest.y(), 0.8f, 0.05f);
    EXPECT_NEAR(dest.z(), 0.0f, TOL);
}

/**
 * @test VecmatUtil.vm_GetNormalizedDirFast_ReturnsFastMagnitude
 * @brief Verifies vm Get Normalized Dir Fast Returns Fast Magnitude.
 *
 * @details
 * Exercises the VecmatUtil code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see vecmat/vector.cpp, lib/vecmat_external.h, fix/fix.h
 * @ingroup descent3_tests
 */
TEST(VecmatUtil, vm_GetNormalizedDirFast_ReturnsFastMagnitude) {
    vector start = {0.0f, 0.0f, 0.0f};
    vector end = {3.0f, 4.0f, 0.0f};
    vector dest;
    scalar mag = vm_GetNormalizedDirFast(&dest, &end, &start);
    EXPECT_NEAR(mag, 5.0f, 5.0f * 0.1f);
}

// ============================================================================
// vm_DistToPlane
// ============================================================================

/**
 * @test VecmatUtil.vm_DistToPlane_PointOnPlane
 * @brief Verifies vm Dist To Plane Point On Plane.
 *
 * @details
 * Exercises the VecmatUtil code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see vecmat/vector.cpp, lib/vecmat_external.h, fix/fix.h
 * @ingroup descent3_tests
 */
TEST(VecmatUtil, vm_DistToPlane_PointOnPlane) {
    vector norm = {0.0f, 1.0f, 0.0f};
    vector planep = {0.0f, 5.0f, 0.0f};
    vector check = {3.0f, 5.0f, -2.0f};
    EXPECT_NEAR(vm_DistToPlane(&check, &norm, &planep), 0.0f, TOL);
}

/**
 * @test VecmatUtil.vm_DistToPlane_SignedDistance
 * @brief Verifies vm Dist To Plane Signed Distance.
 *
 * @details
 * Exercises the VecmatUtil code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see vecmat/vector.cpp, lib/vecmat_external.h, fix/fix.h
 * @ingroup descent3_tests
 */
TEST(VecmatUtil, vm_DistToPlane_SignedDistance) {
    vector norm = {0.0f, 1.0f, 0.0f};
    vector planep = {0.0f, 5.0f, 0.0f};
    vector above = {0.0f, 8.0f, 0.0f};
    vector below = {0.0f, 2.0f, 0.0f};
    EXPECT_NEAR(vm_DistToPlane(&above, &norm, &planep), 3.0f, TOL);
    EXPECT_NEAR(vm_DistToPlane(&below, &norm, &planep), -3.0f, TOL);
}

/**
 * @test VecmatUtil.vm_DistToPlane_NonUnitNormal
 * @brief Verifies vm Dist To Plane Non Unit Normal.
 *
 * @details
 * Exercises the VecmatUtil code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see vecmat/vector.cpp, lib/vecmat_external.h, fix/fix.h
 * @ingroup descent3_tests
 */
TEST(VecmatUtil, vm_DistToPlane_NonUnitNormal) {
    vector norm = {0.0f, 2.0f, 0.0f};
    vector planep = {0.0f, 0.0f, 0.0f};
    vector check = {0.0f, 3.0f, 0.0f};
    // Signed distance uses the raw (possibly non-unit) normal.
    EXPECT_NEAR(vm_DistToPlane(&check, &norm, &planep), 6.0f, TOL);
}

// ============================================================================
// vm_GetCentroid / vm_GetCentroidFast
// ============================================================================

/**
 * @test VecmatUtil.vm_GetCentroid_SquareCenteredAtOrigin
 * @brief Verifies vm Get Centroid Square Centered At Origin.
 *
 * @details
 * Exercises the VecmatUtil code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see vecmat/vector.cpp, lib/vecmat_external.h, fix/fix.h
 * @ingroup descent3_tests
 */
TEST(VecmatUtil, vm_GetCentroid_SquareCenteredAtOrigin) {
    // Unit square in the z=0 plane.
    vector sq[4] = {
        {-1.0f, -1.0f, 0.0f},
        { 1.0f, -1.0f, 0.0f},
        { 1.0f,  1.0f, 0.0f},
        {-1.0f,  1.0f, 0.0f},
    };
    vector centroid;
    scalar area = vm_GetCentroid(&centroid, sq, 4);
    EXPECT_NEAR(centroid.x(), 0.0f, TOL);
    EXPECT_NEAR(centroid.y(), 0.0f, TOL);
    EXPECT_NEAR(centroid.z(), 0.0f, TOL);
    EXPECT_NEAR(area, 4.0f, TOL);
}

/**
 * @test VecmatUtil.vm_GetCentroid_OffsetSquare
 * @brief Verifies vm Get Centroid Offset Square.
 *
 * @details
 * Exercises the VecmatUtil code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see vecmat/vector.cpp, lib/vecmat_external.h, fix/fix.h
 * @ingroup descent3_tests
 */
TEST(VecmatUtil, vm_GetCentroid_OffsetSquare) {
    vector sq[4] = {
        {1.0f, 1.0f, 0.0f},
        {3.0f, 1.0f, 0.0f},
        {3.0f, 3.0f, 0.0f},
        {1.0f, 3.0f, 0.0f},
    };
    vector centroid;
    vm_GetCentroid(&centroid, sq, 4);
    EXPECT_NEAR(centroid.x(), 2.0f, TOL);
    EXPECT_NEAR(centroid.y(), 2.0f, TOL);
}

/**
 * @test VecmatUtil.vm_GetCentroidFast_MatchesCentroid
 * @brief Verifies vm Get Centroid Fast Matches Centroid.
 *
 * @details
 * Exercises the VecmatUtil code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see vecmat/vector.cpp, lib/vecmat_external.h, fix/fix.h
 * @ingroup descent3_tests
 */
TEST(VecmatUtil, vm_GetCentroidFast_MatchesCentroid) {
    vector sq[4] = {
        {-2.0f, -1.0f, 0.0f},
        { 2.0f, -1.0f, 0.0f},
        { 2.0f,  1.0f, 0.0f},
        {-2.0f,  1.0f, 0.0f},
    };
    vector centroid, fast;
    vm_GetCentroid(&centroid, sq, 4);
    vm_GetCentroidFast(&fast, sq, 4);
    EXPECT_NEAR(fast.x(), centroid.x(), 1e-3f);
    EXPECT_NEAR(fast.y(), centroid.y(), 1e-3f);
    EXPECT_NEAR(fast.z(), centroid.z(), 1e-3f);
}

// ============================================================================
// vm_ComputeBoundingSphere
// ============================================================================

/**
 * @test VecmatUtil.vm_ComputeBoundingSphere_CentersCube
 * @brief Verifies vm Compute Bounding Sphere Centers Cube.
 *
 * @details
 * Exercises the VecmatUtil code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see vecmat/vector.cpp, lib/vecmat_external.h, fix/fix.h
 * @ingroup descent3_tests
 */
TEST(VecmatUtil, vm_ComputeBoundingSphere_CentersCube) {
    // Corners of the unit cube centered at origin.
    vector cube[8] = {
        {-1, -1, -1}, { 1, -1, -1}, { 1,  1, -1}, {-1,  1, -1},
        {-1, -1,  1}, { 1, -1,  1}, { 1,  1,  1}, {-1,  1,  1},
    };
    vector center;
    scalar radius = vm_ComputeBoundingSphere(&center, cube, 8);
    // The Graphics Gems I algorithm is a heuristic, not the minimal sphere:
    // center lands near (but not exactly at) the origin and the radius is a
    // bit larger than the minimal sqrt(3).
    EXPECT_NEAR(center.x(), 0.0f, 0.5f);
    EXPECT_NEAR(center.y(), 0.0f, 0.5f);
    EXPECT_NEAR(center.z(), 0.0f, 0.5f);
    EXPECT_GT(radius, std::sqrt(3.0f));
    EXPECT_LT(radius, std::sqrt(3.0f) * 1.3f);
    // Every vertex must be inside the returned sphere.
    for (int i = 0; i < 8; i++) {
        float d = std::sqrt(
            (cube[i].x() - center.x()) * (cube[i].x() - center.x()) +
            (cube[i].y() - center.y()) * (cube[i].y() - center.y()) +
            (cube[i].z() - center.z()) * (cube[i].z() - center.z()));
        EXPECT_LE(d, radius + 1e-3f);
    }
}

/**
 * @test VecmatUtil.vm_ComputeBoundingSphere_AllPointsInside
 * @brief Verifies vm Compute Bounding Sphere All Points Inside.
 *
 * @details
 * Exercises the VecmatUtil code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see vecmat/vector.cpp, lib/vecmat_external.h, fix/fix.h
 * @ingroup descent3_tests
 */
TEST(VecmatUtil, vm_ComputeBoundingSphere_AllPointsInside) {
    vector pts[5] = {
        {10.0f, 0.0f, 0.0f},
        {0.0f, 8.0f, 0.0f},
        {0.0f, 0.0f, 6.0f},
        {-10.0f, 0.0f, 0.0f},
        {0.0f, -8.0f, 0.0f},
    };
    vector center;
    scalar radius = vm_ComputeBoundingSphere(&center, pts, 5);
    EXPECT_GT(radius, 0.0f);
    for (int i = 0; i < 5; i++) {
        float d = std::sqrt(
            (pts[i].x() - center.x()) * (pts[i].x() - center.x()) +
            (pts[i].y() - center.y()) * (pts[i].y() - center.y()) +
            (pts[i].z() - center.z()) * (pts[i].z() - center.z()));
        EXPECT_LE(d, radius + 1e-3f);
    }
}

// ============================================================================
// vm_MakeRandomVector
// ============================================================================

/**
 * @test VecmatUtil.vm_MakeRandomVector_ComponentsInRange
 * @brief Verifies vm Make Random Vector Components In Range.
 *
 * @details
 * Exercises the VecmatUtil code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see vecmat/vector.cpp, lib/vecmat_external.h, fix/fix.h
 * @ingroup descent3_tests
 */
TEST(VecmatUtil, vm_MakeRandomVector_ComponentsInRange) {
    for (int i = 0; i < 10; i++) {
        vector v;
        vm_MakeRandomVector(&v);
        // ps_rand() returns [0, D3_RAND_MAX], shifted by D3_RAND_MAX/2.
        EXPECT_GE(v.x(), -16383.0f);
        EXPECT_LE(v.x(), 16383.0f);
        EXPECT_GE(v.y(), -16383.0f);
        EXPECT_LE(v.y(), 16383.0f);
        EXPECT_GE(v.z(), -16383.0f);
        EXPECT_LE(v.z(), 16383.0f);
    }
}

/**
 * @test VecmatUtil.vm_MakeRandomVector_ProducesVariation
 * @brief Verifies vm Make Random Vector Produces Variation.
 *
 * @details
 * Exercises the VecmatUtil code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see vecmat/vector.cpp, lib/vecmat_external.h, fix/fix.h
 * @ingroup descent3_tests
 */
TEST(VecmatUtil, vm_MakeRandomVector_ProducesVariation) {
    vector a, b;
    vm_MakeRandomVector(&a);
    vm_MakeRandomVector(&b);
    bool different = (a.x() != b.x()) || (a.y() != b.y()) || (a.z() != b.z());
    // Practically guaranteed to differ; guards against a degenerate RNG.
    EXPECT_TRUE(different);
}

// ============================================================================
// vm_MakeInverseMatrix
// ============================================================================

// Historical D3 behavior: this function is a stub that returns diag(-1,-1,-1)
// regardless of the input matrix. It is NOT a true matrix inverse; the
// codebase uses vm_TransposeMatrix for inverting orthonormal matrices.
/**
 * @test VecmatUtil.vm_MakeInverseMatrix_ReturnsNegatedIdentity
 * @brief Verifies vm Make Inverse Matrix Returns Negated Identity.
 *
 * @details
 * Exercises the VecmatUtil code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see vecmat/vector.cpp, lib/vecmat_external.h, fix/fix.h
 * @ingroup descent3_tests
 */
TEST(VecmatUtil, vm_MakeInverseMatrix_ReturnsNegatedIdentity) {
    matrix m, inv;
    vm_AnglesToMatrix(&m, 5461, 8192, 10923); // ~30, 45, 60 degrees
    vm_MakeInverseMatrix(&inv);
    EXPECT_FLOAT_EQ(inv.rvec.x(), -1.0f);
    EXPECT_FLOAT_EQ(inv.uvec.y(), -1.0f);
    EXPECT_FLOAT_EQ(inv.fvec.z(), -1.0f);
    EXPECT_FLOAT_EQ(inv.rvec.y(), 0.0f);
    EXPECT_FLOAT_EQ(inv.rvec.z(), 0.0f);
    EXPECT_FLOAT_EQ(inv.uvec.x(), 0.0f);
    EXPECT_FLOAT_EQ(inv.uvec.z(), 0.0f);
    EXPECT_FLOAT_EQ(inv.fvec.x(), 0.0f);
    EXPECT_FLOAT_EQ(inv.fvec.y(), 0.0f);
}

/**
 * @test VecmatUtil.vm_MakeInverseMatrix_DoesNotDependOnInput
 * @brief Verifies vm Make Inverse Matrix Does Not Depend On Input.
 *
 * @details
 * Exercises the VecmatUtil code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see vecmat/vector.cpp, lib/vecmat_external.h, fix/fix.h
 * @ingroup descent3_tests
 */
TEST(VecmatUtil, vm_MakeInverseMatrix_DoesNotDependOnInput) {
    matrix m, inv;
    vm_MakeIdentity(&m);
    vm_MakeInverseMatrix(&inv);
    EXPECT_FLOAT_EQ(inv.rvec.x(), -1.0f);
    EXPECT_FLOAT_EQ(inv.uvec.y(), -1.0f);
    EXPECT_FLOAT_EQ(inv.fvec.z(), -1.0f);
}

// ============================================================================
// vm_SinCosToMatrix
// ============================================================================

/**
 * @test VecmatUtil.vm_SinCosToMatrix_MatchesDocumentedFormula
 * @brief Verifies vm Sin Cos To Matrix Matches Documented Formula.
 *
 * @details
 * Exercises the VecmatUtil code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see vecmat/vector.cpp, lib/vecmat_external.h, fix/fix.h
 * @ingroup descent3_tests
 */
TEST(VecmatUtil, vm_SinCosToMatrix_MatchesDocumentedFormula) {
    // p = 30deg, h = 45deg, b = 60deg
    const float p = 30.0f * PI / 180.0f;
    const float h = 45.0f * PI / 180.0f;
    const float b = 60.0f * PI / 180.0f;
    const float sinp = std::sin(p), cosp = std::cos(p);
    const float sinh = std::sin(h), cosh = std::cos(h);
    const float sinb = std::sin(b), cosb = std::cos(b);

    matrix m;
    vm_SinCosToMatrix(&m, sinp, cosp, sinb, cosb, sinh, cosh);

    EXPECT_NEAR(m.rvec.x(), cosb * cosh + sinp * sinb * sinh, TOL);
    EXPECT_NEAR(m.uvec.x(), sinp * cosb * sinh - sinb * cosh, TOL);
    EXPECT_NEAR(m.fvec.x(), sinh * cosp, TOL);
    EXPECT_NEAR(m.rvec.y(), sinb * cosp, TOL);
    EXPECT_NEAR(m.uvec.y(), cosb * cosp, TOL);
    EXPECT_NEAR(m.fvec.y(), -sinp, TOL);
    EXPECT_NEAR(m.rvec.z(), sinp * sinb * cosh - cosb * sinh, TOL);
    EXPECT_NEAR(m.uvec.z(), sinb * sinh + sinp * cosb * cosh, TOL);
    EXPECT_NEAR(m.fvec.z(), cosh * cosp, TOL);
}

/**
 * @test VecmatUtil.vm_SinCosToMatrix_OrthonormalRows
 * @brief Verifies vm Sin Cos To Matrix Orthonormal Rows.
 *
 * @details
 * Exercises the VecmatUtil code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see vecmat/vector.cpp, lib/vecmat_external.h, fix/fix.h
 * @ingroup descent3_tests
 */
TEST(VecmatUtil, vm_SinCosToMatrix_OrthonormalRows) {
    // Consistent sin/cos triples so the result is a proper rotation.
    const float sinp = 0.3f, cosp = std::sqrt(1.0f - 0.3f * 0.3f);
    const float sinb = 0.5f, cosb = std::sqrt(1.0f - 0.5f * 0.5f);
    const float sinh = -0.2f, cosh = std::sqrt(1.0f - 0.2f * 0.2f);
    matrix m;
    vm_SinCosToMatrix(&m, sinp, cosp, sinb, cosb, sinh, cosh);
    // Rows are unit length and mutually perpendicular.
    EXPECT_NEAR(vm_GetMagnitude(&m.rvec), 1.0f, 1e-2f);
    EXPECT_NEAR(vm_GetMagnitude(&m.uvec), 1.0f, 1e-2f);
    EXPECT_NEAR(vm_GetMagnitude(&m.fvec), 1.0f, 1e-2f);
    EXPECT_NEAR(vm_Dot3Product(m.rvec, m.uvec), 0.0f, 1e-2f);
    EXPECT_NEAR(vm_Dot3Product(m.rvec, m.fvec), 0.0f, 1e-2f);
    EXPECT_NEAR(vm_Dot3Product(m.uvec, m.fvec), 0.0f, 1e-2f);
}

// ============================================================================
// vm_VectorToMatrix
// ============================================================================

/**
 * @test VecmatUtil.vm_VectorToMatrix_ForwardZGivesIdentity
 * @brief Verifies vm Vector To Matrix Forward ZGives Identity.
 *
 * @details
 * Exercises the VecmatUtil code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see vecmat/vector.cpp, lib/vecmat_external.h, fix/fix.h
 * @ingroup descent3_tests
 */
TEST(VecmatUtil, vm_VectorToMatrix_ForwardZGivesIdentity) {
    matrix m;
    vector fwd = {0.0f, 0.0f, 1.0f};
    vm_VectorToMatrix(&m, &fwd, nullptr, nullptr);
    EXPECT_NEAR(m.rvec.x(), 1.0f, TOL);
    EXPECT_NEAR(m.uvec.y(), 1.0f, TOL);
    EXPECT_NEAR(m.fvec.z(), 1.0f, TOL);
    EXPECT_NEAR(m.rvec.y(), 0.0f, TOL);
    EXPECT_NEAR(m.uvec.x(), 0.0f, TOL);
}

/**
 * @test VecmatUtil.vm_VectorToMatrix_StraightUpDegenerateCase
 * @brief Verifies vm Vector To Matrix Straight Up Degenerate Case.
 *
 * @details
 * Exercises the VecmatUtil code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see vecmat/vector.cpp, lib/vecmat_external.h, fix/fix.h
 * @ingroup descent3_tests
 */
TEST(VecmatUtil, vm_VectorToMatrix_StraightUpDegenerateCase) {
    matrix m;
    vector fwd = {0.0f, 1.0f, 0.0f};
    vm_VectorToMatrix(&m, &fwd, nullptr, nullptr);
    // Documented degenerate handling: right = {1,0,0}, up = {0,0,-1}.
    EXPECT_NEAR(m.rvec.x(), 1.0f, TOL);
    EXPECT_NEAR(m.uvec.z(), -1.0f, TOL);
    EXPECT_NEAR(m.fvec.y(), 1.0f, TOL);
    EXPECT_NEAR(m.rvec.y(), 0.0f, TOL);
    EXPECT_NEAR(m.rvec.z(), 0.0f, TOL);
    EXPECT_NEAR(m.uvec.x(), 0.0f, TOL);
    EXPECT_NEAR(m.uvec.y(), 0.0f, TOL);
    EXPECT_NEAR(m.fvec.x(), 0.0f, TOL);
    EXPECT_NEAR(m.fvec.z(), 0.0f, TOL);
}

/**
 * @test VecmatUtil.vm_VectorToMatrix_FromForwardAndUp
 * @brief Verifies vm Vector To Matrix From Forward And Up.
 *
 * @details
 * Exercises the VecmatUtil code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see vecmat/vector.cpp, lib/vecmat_external.h, fix/fix.h
 * @ingroup descent3_tests
 */
TEST(VecmatUtil, vm_VectorToMatrix_FromForwardAndUp) {
    matrix m;
    vector fwd = {1.0f, 0.0f, 0.0f};
    vector up = {0.0f, 1.0f, 0.0f};
    vm_VectorToMatrix(&m, &fwd, &up, nullptr);
    // fwd along +x, up along +y => right = up x fwd = {0,0,-1}.
    EXPECT_NEAR(m.fvec.x(), 1.0f, TOL);
    EXPECT_NEAR(m.uvec.y(), 1.0f, TOL);
    EXPECT_NEAR(m.rvec.z(), -1.0f, TOL);
    EXPECT_NEAR(vm_GetMagnitude(&m.rvec), 1.0f, TOL);
    EXPECT_NEAR(vm_GetMagnitude(&m.uvec), 1.0f, TOL);
    EXPECT_NEAR(vm_GetMagnitude(&m.fvec), 1.0f, TOL);
}

// ============================================================================
// vm_VectorAngleToMatrix
// ============================================================================

/**
 * @test VecmatUtil.vm_VectorAngleToMatrix_ZeroRollIsIdentity
 * @brief Verifies vm Vector Angle To Matrix Zero Roll Is Identity.
 *
 * @details
 * Exercises the VecmatUtil code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see vecmat/vector.cpp, lib/vecmat_external.h, fix/fix.h
 * @ingroup descent3_tests
 */
TEST(VecmatUtil, vm_VectorAngleToMatrix_ZeroRollIsIdentity) {
    matrix m;
    vector fwd = {0.0f, 0.0f, 1.0f};
    vm_VectorAngleToMatrix(&m, &fwd, 0);
    EXPECT_NEAR(m.rvec.x(), 1.0f, TOL);
    EXPECT_NEAR(m.uvec.y(), 1.0f, TOL);
    EXPECT_NEAR(m.fvec.z(), 1.0f, TOL);
}

/**
 * @test VecmatUtil.vm_VectorAngleToMatrix_RollRotatesRightVector
 * @brief Verifies vm Vector Angle To Matrix Roll Rotates Right Vector.
 *
 * @details
 * Exercises the VecmatUtil code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see vecmat/vector.cpp, lib/vecmat_external.h, fix/fix.h
 * @ingroup descent3_tests
 */
TEST(VecmatUtil, vm_VectorAngleToMatrix_RollRotatesRightVector) {
    matrix m;
    vector fwd = {0.0f, 0.0f, 1.0f};
    // 90-degree roll (16384/65536 of a circle).
    vm_VectorAngleToMatrix(&m, &fwd, 16384);
    // Forward stays along +z, right/up rotate by 90 degrees.
    EXPECT_NEAR(m.fvec.z(), 1.0f, 1e-3f);
    EXPECT_NEAR(m.rvec.y(), 1.0f, 1e-3f);
    EXPECT_NEAR(m.rvec.x(), 0.0f, 1e-3f);
    EXPECT_NEAR(m.uvec.x(), -1.0f, 1e-3f);
    EXPECT_NEAR(m.uvec.y(), 0.0f, 1e-3f);
}

// ============================================================================
// vm_DeltaAngVec / vm_DeltaAngVecNorm
// ============================================================================

/**
 * @test VecmatUtil.vm_DeltaAngVecNorm_NinetyDegrees
 * @brief Verifies vm Delta Ang Vec Norm Ninety Degrees.
 *
 * @details
 * Exercises the VecmatUtil code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see vecmat/vector.cpp, lib/vecmat_external.h, fix/fix.h
 * @ingroup descent3_tests
 */
TEST(VecmatUtil, vm_DeltaAngVecNorm_NinetyDegrees) {
    vector v0 = {1.0f, 0.0f, 0.0f};
    vector v1 = {0.0f, 1.0f, 0.0f};
    angle a = vm_DeltaAngVecNorm(&v0, &v1, nullptr);
    EXPECT_EQ(a, 16384); // 90 degrees in angle units
}

/**
 * @test VecmatUtil.vm_DeltaAngVecNorm_OppositeVectorsClampToZero
 * @brief Verifies vm Delta Ang Vec Norm Opposite Vectors Clamp To Zero.
 *
 * @details
 * Exercises the VecmatUtil code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see vecmat/vector.cpp, lib/vecmat_external.h, fix/fix.h
 * @ingroup descent3_tests
 */
TEST(VecmatUtil, vm_DeltaAngVecNorm_OppositeVectorsClampToZero) {
    vector v0 = {1.0f, 0.0f, 0.0f};
    vector v1 = {-1.0f, 0.0f, 0.0f};
    // FixAcos(|v|>=1) returns 0 before applying the sign, so 180 degrees
    // wraps around to 0 in the LUT.
    angle a = vm_DeltaAngVecNorm(&v0, &v1, nullptr);
    EXPECT_EQ(a, 0);
}

/**
 * @test VecmatUtil.vm_DeltaAngVecNorm_SignAroundAxis
 * @brief Verifies vm Delta Ang Vec Norm Sign Around Axis.
 *
 * @details
 * Exercises the VecmatUtil code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see vecmat/vector.cpp, lib/vecmat_external.h, fix/fix.h
 * @ingroup descent3_tests
 */
TEST(VecmatUtil, vm_DeltaAngVecNorm_SignAroundAxis) {
    vector v0 = {1.0f, 0.0f, 0.0f};
    vector v1 = {0.0f, 1.0f, 0.0f};
    vector axis_pos = {0.0f, 0.0f, 1.0f};
    vector axis_neg = {0.0f, 0.0f, -1.0f};
    angle pos = vm_DeltaAngVecNorm(&v0, &v1, &axis_pos);
    angle neg = vm_DeltaAngVecNorm(&v0, &v1, &axis_neg);
    // Same magnitude, opposite sign (mod 65536).
    EXPECT_EQ(pos, 16384);
    EXPECT_EQ(neg, 49152);
}

/**
 * @test VecmatUtil.vm_DeltaAngVec_NonNormalizedInput
 * @brief Verifies vm Delta Ang Vec Non Normalized Input.
 *
 * @details
 * Exercises the VecmatUtil code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see vecmat/vector.cpp, lib/vecmat_external.h, fix/fix.h
 * @ingroup descent3_tests
 */
TEST(VecmatUtil, vm_DeltaAngVec_NonNormalizedInput) {
    vector v0 = {2.0f, 0.0f, 0.0f};
    vector v1 = {0.0f, 3.0f, 0.0f};
    angle a = vm_DeltaAngVec(&v0, &v1, nullptr);
    EXPECT_EQ(a, 16384);
}

// ============================================================================
// vm_MatrixMulTMatrix / vm_VectorMulTMatrix
// ============================================================================

static float MatElement(const matrix &m, int row, int col) {
    const vector *r = (row == 0) ? &m.rvec : (row == 1) ? &m.uvec : &m.fvec;
    return (col == 0) ? r->x() : (col == 1) ? r->y() : r->z();
}

/**
 * @test VecmatUtil.vm_MatrixMulTMatrix_MatchesManualTransposeMultiply
 * @brief Verifies vm Matrix Mul TMatrix Matches Manual Transpose Multiply.
 *
 * @details
 * Exercises the VecmatUtil code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see vecmat/vector.cpp, lib/vecmat_external.h, fix/fix.h
 * @ingroup descent3_tests
 */
TEST(VecmatUtil, vm_MatrixMulTMatrix_MatchesManualTransposeMultiply) {
    matrix a, b, dest;
    a.rvec = {1.0f, 2.0f, 3.0f};
    a.uvec = {4.0f, 5.0f, 6.0f};
    a.fvec = {7.0f, 8.0f, 9.0f};
    b.rvec = {9.0f, 8.0f, 7.0f};
    b.uvec = {6.0f, 5.0f, 4.0f};
    b.fvec = {3.0f, 2.0f, 1.0f};

    vm_MatrixMulTMatrix(&dest, &a, &b);
    // The implementation computes dest = b^T * a:
    // dest[i][j] = sum_k b[k][i] * a[k][j].
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            float sum = 0.0f;
            for (int k = 0; k < 3; k++) {
                sum += MatElement(b, k, i) * MatElement(a, k, j);
            }
            EXPECT_NEAR(MatElement(dest, i, j), sum, TOL);
        }
    }
}

/**
 * @test VecmatUtil.vm_VectorMulTMatrix_MatchesManualTransposeMultiply
 * @brief Verifies vm Vector Mul TMatrix Matches Manual Transpose Multiply.
 *
 * @details
 * Exercises the VecmatUtil code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see vecmat/vector.cpp, lib/vecmat_external.h, fix/fix.h
 * @ingroup descent3_tests
 */
TEST(VecmatUtil, vm_VectorMulTMatrix_MatchesManualTransposeMultiply) {
    matrix m;
    m.rvec = {1.0f, 2.0f, 3.0f};
    m.uvec = {4.0f, 5.0f, 6.0f};
    m.fvec = {7.0f, 8.0f, 9.0f};
    vector v = {1.0f, 1.0f, 1.0f};
    vector r;
    vm_VectorMulTMatrix(&r, &v, &m);
    // result = m^T * v: result.x = sum_k v[k] * m.rvec[k] (column 1 of m).
    EXPECT_NEAR(r.x(), m.rvec.x() * v.x() + m.uvec.x() * v.y() + m.fvec.x() * v.z(), TOL);
    EXPECT_NEAR(r.y(), m.rvec.y() * v.x() + m.uvec.y() * v.y() + m.fvec.y() * v.z(), TOL);
    EXPECT_NEAR(r.z(), m.rvec.z() * v.x() + m.uvec.z() * v.y() + m.fvec.z() * v.z(), TOL);
}
