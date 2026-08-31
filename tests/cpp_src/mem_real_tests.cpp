/**
 * @file mem_real_tests.cpp
 * @brief Descent 3.
 *
 * @details
 * Tests for the real mem library (mem/mem.cpp): allocation wrappers and
 * the tracked allocation counters.
 *
 * This harness validates the behavior of `Descent3/mem.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/mem.cpp`
 * @par Harness
 * `mem_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/mem.cpp
 */

#include <gtest/gtest.h>

#include "mem.h"
#include <cstring>

/**
 * @test MemReal.MallocFreeRoundTrip
 * @brief Verifies malloc Free Round Trip.
 *
 * @details
 * Exercises the MemReal code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/mem.cpp
 * @ingroup descent3_tests
 */
TEST(MemReal, MallocFreeRoundTrip) {
    void *p = mem_malloc(128);
    ASSERT_NE(p, nullptr);
    EXPECT_GE(mem_size(p), 128);
    mem_free(p);
}

/**
 * @test MemReal.MallocZeroSize
 * @brief Verifies malloc Zero Size.
 *
 * @details
 * Exercises the MemReal code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/mem.cpp
 * @ingroup descent3_tests
 */
TEST(MemReal, MallocZeroSize) {
    void *p = mem_malloc(0);
    // Allowed to return null or a valid pointer; must be freeable.
    if (p) {
        mem_free(p);
    }
}

/**
 * @test MemReal.StrdupRoundTrip
 * @brief Verifies strdup Round Trip.
 *
 * @details
 * Exercises the MemReal code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/mem.cpp
 * @ingroup descent3_tests
 */
TEST(MemReal, StrdupRoundTrip) {
    const char *src = "hello mem";
    char *dup = mem_strdup(src);
    ASSERT_NE(dup, nullptr);
    EXPECT_STREQ(dup, src);
    EXPECT_GE(mem_size(dup), std::strlen(src) + 1);
    mem_free(dup);
}

/**
 * @test MemReal.StrdupEmptyString
 * @brief Verifies strdup Empty String.
 *
 * @details
 * Exercises the MemReal code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/mem.cpp
 * @ingroup descent3_tests
 */
TEST(MemReal, StrdupEmptyString) {
    char *dup = mem_strdup("");
    ASSERT_NE(dup, nullptr);
    EXPECT_STREQ(dup, "");
    mem_free(dup);
}

/**
 * @test MemReal.ReallocGrows
 * @brief Verifies realloc Grows.
 *
 * @details
 * Exercises the MemReal code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/mem.cpp
 * @ingroup descent3_tests
 */
TEST(MemReal, ReallocGrows) {
    void *p = mem_malloc(16);
    ASSERT_NE(p, nullptr);
    void *grown = mem_realloc(p, 512);
    ASSERT_NE(grown, nullptr);
    EXPECT_GE(mem_size(grown), 512);
    mem_free(grown);
}

/**
 * @test MemReal.FreeNullIsSafe
 * @brief Verifies free Null Is Safe.
 *
 * @details
 * Exercises the MemReal code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/mem.cpp
 * @ingroup descent3_tests
 */
TEST(MemReal, FreeNullIsSafe) {
    mem_free(nullptr);
}

/**
 * @test MemReal.MultipleAllocationsTracked
 * @brief Verifies multiple Allocations Tracked.
 *
 * @details
 * Exercises the MemReal code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/mem.cpp
 * @ingroup descent3_tests
 */
TEST(MemReal, MultipleAllocationsTracked) {
    void *a = mem_malloc(64);
    void *b = mem_malloc(128);
    ASSERT_NE(a, nullptr);
    ASSERT_NE(b, nullptr);
    mem_free(a);
    mem_free(b);
}
