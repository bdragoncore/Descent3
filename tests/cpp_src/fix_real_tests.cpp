/**
 * @file fix_real_tests.cpp
 * @brief Descent 3.
 *
 * @details
 * Tests for the real fix library (fix/fix.cpp): LUT trig, conversions,
 * and the angle helpers.
 *
 * This harness validates the behavior of `Descent3/fix.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/fix.cpp`
 * @par Harness
 * `fix_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/fix.cpp
 */

#include <gtest/gtest.h>
#include "fix.h"
#include <cmath>

// The trig tables are initialized by the game's startup code; initialize
// them here for library use.
static struct MathTablesInit {
    MathTablesInit() { InitMathTables(); }
} math_tables_init;

// LUT tolerance: the sincos table has 256 entries with linear interpolation
// over a full circle, so results are accurate to roughly 1e-2.
static constexpr float LUT_TOL = 0.02f;

// ============================================================================
// Conversions
// ============================================================================

/**
 * @test FixReal.FloatToFix_FixToFloat_RoundTrip
 * @brief Verifies float To Fix Fix To Float Round Trip.
 *
 * @details
 * Exercises the FixReal code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/fix.cpp
 * @ingroup descent3_tests
 */
TEST(FixReal, FloatToFix_FixToFloat_RoundTrip) {
    for (float f : {0.0f, 1.0f, -1.0f, 0.5f, 3.25f, -12.75f, 0.0001f}) {
        EXPECT_NEAR(FixToFloat(FloatToFix(f)), f, 1e-4f) << "round trip of " << f;
    }
}

/**
 * @test FixReal.IntToFix_FixToInt_RoundTrip
 * @brief Verifies int To Fix Fix To Int Round Trip.
 *
 * @details
 * Exercises the FixReal code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/fix.cpp
 * @ingroup descent3_tests
 */
TEST(FixReal, IntToFix_FixToInt_RoundTrip) {
    for (int i : {0, 1, -1, 42, -1000, 32767}) {
        EXPECT_EQ(FixToInt(IntToFix(i)), i) << "round trip of " << i;
    }
}

/**
 * @test FixReal.IntToFix_MaximumValue
 * @brief Verifies int To Fix Maximum Value.
 *
 * @details
 * Exercises the FixReal code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/fix.cpp
 * @ingroup descent3_tests
 */
TEST(FixReal, IntToFix_MaximumValue) {
    // fix is a 32-bit value; IntToFix(65536) would overflow.
    EXPECT_EQ(IntToFix(32767), 32767 << 16);
}

/**
 * @test FixReal.FloatToFix_ScalesByFLOAT_SCALER
 * @brief Verifies float To Fix Scales By FLOAT SCALER.
 *
 * @details
 * Exercises the FixReal code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/fix.cpp
 * @ingroup descent3_tests
 */
TEST(FixReal, FloatToFix_ScalesByFLOAT_SCALER) {
    EXPECT_EQ(FloatToFix(1.0f), F1_0);
    EXPECT_EQ(FloatToFix(0.5f), F1_0 / 2);
}

// ============================================================================
// FixSin / FixCos
// ============================================================================

/**
 * @test FixReal.FixSinCos_QuarterTurns
 * @brief Verifies fix Sin Cos Quarter Turns.
 *
 * @details
 * Exercises the FixReal code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/fix.cpp
 * @ingroup descent3_tests
 */
TEST(FixReal, FixSinCos_QuarterTurns) {
    // angle units: 0 = 0deg, 16384 = 90deg, 32768 = 180deg, 49152 = 270deg
    EXPECT_NEAR(FixSin(0), 0.0f, LUT_TOL);
    EXPECT_NEAR(FixCos(0), 1.0f, LUT_TOL);
    EXPECT_NEAR(FixSin(16384), 1.0f, LUT_TOL);
    EXPECT_NEAR(FixCos(16384), 0.0f, LUT_TOL);
    EXPECT_NEAR(FixSin(32768), 0.0f, LUT_TOL);
    EXPECT_NEAR(FixCos(32768), -1.0f, LUT_TOL);
    EXPECT_NEAR(FixSin(49152), -1.0f, LUT_TOL);
    EXPECT_NEAR(FixCos(49152), 0.0f, LUT_TOL);
}

/**
 * @test FixReal.FixSinCos_MatchesStdTrig
 * @brief Verifies fix Sin Cos Matches Std Trig.
 *
 * @details
 * Exercises the FixReal code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/fix.cpp
 * @ingroup descent3_tests
 */
TEST(FixReal, FixSinCos_MatchesStdTrig) {
    // 16 evenly spaced angles around the circle (angles in 4096-unit steps)
    for (int a = 0; a < 65536; a += 4096) {
        double rad = a / 65536.0 * 2.0 * 3.14159265358979;
        EXPECT_NEAR(FixSin(a), std::sin(rad), LUT_TOL) << "angle " << a;
        EXPECT_NEAR(FixCos(a), std::cos(rad), LUT_TOL) << "angle " << a;
    }
}

/**
 * @test FixReal.FixSinCos_Identity
 * @brief Verifies fix Sin Cos Identity.
 *
 * @details
 * Exercises the FixReal code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/fix.cpp
 * @ingroup descent3_tests
 */
TEST(FixReal, FixSinCos_Identity) {
    for (int a = 0; a < 65536; a += 8192) {
        float s = FixSin(a), c = FixCos(a);
        EXPECT_NEAR(s * s + c * c, 1.0f, 0.05f) << "angle " << a;
    }
}

// ============================================================================
// FixAsin / FixAcos
// ============================================================================

/**
 * @test FixReal.FixAsin_KnownValues
 * @brief Verifies fix Asin Known Values.
 *
 * @details
 * Exercises the FixReal code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/fix.cpp
 * @ingroup descent3_tests
 */
TEST(FixReal, FixAsin_KnownValues) {
    EXPECT_EQ(FixAsin(0.0f), 0);
    EXPECT_EQ(FixAsin(1.0f), 16384);   // 90 degrees
    EXPECT_NEAR(FixAsin(0.7071f), 8192, 128); // ~45 degrees
}

/**
 * @test FixReal.FixAsin_OutOfRangeClampsToNinety
 * @brief Verifies fix Asin Out Of Range Clamps To Ninety.
 *
 * @details
 * Exercises the FixReal code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/fix.cpp
 * @ingroup descent3_tests
 */
TEST(FixReal, FixAsin_OutOfRangeClampsToNinety) {
    // |v| >= 1 returns 0x4000 (90 degrees) before the sign branch, so
    // FixAsin(-1) is also 90 degrees rather than -90.
    EXPECT_EQ(FixAsin(-1.0f), 16384);
    EXPECT_EQ(FixAsin(2.0f), 16384);
}

/**
 * @test FixReal.FixAcos_KnownValues
 * @brief Verifies fix Acos Known Values.
 *
 * @details
 * Exercises the FixReal code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/fix.cpp
 * @ingroup descent3_tests
 */
TEST(FixReal, FixAcos_KnownValues) {
    EXPECT_EQ(FixAcos(0.0f), 16384);   // 90 degrees
    EXPECT_EQ(FixAcos(1.0f), 0);       // 0 degrees
    EXPECT_NEAR(FixAcos(0.7071f), 8192, 128); // ~45 degrees
}

// ============================================================================
// FixAtan2 (left-handed: FixAtan2(cos, sin))
// ============================================================================

/**
 * @test FixReal.FixAtan2_Quadrants
 * @brief Verifies fix Atan2 Quadrants.
 *
 * @details
 * Exercises the FixReal code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/fix.cpp
 * @ingroup descent3_tests
 */
TEST(FixReal, FixAtan2_Quadrants) {
    EXPECT_EQ(FixAtan2(1.0f, 0.0f), 0);        // +x axis
    EXPECT_EQ(FixAtan2(0.0f, 1.0f), 16384);    // +y axis (90 deg)
    EXPECT_EQ(FixAtan2(-1.0f, 0.0f), 32768);   // -x axis (180 deg)
    EXPECT_EQ(FixAtan2(0.0f, -1.0f), 49152);   // -y axis (270 deg)
}

/**
 * @test FixReal.FixAtan2_Octants
 * @brief Verifies fix Atan2 Octants.
 *
 * @details
 * Exercises the FixReal code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/fix.cpp
 * @ingroup descent3_tests
 */
TEST(FixReal, FixAtan2_Octants) {
    // 45 degrees: cos = sin = sqrt(0.5)
    const float r = std::sqrt(0.5f);
    EXPECT_NEAR(FixAtan2(r, r), 8192, 128);         // +45 deg
    EXPECT_NEAR(FixAtan2(r, -r), 57344, 128);       // -45 deg (315 deg)
    EXPECT_NEAR(FixAtan2(-r, r), 8192 + 16384, 128); // 135 deg
    EXPECT_NEAR(FixAtan2(-r, -r), 57344 - 16384, 128); // 225 deg
}

/**
 * @test FixReal.FixAtan2_ZeroVector
 * @brief Verifies fix Atan2 Zero Vector.
 *
 * @details
 * Exercises the FixReal code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/fix.cpp
 * @ingroup descent3_tests
 */
TEST(FixReal, FixAtan2_ZeroVector) {
    EXPECT_EQ(FixAtan2(0.0f, 0.0f), 0);
}

/**
 * @test FixReal.FixAtan2_ScaleInvariant
 * @brief Verifies fix Atan2 Scale Invariant.
 *
 * @details
 * Exercises the FixReal code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/fix.cpp
 * @ingroup descent3_tests
 */
TEST(FixReal, FixAtan2_ScaleInvariant) {
    // "parms need not be normalized"
    EXPECT_NEAR(FixAtan2(3.0f, 3.0f), 8192, 128);
    EXPECT_EQ(FixAtan2(10.0f, 0.0f), 0);
}

// ============================================================================
// F1_0 / FIX_SHIFT constants
// ============================================================================

/**
 * @test FixReal.Constants
 * @brief Verifies constants.
 *
 * @details
 * Exercises the FixReal code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/fix.cpp
 * @ingroup descent3_tests
 */
TEST(FixReal, Constants) {
    EXPECT_EQ(F1_0, 65536);
    EXPECT_EQ(FIX_SHIFT, 16);
    EXPECT_EQ(FLOAT_SCALER, 65536.0f);
}
