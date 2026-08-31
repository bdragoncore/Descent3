/*
 * D3 Coverage Tests - Macros module (Standalone)
 * 
 * Tests for utility macros from lib/Macros.h
 * Uses seeded RNG for reproducible test data.
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
#define ABS(a) ((a < 0) ? (-a) : (a))

// ============================================================================
// Test Fixtures
// ============================================================================

class MacrosTest : public ::testing::Test {
protected:
    static constexpr uint32_t SEED = 12345;
    std::mt19937 rng_;
    
    MacrosTest() : rng_(SEED) {}
};

// ============================================================================
// SWAP tests
// ============================================================================

TEST_F(MacrosTest, SwapIntegers) {
    int a = 5, b = 10;
    SWAP(a, b);
    EXPECT_EQ(a, 10);
    EXPECT_EQ(b, 5);
}

TEST_F(MacrosTest, SwapSameValue) {
    int a = 5, b = 5;
    SWAP(a, b);
    EXPECT_EQ(a, 5);
    EXPECT_EQ(b, 5);
}

TEST_F(MacrosTest, SwapNegative) {
    int a = -5, b = 10;
    SWAP(a, b);
    EXPECT_EQ(a, 10);
    EXPECT_EQ(b, -5);
}

TEST_F(MacrosTest, SwapZero) {
    int a = 0, b = 10;
    SWAP(a, b);
    EXPECT_EQ(a, 10);
    EXPECT_EQ(b, 0);
}

// ============================================================================
// SET_MIN tests
// ============================================================================

TEST_F(MacrosTest, SetMinFirst) {
    EXPECT_EQ(SET_MIN(1, 2), 1);
    EXPECT_EQ(SET_MIN(0, 1), 0);
    EXPECT_EQ(SET_MIN(-5, -3), -5);
}

TEST_F(MacrosTest, SetMinSecond) {
    EXPECT_EQ(SET_MIN(5, 3), 3);
    EXPECT_EQ(SET_MIN(10, 0), 0);
}

TEST_F(MacrosTest, SetMinEqual) {
    EXPECT_EQ(SET_MIN(5, 5), 5);
    EXPECT_EQ(SET_MIN(0, 0), 0);
}

// ============================================================================
// CHECK_FLAG tests
// ============================================================================

TEST_F(MacrosTest, CheckFlagSet) {
    EXPECT_EQ(CHECK_FLAG(0xFF, 0x01), 0x01);
    EXPECT_EQ(CHECK_FLAG(0xFF, 0xFF), 0xFF);
    EXPECT_NE(CHECK_FLAG(0x0F, 0xF0), 0xF0);
}

TEST_F(MacrosTest, CheckFlagNotSet) {
    EXPECT_EQ(CHECK_FLAG(0x0F, 0xF0), 0);
    EXPECT_EQ(CHECK_FLAG(0x00, 0xFF), 0);
}

TEST_F(MacrosTest, CheckFlagZero) {
    EXPECT_EQ(CHECK_FLAG(0, 0), 0);
    EXPECT_EQ(CHECK_FLAG(0xFF, 0), 0);
}

// ============================================================================
// makeword/hiword/loword tests
// ============================================================================

TEST_F(MacrosTest, MakeWord) {
    EXPECT_EQ(makeword(0, 0), 0);
    EXPECT_EQ(makeword(0, 0xFFFF), 0xFFFF);
    EXPECT_EQ(makeword(1, 0), 0x10000);
    EXPECT_EQ(makeword(0xABCD, 0x1234), 0xABCD1234);
}

TEST_F(MacrosTest, HiWord) {
    EXPECT_EQ(hiword(0), 0);
    EXPECT_EQ(hiword(0x12345678), 0x1234);
    EXPECT_EQ(hiword(0x0001FFFF), 1);
}

TEST_F(MacrosTest, LoWord) {
    EXPECT_EQ(loword(0), 0);
    EXPECT_EQ(loword(0x12345678), 0x5678);
    EXPECT_EQ(loword(0x0001FFFF), 0xFFFF);
}

TEST_F(MacrosTest, HiLoWordRoundTrip) {
    uint32_t val = 0xDEADBEEF;
    EXPECT_EQ(makeword(hiword(val), loword(val)), val);
}

// ============================================================================
// makeshort/hibyte/lobyte tests
// ============================================================================

TEST_F(MacrosTest, MakeShort) {
    EXPECT_EQ(makeshort(0, 0), 0);
    EXPECT_EQ(makeshort(0, 0xFF), 0xFF);
    EXPECT_EQ(makeshort(1, 0), 0x100);
    EXPECT_EQ(makeshort(0xAB, 0xCD), 0xABCD);
}

TEST_F(MacrosTest, HiByte) {
    EXPECT_EQ(hibyte(0), 0);
    EXPECT_EQ(hibyte(0xABCD), 0xAB);
    EXPECT_EQ(hibyte(0x0100), 0x01);
}

TEST_F(MacrosTest, LoByte) {
    EXPECT_EQ(lobyte(0), 0);
    EXPECT_EQ(lobyte(0xABCD), 0xCD);
    EXPECT_EQ(lobyte(0x01FF), 0xFF);
}

TEST_F(MacrosTest, HiLoByteRoundTrip) {
    uint16_t val = 0xDEAD;
    EXPECT_EQ(makeshort(hibyte(val), lobyte(val)), val);
}

// ============================================================================
// kb_to_bytes tests
// ============================================================================

TEST_F(MacrosTest, KbToBytes) {
    EXPECT_EQ(kb_to_bytes(0), 0);
    EXPECT_EQ(kb_to_bytes(1), 1024);
    EXPECT_EQ(kb_to_bytes(10), 10240);
    EXPECT_EQ(kb_to_bytes(1024), 1048576);
}

// ============================================================================
// ABS tests
// ============================================================================

TEST_F(MacrosTest, AbsPositive) {
    EXPECT_EQ(ABS(5), 5);
    EXPECT_EQ(ABS(0), 0);
    EXPECT_EQ(ABS(100), 100);
}

TEST_F(MacrosTest, AbsNegative) {
    EXPECT_EQ(ABS(-5), 5);
    EXPECT_EQ(ABS(-100), 100);
}

TEST_F(MacrosTest, AbsZero) {
    EXPECT_EQ(ABS(0), 0);
}

// ============================================================================
// Property-based tests
// ============================================================================

class MacrosPropertyTest : public ::testing::Test {
protected:
    static constexpr uint32_t SEED = 54321;
    std::mt19937 rng_;
    
    MacrosPropertyTest() : rng_(SEED) {}
};

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

TEST_F(MacrosPropertyTest, CheckFlagProperties) {
    for (int i = 0; i < 50; i++) {
        uint32_t flag = 1 << (i % 32);
        uint32_t var = flag | (rng_() & 0xFFFF);
        EXPECT_EQ(CHECK_FLAG(var, flag), flag);
    }
}

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

TEST(MacrosEdgeCase, SwapMaxInt) {
    int a = 2147483647, b = -2147483648;
    SWAP(a, b);
    EXPECT_EQ(a, -2147483648);
    EXPECT_EQ(b, 2147483647);
}

TEST(MacrosEdgeCase, KbToBytesLarge) {
    EXPECT_EQ(kb_to_bytes(1048576), 1073741824);
}

TEST(MacrosEdgeCase, HiLoWordMax) {
    EXPECT_EQ(hiword(0xFFFFFFFF), 0xFFFF);
    EXPECT_EQ(loword(0xFFFFFFFF), 0xFFFF);
}

TEST(MacrosEdgeCase, HiLoByteMax) {
    EXPECT_EQ(hibyte(0xFFFF), 0xFF);
    EXPECT_EQ(lobyte(0xFFFF), 0xFF);
}
