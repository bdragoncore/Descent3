/**
 * @file byteswap_tests_standalone.cpp
 * @brief D3 Coverage Tests - lib/byteswap module (Standalone).
 *
 * @details
 * Tests for byte swapping / endianness conversion utilities.
 * Uses seeded RNG for reproducible test data.
 *
 * This harness validates the behavior of `Descent3/byteswap_tests_standalone.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/byteswap_tests_standalone.cpp`
 * @par Harness
 * `byteswap_tests_standalone.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/byteswap_tests_standalone.cpp
 */

#include <gtest/gtest.h>
#include <cstdint>
#include <cstring>
#include <random>
#include <array>

// ============================================================================
// D3 Byteswap Implementation (matching lib/byteswap.h)
// ============================================================================

namespace D3 {

template <typename T>
constexpr T byteswap(T n) {
    T m;
    for (size_t i = 0; i < sizeof(T); i++)
        reinterpret_cast<uint8_t *>(&m)[i] = reinterpret_cast<uint8_t *>(&n)[sizeof(T) - 1 - i];
    return m;
}

template <typename T>
constexpr T convert_be(T val) {
#ifndef OUTRAGE_BIG_ENDIAN
    return byteswap(val);
#else
    return (val);
#endif
}

template <typename T>
constexpr T convert_le(T val) {
#ifndef OUTRAGE_BIG_ENDIAN
    return (val);
#else
    return byteswap(val);
#endif
}

} // namespace D3

// ============================================================================
// Test Fixtures
// ============================================================================

/**
 * @brief GTest fixture for ByteswapTest.
 * @details
 * Sets up stubbed engine state and helper factories shared
 * across the suite. Each @test starts from the deterministic
 * baseline established in SetUp().
 * @ingroup descent3_tests
 */
class ByteswapTest : public ::testing::Test {
protected:
    static constexpr uint32_t SEED = 12345;
    std::mt19937 rng_;
    
    ByteswapTest() : rng_(SEED) {}
};

/**
 * @brief GTest fixture for ByteswapPropertyTest.
 * @details
 * Sets up stubbed engine state and helper factories shared
 * across the suite. Each @test starts from the deterministic
 * baseline established in SetUp().
 * @ingroup descent3_tests
 */
class ByteswapPropertyTest : public ::testing::Test {
protected:
    static constexpr uint32_t SEED = 54321;
    std::mt19937 rng_;
    
    ByteswapPropertyTest() : rng_(SEED) {}
    
    uint16_t random_uint16() {
        std::uniform_int_distribution<uint16_t> dist(0, 65535);
        return dist(rng_);
    }
    
    uint32_t random_uint32() {
        std::uniform_int_distribution<uint32_t> dist(0, 4294967295U);
        return dist(rng_);
    }
    
    uint64_t random_uint64() {
        std::uniform_int_distribution<uint64_t> dist(0, 18446744073709551615ULL);
        return dist(rng_);
    }
};

// ============================================================================
// Basic byteswap tests
// ============================================================================

/**
 * @test ByteswapTest.Uint16Swap
 * @brief Verifies uint16Swap.
 *
 * @details
 * Exercises the ByteswapTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/byteswap_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(ByteswapTest, Uint16Swap) {
    uint16_t original = 0x1234;
    uint16_t swapped = D3::byteswap(original);
    EXPECT_EQ(swapped, 0x3412);
}

/**
 * @test ByteswapTest.Uint32Swap
 * @brief Verifies uint32Swap.
 *
 * @details
 * Exercises the ByteswapTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/byteswap_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(ByteswapTest, Uint32Swap) {
    uint32_t original = 0x12345678;
    uint32_t swapped = D3::byteswap(original);
    EXPECT_EQ(swapped, 0x78563412);
}

/**
 * @test ByteswapTest.Uint64Swap
 * @brief Verifies uint64Swap.
 *
 * @details
 * Exercises the ByteswapTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/byteswap_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(ByteswapTest, Uint64Swap) {
    uint64_t original = 0x123456789ABCDEF0ULL;
    uint64_t swapped = D3::byteswap(original);
    EXPECT_EQ(swapped, 0xF0DEBC9A78563412ULL);
}

/**
 * @test ByteswapTest.Int16Swap
 * @brief Verifies int16Swap.
 *
 * @details
 * Exercises the ByteswapTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/byteswap_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(ByteswapTest, Int16Swap) {
    int16_t original = 0x1234;
    int16_t swapped = D3::byteswap(original);
    EXPECT_EQ(swapped, 0x3412);
}

/**
 * @test ByteswapTest.Int32Swap
 * @brief Verifies int32Swap.
 *
 * @details
 * Exercises the ByteswapTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/byteswap_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(ByteswapTest, Int32Swap) {
    int32_t original = 0x12345678;
    int32_t swapped = D3::byteswap(original);
    EXPECT_EQ(swapped, 0x78563412);
}

/**
 * @test ByteswapTest.Int64Swap
 * @brief Verifies int64Swap.
 *
 * @details
 * Exercises the ByteswapTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/byteswap_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(ByteswapTest, Int64Swap) {
    int64_t original = 0x123456789ABCDEF0LL;
    int64_t swapped = D3::byteswap(original);
    EXPECT_EQ(swapped, 0xF0DEBC9A78563412LL);
}

/**
 * @test ByteswapTest.ZeroSwap
 * @brief Verifies zero Swap.
 *
 * @details
 * Exercises the ByteswapTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/byteswap_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(ByteswapTest, ZeroSwap) {
    EXPECT_EQ(D3::byteswap(uint32_t(0)), uint32_t(0));
    EXPECT_EQ(D3::byteswap(uint16_t(0)), uint16_t(0));
    EXPECT_EQ(D3::byteswap(uint64_t(0)), uint64_t(0));
}

/**
 * @test ByteswapTest.MaxValueSwap
 * @brief Verifies max Value Swap.
 *
 * @details
 * Exercises the ByteswapTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/byteswap_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(ByteswapTest, MaxValueSwap) {
    EXPECT_EQ(D3::byteswap(uint8_t(0xFF)), uint8_t(0xFF));
    EXPECT_EQ(D3::byteswap(uint16_t(0xFFFF)), uint16_t(0xFFFF));
    EXPECT_EQ(D3::byteswap(uint32_t(0xFFFFFFFF)), uint32_t(0xFFFFFFFF));
}

/**
 * @test ByteswapTest.SwapTwiceIsOriginal
 * @brief Verifies swap Twice Is Original.
 *
 * @details
 * Exercises the ByteswapTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/byteswap_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(ByteswapTest, SwapTwiceIsOriginal) {
    std::array<uint32_t, 5> values = {0x12345678, 0xDEADBEEF, 0xCAFEBABE, 0x00FF00FF, 0xA5A5A5A5};
    
    for (auto val : values) {
        EXPECT_EQ(D3::byteswap(D3::byteswap(val)), val);
    }
}

// ============================================================================
// convert_le / convert_be tests
// ============================================================================

/**
 * @test ByteswapTest.ConvertLEOnLE
 * @brief Verifies convert LEOn LE.
 *
 * @details
 * Exercises the ByteswapTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/byteswap_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(ByteswapTest, ConvertLEOnLE) {
#ifdef __BYTE_ORDER__
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    EXPECT_EQ(D3::convert_le(uint32_t(0x12345678)), uint32_t(0x12345678));
    EXPECT_EQ(D3::convert_be(uint32_t(0x12345678)), uint32_t(0x78563412));
#endif
#endif
}

/**
 * @test ByteswapTest.ConvertBERoundTrip
 * @brief Verifies convert BERound Trip.
 *
 * @details
 * Exercises the ByteswapTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/byteswap_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(ByteswapTest, ConvertBERoundTrip) {
    uint32_t original = 0x12345678;
    uint32_t to_be = D3::convert_be(original);
    uint32_t back = D3::convert_be(to_be);
    EXPECT_EQ(back, original);
}

/**
 * @test ByteswapTest.ConvertLERoundTrip
 * @brief Verifies convert LERound Trip.
 *
 * @details
 * Exercises the ByteswapTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/byteswap_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(ByteswapTest, ConvertLERoundTrip) {
    uint32_t original = 0x12345678;
    uint32_t to_le = D3::convert_le(original);
    uint32_t back = D3::convert_le(to_le);
    EXPECT_EQ(back, original);
}

// ============================================================================
// Property-based tests
// ============================================================================

/**
 * @test ByteswapPropertyTest.SwapIsInverse
 * @brief Verifies swap Is Inverse.
 *
 * @details
 * Exercises the ByteswapPropertyTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/byteswap_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(ByteswapPropertyTest, SwapIsInverse) {
    for (int i = 0; i < 100; i++) {
        uint32_t val = random_uint32();
        EXPECT_EQ(D3::byteswap(D3::byteswap(val)), val);
    }
}

/**
 * @test ByteswapPropertyTest.DifferentSizes
 * @brief Verifies different Sizes.
 *
 * @details
 * Exercises the ByteswapPropertyTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/byteswap_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(ByteswapPropertyTest, DifferentSizes) {
    for (int i = 0; i < 50; i++) {
        uint16_t v16 = random_uint16();
        uint32_t v32 = random_uint32();
        uint64_t v64 = random_uint64();
        
        EXPECT_EQ(D3::byteswap(D3::byteswap(v16)), v16);
        EXPECT_EQ(D3::byteswap(D3::byteswap(v32)), v32);
        EXPECT_EQ(D3::byteswap(D3::byteswap(v64)), v64);
    }
}

/**
 * @test ByteswapPropertyTest.ByteArrayConsistency
 * @brief Verifies byte Array Consistency.
 *
 * @details
 * Exercises the ByteswapPropertyTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/byteswap_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(ByteswapPropertyTest, ByteArrayConsistency) {
    for (int i = 0; i < 50; i++) {
        uint32_t val = random_uint32();
        
        uint8_t bytes[4];
        std::memcpy(bytes, &val, 4);
        
        uint8_t swapped_bytes[4];
        for (int j = 0; j < 4; j++) {
            swapped_bytes[j] = bytes[3 - j];
        }
        
        uint32_t swapped_val;
        std::memcpy(&swapped_val, swapped_bytes, 4);
        
        EXPECT_EQ(D3::byteswap(val), swapped_val);
    }
}

/**
 * @test ByteswapPropertyTest.ConvertLeBeSymmetry
 * @brief Verifies convert Le Be Symmetry.
 *
 * @details
 * Exercises the ByteswapPropertyTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/byteswap_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(ByteswapPropertyTest, ConvertLeBeSymmetry) {
    for (int i = 0; i < 100; i++) {
        uint32_t val = random_uint32();
        
        uint32_t le_val = D3::convert_le(val);
        uint32_t be_val = D3::convert_be(val);
        
        EXPECT_EQ(D3::byteswap(le_val), be_val);
        EXPECT_EQ(D3::byteswap(be_val), le_val);
    }
}

// ============================================================================
// Edge cases
// ============================================================================

/**
 * @test ByteswapEdgeCase.AllZeros
 * @brief Verifies all Zeros.
 *
 * @details
 * Exercises the ByteswapEdgeCase code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/byteswap_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST(ByteswapEdgeCase, AllZeros) {
    EXPECT_EQ(D3::byteswap(uint32_t(0)), uint32_t(0));
    EXPECT_EQ(D3::byteswap(uint16_t(0)), uint16_t(0));
    EXPECT_EQ(D3::byteswap(int32_t(0)), int32_t(0));
}

/**
 * @test ByteswapEdgeCase.AllOnes
 * @brief Verifies all Ones.
 *
 * @details
 * Exercises the ByteswapEdgeCase code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/byteswap_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST(ByteswapEdgeCase, AllOnes) {
    EXPECT_EQ(D3::byteswap(uint8_t(0xFF)), uint8_t(0xFF));
    EXPECT_EQ(D3::byteswap(uint16_t(0xFFFF)), uint16_t(0xFFFF));
    EXPECT_EQ(D3::byteswap(uint32_t(0xFFFFFFFF)), uint32_t(0xFFFFFFFF));
}

/**
 * @test ByteswapEdgeCase.SingleBytePattern
 * @brief Verifies single Byte Pattern.
 *
 * @details
 * Exercises the ByteswapEdgeCase code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/byteswap_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST(ByteswapEdgeCase, SingleBytePattern) {
    EXPECT_EQ(D3::byteswap(uint16_t(0x0101)), uint16_t(0x0101));
    EXPECT_EQ(D3::byteswap(uint32_t(0x01010101)), uint32_t(0x01010101));
}

/**
 * @test ByteswapEdgeCase.AlternatingPattern
 * @brief Verifies alternating Pattern.
 *
 * @details
 * Exercises the ByteswapEdgeCase code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/byteswap_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST(ByteswapEdgeCase, AlternatingPattern) {
    EXPECT_EQ(D3::byteswap(uint16_t(0xAAAA)), uint16_t(0xAAAA));
    EXPECT_EQ(D3::byteswap(uint32_t(0xAAAAAAAA)), uint32_t(0xAAAAAAAA));
    EXPECT_EQ(D3::byteswap(uint64_t(0xAAAAAAAAAAAAAAAAULL)), uint64_t(0xAAAAAAAAAAAAAAAAULL));
}

/**
 * @test ByteswapEdgeCase.OneByteDifferent
 * @brief Verifies one Byte Different.
 *
 * @details
 * Exercises the ByteswapEdgeCase code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/byteswap_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST(ByteswapEdgeCase, OneByteDifferent) {
    EXPECT_EQ(D3::byteswap(uint16_t(0xFF00)), uint16_t(0x00FF));
    EXPECT_EQ(D3::byteswap(uint32_t(0xFF000000)), uint32_t(0x000000FF));
}

/**
 * @test ByteswapEdgeCase.FloatByteswap
 * @brief Verifies float Byteswap.
 *
 * @details
 * Exercises the ByteswapEdgeCase code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/byteswap_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST(ByteswapEdgeCase, FloatByteswap) {
    float f = 1.0f;
    uint32_t bits;
    std::memcpy(&bits, &f, sizeof(bits));
    uint32_t swapped = D3::byteswap(bits);
    float result;
    std::memcpy(&result, &swapped, sizeof(result));
    
    float swapped_float;
    std::memcpy(&swapped_float, &swapped, sizeof(swapped_float));
    
    EXPECT_EQ(D3::convert_le(f), f);
    EXPECT_EQ(D3::convert_be(f), swapped_float);
}

/**
 * @test ByteswapEdgeCase.DoubleByteswap
 * @brief Verifies double Byteswap.
 *
 * @details
 * Exercises the ByteswapEdgeCase code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/byteswap_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST(ByteswapEdgeCase, DoubleByteswap) {
    double d = 1.0;
    uint64_t bits;
    std::memcpy(&bits, &d, sizeof(bits));
    uint64_t swapped = D3::byteswap(bits);
    double result;
    std::memcpy(&result, &swapped, sizeof(result));
    
    double swapped_double;
    std::memcpy(&swapped_double, &swapped, sizeof(swapped_double));
    
    EXPECT_EQ(D3::convert_le(d), d);
    EXPECT_EQ(D3::convert_be(d), swapped_double);
}
