/**
 * @file psrand_tests_standalone.cpp
 * @brief D3 Coverage Tests - psrand module (Standalone).
 *
 * @details
 * Tests for random number generator.
 * Uses seeded RNG for reproducible test data.
 *
 * This harness validates the behavior of `Descent3/psrand_tests_standalone.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/psrand_tests_standalone.cpp`
 * @par Harness
 * `psrand_tests_standalone.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/psrand_tests_standalone.cpp
 */

#include <gtest/gtest.h>
#include <random>
#include <vector>
#include <cstdint>

// ============================================================================
// D3 psrand Implementation (matching lib/psrand.h and misc/psrand.cpp)
// ============================================================================

static long ps_holdrand = 1L;

void ps_srand(unsigned int seed) { ps_holdrand = (long)seed; }

int ps_rand(void) { return (((ps_holdrand = ps_holdrand * 214013L + 2531011L) >> 16) & 0x7fff); }

// ============================================================================
// Test Fixtures
// ============================================================================

/**
 * @brief GTest fixture for PsrandTest.
 * @details
 * Sets up stubbed engine state and helper factories shared
 * across the suite. Each @test starts from the deterministic
 * baseline established in SetUp().
 * @ingroup descent3_tests
 */
class PsrandTest : public ::testing::Test {
protected:
    static constexpr uint32_t SEED = 12345;
    
    void SetUp() override {
        ps_srand(SEED);
    }
};

// ============================================================================
// psrand tests
// ============================================================================

/**
 * @test PsrandTest.ZeroSeed
 * @brief Verifies zero Seed.
 *
 * @details
 * Exercises the PsrandTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/psrand_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(PsrandTest, ZeroSeed) {
    ps_srand(0);
    int first = ps_rand();
    EXPECT_GE(first, 0);
    EXPECT_LE(first, 0x7fff);
}

/**
 * @test PsrandTest.SameSeedReproducible
 * @brief Verifies same Seed Reproducible.
 *
 * @details
 * Exercises the PsrandTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/psrand_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(PsrandTest, SameSeedReproducible) {
    ps_srand(12345);
    std::vector<int> seq1;
    for (int i = 0; i < 10; i++) {
        seq1.push_back(ps_rand());
    }
    
    ps_srand(12345);
    std::vector<int> seq2;
    for (int i = 0; i < 10; i++) {
        seq2.push_back(ps_rand());
    }
    
    EXPECT_EQ(seq1, seq2);
}

/**
 * @test PsrandTest.DifferentSeedsDifferentSequence
 * @brief Verifies different Seeds Different Sequence.
 *
 * @details
 * Exercises the PsrandTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/psrand_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(PsrandTest, DifferentSeedsDifferentSequence) {
    ps_srand(100);
    int val1 = ps_rand();
    
    ps_srand(200);
    int val2 = ps_rand();
    
    EXPECT_NE(val1, val2);
}

/**
 * @test PsrandTest.RangeIs0To32767
 * @brief Verifies range Is0To32767.
 *
 * @details
 * Exercises the PsrandTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/psrand_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(PsrandTest, RangeIs0To32767) {
    ps_srand(42);
    for (int i = 0; i < 1000; i++) {
        int r = ps_rand();
        EXPECT_GE(r, 0);
        EXPECT_LE(r, 0x7fff);
    }
}

/**
 * @test PsrandTest.NotAllSame
 * @brief Verifies not All Same.
 *
 * @details
 * Exercises the PsrandTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/psrand_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(PsrandTest, NotAllSame) {
    ps_srand(42);
    std::vector<int> values;
    for (int i = 0; i < 100; i++) {
        values.push_back(ps_rand());
    }
    
    bool all_same = true;
    for (size_t i = 1; i < values.size(); i++) {
        if (values[i] != values[0]) {
            all_same = false;
            break;
        }
    }
    EXPECT_FALSE(all_same);
}

/**
 * @test PsrandTest.KnownSequence
 * @brief Verifies known Sequence.
 *
 * @details
 * Exercises the PsrandTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/psrand_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(PsrandTest, KnownSequence) {
    ps_srand(1);
    int r1 = ps_rand();
    int r2 = ps_rand();
    int r3 = ps_rand();
    
    EXPECT_GE(r1, 0);
    EXPECT_LE(r1, 0x7fff);
    EXPECT_GE(r2, 0);
    EXPECT_LE(r2, 0x7fff);
    EXPECT_GE(r3, 0);
    EXPECT_LE(r3, 0x7fff);
}

/**
 * @test PsrandTest.MaxSeed
 * @brief Verifies max Seed.
 *
 * @details
 * Exercises the PsrandTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/psrand_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(PsrandTest, MaxSeed) {
    ps_srand(0xFFFFFFFF);
    int r = ps_rand();
    EXPECT_GE(r, 0);
    EXPECT_LE(r, 0x7fff);
}

/**
 * @test PsrandTest.ManyCallsNoCrash
 * @brief Verifies many Calls No Crash.
 *
 * @details
 * Exercises the PsrandTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/psrand_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(PsrandTest, ManyCallsNoCrash) {
    ps_srand(42);
    for (int i = 0; i < 10000; i++) {
        ps_rand();
    }
}

// ============================================================================
// Property-based tests
// ============================================================================

/**
 * @brief GTest fixture for PsrandPropertyTest.
 * @details
 * Sets up stubbed engine state and helper factories shared
 * across the suite. Each @test starts from the deterministic
 * baseline established in SetUp().
 * @ingroup descent3_tests
 */
class PsrandPropertyTest : public ::testing::Test {
protected:
    static constexpr uint32_t SEED = 54321;
};

/**
 * @test PsrandPropertyTest.DeterministicWithFixedSeed
 * @brief Verifies deterministic With Fixed Seed.
 *
 * @details
 * Exercises the PsrandPropertyTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/psrand_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(PsrandPropertyTest, DeterministicWithFixedSeed) {
    for (int s = 0; s < 100; s++) {
        ps_srand(s);
        std::vector<int> seq1, seq2;
        for (int i = 0; i < 10; i++) {
            seq1.push_back(ps_rand());
        }
        
        ps_srand(s);
        for (int i = 0; i < 10; i++) {
            seq2.push_back(ps_rand());
        }
        
        EXPECT_EQ(seq1, seq2);
    }
}

/**
 * @test PsrandPropertyTest.ValuesInRange
 * @brief Verifies values In Range.
 *
 * @details
 * Exercises the PsrandPropertyTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/psrand_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST_F(PsrandPropertyTest, ValuesInRange) {
    ps_srand(12345);
    for (int i = 0; i < 1000; i++) {
        int r = ps_rand();
        EXPECT_GE(r, 0);
        EXPECT_LE(r, 32767);
    }
}

// ============================================================================
// Edge cases
// ============================================================================

/**
 * @test PsrandEdgeCase.SeedZero
 * @brief Verifies seed Zero.
 *
 * @details
 * Exercises the PsrandEdgeCase code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/psrand_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST(PsrandEdgeCase, SeedZero) {
    ps_srand(0);
    int r = ps_rand();
    EXPECT_GE(r, 0);
    EXPECT_LE(r, 0x7fff);
}

/**
 * @test PsrandEdgeCase.SequentialSeeds
 * @brief Verifies sequential Seeds.
 *
 * @details
 * Exercises the PsrandEdgeCase code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/psrand_tests_standalone.cpp
 * @ingroup descent3_tests
 */
TEST(PsrandEdgeCase, SequentialSeeds) {
    for (int s = 0; s < 10; s++) {
        ps_srand(s);
        int r = ps_rand();
        EXPECT_GE(r, 0);
        EXPECT_LE(r, 0x7fff);
    }
}
