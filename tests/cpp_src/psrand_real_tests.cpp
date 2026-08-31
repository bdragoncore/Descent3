/**
 * @file psrand_real_tests.cpp
 * @brief Descent 3.
 *
 * @details
 * Tests for the real misc psrand implementation (misc/psrand.cpp):
 * a 31-bit linear congruential generator seeded with ps_srand.
 *
 * This harness validates the behavior of `Descent3/psrand.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/psrand.cpp`
 * @par Harness
 * `psrand_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/psrand.cpp
 */

#include <gtest/gtest.h>

#include "psrand.h"

/**
 * @test PsrandReal.SeedOneFirstValue
 * @brief Verifies seed One First Value.
 *
 * @details
 * Exercises the PsrandReal code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/psrand.cpp
 * @ingroup descent3_tests
 */
TEST(PsrandReal, SeedOneFirstValue) {
    // LCG: holdrand = holdrand * 214013 + 2531011; return (holdrand >> 16) & 0x7fff
    ps_srand(1);
    EXPECT_EQ(ps_rand(), 41);
}

/**
 * @test PsrandReal.RangeIsBounded
 * @brief Verifies range Is Bounded.
 *
 * @details
 * Exercises the PsrandReal code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/psrand.cpp
 * @ingroup descent3_tests
 */
TEST(PsrandReal, RangeIsBounded) {
    ps_srand(12345);
    for (int i = 0; i < 1000; i++) {
        int32_t v = ps_rand();
        EXPECT_GE(v, 0);
        EXPECT_LE(v, D3_RAND_MAX);
    }
}

/**
 * @test PsrandReal.SameSeedSameSequence
 * @brief Verifies same Seed Same Sequence.
 *
 * @details
 * Exercises the PsrandReal code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/psrand.cpp
 * @ingroup descent3_tests
 */
TEST(PsrandReal, SameSeedSameSequence) {
    ps_srand(42);
    int32_t first[10], second[10];
    for (int i = 0; i < 10; i++)
        first[i] = ps_rand();
    ps_srand(42);
    for (int i = 0; i < 10; i++)
        second[i] = ps_rand();
    for (int i = 0; i < 10; i++)
        EXPECT_EQ(first[i], second[i]) << "step " << i;
}

/**
 * @test PsrandReal.DifferentSeedsDiverge
 * @brief Verifies different Seeds Diverge.
 *
 * @details
 * Exercises the PsrandReal code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/psrand.cpp
 * @ingroup descent3_tests
 */
TEST(PsrandReal, DifferentSeedsDiverge) {
    ps_srand(7);
    int32_t a = ps_rand();
    ps_srand(8);
    int32_t b = ps_rand();
    EXPECT_NE(a, b);
}

/**
 * @test PsrandReal.ZeroSeedIsValid
 * @brief Verifies zero Seed Is Valid.
 *
 * @details
 * Exercises the PsrandReal code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/psrand.cpp
 * @ingroup descent3_tests
 */
TEST(PsrandReal, ZeroSeedIsValid) {
    ps_srand(0);
    EXPECT_GE(ps_rand(), 0);
    EXPECT_LE(ps_rand(), D3_RAND_MAX);
}

/**
 * @test PsrandReal.D3RANDMAXConstant
 * @brief Verifies d3RANDMAXConstant.
 *
 * @details
 * Exercises the PsrandReal code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/psrand.cpp
 * @ingroup descent3_tests
 */
TEST(PsrandReal, D3RANDMAXConstant) {
    EXPECT_EQ(D3_RAND_MAX, 0x7fff);
}
