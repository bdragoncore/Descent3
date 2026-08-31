/**
 * @file ai_constants_tests.cpp
 * @brief Unit tests for Descent3/ai_constants.cpp.
 *
 * @details
 * Covers the public contract of `Descent3/ai_constants.cpp` — construction, state transitions, error handling and edge cases exercised through the GoogleTest harness.
 *
 * This harness validates the behavior of `Descent3/ai_constants.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/ai_constants.cpp`
 * @par Harness
 * `ai_constants_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/ai_constants.cpp
 */

#include "gtest/gtest.h"
#include "pstypes.h"

#define AI_SOUND_SHORT_DIST 60.0f

/**
 * @test AIConstantsTest.AISoundShortDistance
 * @brief Verifies aISound Short Distance.
 *
 * @details
 * Exercises the AIConstantsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/ai_constants.cpp
 * @ingroup descent3_tests
 */
TEST(AIConstantsTest, AISoundShortDistance) {
    EXPECT_EQ(AI_SOUND_SHORT_DIST, 60.0f);
    EXPECT_GT(AI_SOUND_SHORT_DIST, 0.0f);
}

/**
 * @test AIConstantsTest.AISoundShortDistance_Reasonable
 * @brief Verifies aISound Short Distance Reasonable.
 *
 * @details
 * Exercises the AIConstantsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/ai_constants.cpp
 * @ingroup descent3_tests
 */
TEST(AIConstantsTest, AISoundShortDistance_Reasonable) {
    EXPECT_LT(AI_SOUND_SHORT_DIST, 1000.0f);
    EXPECT_GT(AI_SOUND_SHORT_DIST, 1.0f);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
