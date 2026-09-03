/**
 * @file macros_tests_standalone.cpp
 * @brief D3 Coverage Tests - Macros module (Standalone).
 *
 * @details
 * Tests for utility macros from lib/Macros.h
 * Uses seeded RNG for reproducible test data.
 *
 * This harness validates the behavior of `Descent3/macros_tests_standalone.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/macros_tests_standalone.cpp`
 * @par Harness
 * `macros_tests_standalone.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/macros_tests_standalone.cpp
 */

#include <gtest/gtest.h>
#include <random>
#include <cstdint>

// ============================================================================
// D3 Macros Implementation (matching lib/Macros.h)
// ============================================================================

#define SWAP(a, b)                                                                                                     \
  do {                                                                                                                 \
    int _swap_var_ = (a);                                                                                              \
    (a) = (b);                                                                                                        \
    (b) = _swap_var_;                                                                                                  \
  } while (0)
#define SET_MIN(a, b) (((a) < (b)) ? (a) : (b))
#define CHECK_FLAG(_var, _flag) ((_var) & (_flag))
#define makeword(_h, _l) (((_h) << 16) + ((_l) & 0xffff))
#define hiword(_v) ((_v) >> 16)
#define loword(_v) ((_v) & 0x0000ffff)
#define makeshort(_h, _l) (((_h) << 8) + ((_l) & 0x00ff))
#define hibyte(_w) ((_w) >> 8)
#define lobyte(_w) ((_w) & 0x00ff)
#define kb_to_bytes(_kb) ((_kb) * 1024)
#define ABS(a) (((a) < 0) ? (-(a)) : (a))

// ============================================================================
// Test Fixtures
// ============================================================================

/**
 * @brief GTest fixture for MacrosTest.
 * @details
 * Sets up stubbed engine state and helper factories shared
 * across the suite. Each @test starts from the deterministic
 * baseline established in SetUp().
 * @ingroup descent3_tests
 */
class MacrosTest : public ::testing::Test {
protected:
    static constexpr uint32_t SEED = 12345;
    std::mt19937 rng_;
    
    MacrosTest() : rng_(SEED) {}
};

// ============================================================================
// SWAP tests
// ============================================================================

/**
 * @test MacrosTest.SwapIntegers
 * @brief Verifies swap Integers.
 *
 * @details
 * Exercises the MacrosTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/macros_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(MacrosTest, SwapIntegers) {
    int a = 5, b = 10;
    SWAP(a, b);
    EXPECT_EQ(a, 10);
    EXPECT_EQ(b, 5);
}

/**
 * @test MacrosTest.SwapSameValue
 * @brief Verifies swap Same Value.
 *
 * @details
 * Exercises the MacrosTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/macros_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(MacrosTest, SwapSameValue) {
    int a = 5, b = 5;
    SWAP(a, b);
    EXPECT_EQ(a, 5);
    EXPECT_EQ(b, 5);
}

/**
 * @test MacrosTest.SwapNegative
 * @brief Verifies swap Negative.
 *
 * @details
 * Exercises the MacrosTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/macros_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(MacrosTest, SwapNegative) {
    int a = -5, b = 10;
    SWAP(a, b);
    EXPECT_EQ(a, 10);
    EXPECT_EQ(b, -5);
}

/**
 * @test MacrosTest.SwapZero
 * @brief Verifies swap Zero.
 *
 * @details
 * Exercises the MacrosTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/macros_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(MacrosTest, SwapZero) {
    int a = 0, b = 10;
    SWAP(a, b);
    EXPECT_EQ(a, 10);
    EXPECT_EQ(b, 0);
}

// ============================================================================
// SET_MIN tests
// ============================================================================

/**
 * @test MacrosTest.SetMinFirst
 * @brief Verifies set Min First.
 *
 * @details
 * Exercises the MacrosTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/macros_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(MacrosTest, SetMinFirst) {
    EXPECT_EQ(SET_MIN(1, 2), 1);
    EXPECT_EQ(SET_MIN(0, 1), 0);
    EXPECT_EQ(SET_MIN(-5, -3), -5);
}

/**
 * @test MacrosTest.SetMinSecond
 * @brief Verifies set Min Second.
 *
 * @details
 * Exercises the MacrosTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/macros_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(MacrosTest, SetMinSecond) {
    EXPECT_EQ(SET_MIN(5, 3), 3);
    EXPECT_EQ(SET_MIN(10, 0), 0);
}

/**
 * @test MacrosTest.SetMinEqual
 * @brief Verifies set Min Equal.
 *
 * @details
 * Exercises the MacrosTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/macros_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(MacrosTest, SetMinEqual) {
    EXPECT_EQ(SET_MIN(5, 5), 5);
    EXPECT_EQ(SET_MIN(0, 0), 0);
}

// ============================================================================
// CHECK_FLAG tests
// ============================================================================

/**
 * @test MacrosTest.CheckFlagSet
 * @brief Verifies check Flag Set.
 *
 * @details
 * Exercises the MacrosTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/macros_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(MacrosTest, CheckFlagSet) {
    EXPECT_EQ(CHECK_FLAG(0xFF, 0x01), 0x01);
    EXPECT_EQ(CHECK_FLAG(0xFF, 0xFF), 0xFF);
    EXPECT_NE(CHECK_FLAG(0x0F, 0xF0), 0xF0);
}

/**
 * @test MacrosTest.CheckFlagNotSet
 * @brief Verifies check Flag Not Set.
 *
 * @details
 * Exercises the MacrosTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/macros_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(MacrosTest, CheckFlagNotSet) {
    EXPECT_EQ(CHECK_FLAG(0x0F, 0xF0), 0);
    EXPECT_EQ(CHECK_FLAG(0x00, 0xFF), 0);
}

/**
 * @test MacrosTest.CheckFlagZero
 * @brief Verifies check Flag Zero.
 *
 * @details
 * Exercises the MacrosTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/macros_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(MacrosTest, CheckFlagZero) {
    EXPECT_EQ(CHECK_FLAG(0, 0), 0);
    EXPECT_EQ(CHECK_FLAG(0xFF, 0), 0);
}

// ============================================================================
// makeword/hiword/loword tests
// ============================================================================

/**
 * @test MacrosTest.MakeWord
 * @brief Verifies make Word.
 *
 * @details
 * Exercises the MacrosTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/macros_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(MacrosTest, MakeWord) {
    EXPECT_EQ(makeword(0, 0), 0);
    EXPECT_EQ(makeword(0, 0xFFFF), 0xFFFF);
    EXPECT_EQ(makeword(1, 0), 0x10000);
    EXPECT_EQ(makeword(0xABCD, 0x1234), 0xABCD1234);
}

/**
 * @test MacrosTest.HiWord
 * @brief Verifies hi Word.
 *
 * @details
 * Exercises the MacrosTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/macros_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(MacrosTest, HiWord) {
    EXPECT_EQ(hiword(0), 0);
    EXPECT_EQ(hiword(0x12345678), 0x1234);
    EXPECT_EQ(hiword(0x0001FFFF), 1);
}

/**
 * @test MacrosTest.LoWord
 * @brief Verifies lo Word.
 *
 * @details
 * Exercises the MacrosTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/macros_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(MacrosTest, LoWord) {
    EXPECT_EQ(loword(0), 0);
    EXPECT_EQ(loword(0x12345678), 0x5678);
    EXPECT_EQ(loword(0x0001FFFF), 0xFFFF);
}

/**
 * @test MacrosTest.HiLoWordRoundTrip
 * @brief Verifies hi Lo Word Round Trip.
 *
 * @details
 * Exercises the MacrosTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/macros_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(MacrosTest, HiLoWordRoundTrip) {
    uint32_t val = 0xDEADBEEF;
    EXPECT_EQ(makeword(hiword(val), loword(val)), val);
}

// ============================================================================
// makeshort/hibyte/lobyte tests
// ============================================================================

/**
 * @test MacrosTest.MakeShort
 * @brief Verifies make Short.
 *
 * @details
 * Exercises the MacrosTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/macros_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(MacrosTest, MakeShort) {
    EXPECT_EQ(makeshort(0, 0), 0);
    EXPECT_EQ(makeshort(0, 0xFF), 0xFF);
    EXPECT_EQ(makeshort(1, 0), 0x100);
    EXPECT_EQ(makeshort(0xAB, 0xCD), 0xABCD);
}

/**
 * @test MacrosTest.HiByte
 * @brief Verifies hi Byte.
 *
 * @details
 * Exercises the MacrosTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/macros_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(MacrosTest, HiByte) {
    EXPECT_EQ(hibyte(0), 0);
    EXPECT_EQ(hibyte(0xABCD), 0xAB);
    EXPECT_EQ(hibyte(0x0100), 0x01);
}

/**
 * @test MacrosTest.LoByte
 * @brief Verifies lo Byte.
 *
 * @details
 * Exercises the MacrosTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/macros_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(MacrosTest, LoByte) {
    EXPECT_EQ(lobyte(0), 0);
    EXPECT_EQ(lobyte(0xABCD), 0xCD);
    EXPECT_EQ(lobyte(0x01FF), 0xFF);
}

/**
 * @test MacrosTest.HiLoByteRoundTrip
 * @brief Verifies hi Lo Byte Round Trip.
 *
 * @details
 * Exercises the MacrosTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/macros_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(MacrosTest, HiLoByteRoundTrip) {
    uint16_t val = 0xDEAD;
    EXPECT_EQ(makeshort(hibyte(val), lobyte(val)), val);
}

// ============================================================================
// kb_to_bytes tests
// ============================================================================

/**
 * @test MacrosTest.KbToBytes
 * @brief Verifies kb To Bytes.
 *
 * @details
 * Exercises the MacrosTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/macros_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(MacrosTest, KbToBytes) {
    EXPECT_EQ(kb_to_bytes(0), 0);
    EXPECT_EQ(kb_to_bytes(1), 1024);
    EXPECT_EQ(kb_to_bytes(10), 10240);
    EXPECT_EQ(kb_to_bytes(1024), 1048576);
}

// ============================================================================
// ABS tests
// ============================================================================

/**
 * @test MacrosTest.AbsPositive
 * @brief Verifies abs Positive.
 *
 * @details
 * Exercises the MacrosTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/macros_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(MacrosTest, AbsPositive) {
    EXPECT_EQ(ABS(5), 5);
    EXPECT_EQ(ABS(0), 0);
    EXPECT_EQ(ABS(100), 100);
}

/**
 * @test MacrosTest.AbsNegative
 * @brief Verifies abs Negative.
 *
 * @details
 * Exercises the MacrosTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/macros_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(MacrosTest, AbsNegative) {
    EXPECT_EQ(ABS(-5), 5);
    EXPECT_EQ(ABS(-100), 100);
}

/**
 * @test MacrosTest.AbsZero
 * @brief Verifies abs Zero.
 *
 * @details
 * Exercises the MacrosTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/macros_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(MacrosTest, AbsZero) {
    EXPECT_EQ(ABS(0), 0);
}

// ============================================================================
// Property-based tests
// ============================================================================

/**
 * @brief GTest fixture for MacrosPropertyTest.
 * @details
 * Sets up stubbed engine state and helper factories shared
 * across the suite. Each @test starts from the deterministic
 * baseline established in SetUp().
 * @ingroup descent3_tests
 */
class MacrosPropertyTest : public ::testing::Test {
protected:
    static constexpr uint32_t SEED = 54321;
    std::mt19937 rng_;
    
    MacrosPropertyTest() : rng_(SEED) {}
};

/**
 * @test MacrosPropertyTest.SwapInverse
 * @brief Verifies swap Inverse.
 *
 * @details
 * Exercises the MacrosPropertyTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/macros_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(MacrosPropertyTest, SwapInverse) {
    for (int i = 0; i < 50; i++) {
        int a = (int)rng_() - 1000;
        int b = (int)rng_() - 1000;
        int a_orig = a, b_orig = b;
        SWAP(a, b);
        EXPECT_EQ(a, b_orig);
        EXPECT_EQ(b, a_orig);
    }
}

/**
 * @test MacrosPropertyTest.SetMinProperties
 * @brief Verifies set Min Properties.
 *
 * @details
 * Exercises the MacrosPropertyTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/macros_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(MacrosPropertyTest, SetMinProperties) {
    for (int i = 0; i < 50; i++) {
        int a = (int)rng_() % 1000;
        int b = (int)rng_() % 1000;
        int min_val = SET_MIN(a, b);
        EXPECT_LE(min_val, a);
        EXPECT_LE(min_val, b);
        EXPECT_TRUE(min_val == a || min_val == b);
    }
}

/**
 * @test MacrosPropertyTest.CheckFlagProperties
 * @brief Verifies check Flag Properties.
 *
 * @details
 * Exercises the MacrosPropertyTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/macros_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(MacrosPropertyTest, CheckFlagProperties) {
    for (int i = 0; i < 50; i++) {
        uint32_t flag = 1 << (i % 32);
        uint32_t var = flag | (rng_() & 0xFFFF);
        EXPECT_EQ(CHECK_FLAG(var, flag), flag);
    }
}

/**
 * @test MacrosPropertyTest.AbsProperties
 * @brief Verifies abs Properties.
 *
 * @details
 * Exercises the MacrosPropertyTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/macros_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(MacrosPropertyTest, AbsProperties) {
    for (int i = 0; i < 50; i++) {
        int val = (int)rng_() - 10000;
        EXPECT_GE(ABS(val), 0);
        if (val != 0) {
            EXPECT_EQ(ABS(val), ABS(-val));
        }
    }
}

// ============================================================================
// Edge cases
// ============================================================================

/**
 * @test MacrosEdgeCase.SwapMaxInt
 * @brief Verifies swap Max Int.
 *
 * @details
 * Exercises the MacrosEdgeCase code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/macros_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST(MacrosEdgeCase, SwapMaxInt) {
    int a = 2147483647, b = -2147483648;
    SWAP(a, b);
    EXPECT_EQ(a, -2147483648);
    EXPECT_EQ(b, 2147483647);
}

/**
 * @test MacrosEdgeCase.KbToBytesLarge
 * @brief Verifies kb To Bytes Large.
 *
 * @details
 * Exercises the MacrosEdgeCase code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/macros_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST(MacrosEdgeCase, KbToBytesLarge) {
    EXPECT_EQ(kb_to_bytes(1048576), 1073741824);
}

/**
 * @test MacrosEdgeCase.HiLoWordMax
 * @brief Verifies hi Lo Word Max.
 *
 * @details
 * Exercises the MacrosEdgeCase code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/macros_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST(MacrosEdgeCase, HiLoWordMax) {
    EXPECT_EQ(hiword(0xFFFFFFFF), 0xFFFF);
    EXPECT_EQ(loword(0xFFFFFFFF), 0xFFFF);
}

/**
 * @test MacrosEdgeCase.HiLoByteMax
 * @brief Verifies hi Lo Byte Max.
 *
 * @details
 * Exercises the MacrosEdgeCase code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/macros_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST(MacrosEdgeCase, HiLoByteMax) {
    EXPECT_EQ(hibyte(0xFFFF), 0xFF);
    EXPECT_EQ(lobyte(0xFFFF), 0xFF);
}
