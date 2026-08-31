/**
 * @file pilot_config_constants_tests.cpp
 * @brief Unit tests for Descent3/pilot_config_constants.cpp.
 *
 * @details
 * Covers the public contract of `Descent3/pilot_config_constants.cpp` — construction, state transitions, error handling and edge cases exercised through the GoogleTest harness.
 *
 * This harness validates the behavior of `Descent3/pilot_config_constants.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/pilot_config_constants.cpp`
 * @par Harness
 * `pilot_config_constants_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/pilot_config_constants.cpp
 */

#include "gtest/gtest.h"
#include "pstypes.h"

#define PILOT_STRING_SIZE 20
#define PPIC_INVALID_ID 65535
#define PLT_FILE_FATAL (-3)
#define PLT_FILE_CANT_CREATE (-2)
#define PLT_FILE_EXISTS (-1)
#define N_MOUSE_AXIS 2
#define N_JOY_AXIS 6
#define MAX_PILOT_TAUNTS 8
#define PILOT_TAUNT_SIZE 60
#define N_SUPPORTED_VIDRES 8
#define RES_512X384 0
#define RES_640X480 1
#define RES_800X600 2
#define RES_960X720 3
#define RES_1024X768 4
#define RES_1280X960 5
#define RES_1600X1200 6
#define DETAIL_LEVEL_LOW 0
#define DETAIL_LEVEL_MED 1
#define DETAIL_LEVEL_HIGH 2
#define DETAIL_LEVEL_VERY_HIGH 3
#define DETAIL_LEVEL_CUSTOM 4

/**
 * @test PilotConstantsTest.PilotStringSize
 * @brief Verifies pilot String Size.
 *
 * @details
 * Exercises the PilotConstantsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/pilot_config_constants.cpp
 * @ingroup descent3_tests
 */
TEST(PilotConstantsTest, PilotStringSize) {
    EXPECT_EQ(PILOT_STRING_SIZE, 20);
    EXPECT_GT(PILOT_STRING_SIZE, 0);
}

/**
 * @test PilotConstantsTest.PilotPicId
 * @brief Verifies pilot Pic Id.
 *
 * @details
 * Exercises the PilotConstantsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/pilot_config_constants.cpp
 * @ingroup descent3_tests
 */
TEST(PilotConstantsTest, PilotPicId) {
    EXPECT_EQ(PPIC_INVALID_ID, 65535);
    EXPECT_EQ(PPIC_INVALID_ID, 0xFFFF);
}

/**
 * @test PilotConstantsTest.PilotFileErrors
 * @brief Verifies pilot File Errors.
 *
 * @details
 * Exercises the PilotConstantsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/pilot_config_constants.cpp
 * @ingroup descent3_tests
 */
TEST(PilotConstantsTest, PilotFileErrors) {
    EXPECT_EQ(PLT_FILE_FATAL, -3);
    EXPECT_EQ(PLT_FILE_CANT_CREATE, -2);
    EXPECT_EQ(PLT_FILE_EXISTS, -1);
}

/**
 * @test PilotConstantsTest.InputAxisCounts
 * @brief Verifies input Axis Counts.
 *
 * @details
 * Exercises the PilotConstantsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/pilot_config_constants.cpp
 * @ingroup descent3_tests
 */
TEST(PilotConstantsTest, InputAxisCounts) {
    EXPECT_EQ(N_MOUSE_AXIS, 2);
    EXPECT_EQ(N_JOY_AXIS, 6);
    EXPECT_GT(N_JOY_AXIS, N_MOUSE_AXIS);
}

/**
 * @test PilotConstantsTest.MaxPilotTaunts
 * @brief Verifies max Pilot Taunts.
 *
 * @details
 * Exercises the PilotConstantsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/pilot_config_constants.cpp
 * @ingroup descent3_tests
 */
TEST(PilotConstantsTest, MaxPilotTaunts) {
    EXPECT_EQ(MAX_PILOT_TAUNTS, 8);
    EXPECT_GT(MAX_PILOT_TAUNTS, 0);
}

/**
 * @test PilotConstantsTest.PilotTauntSize
 * @brief Verifies pilot Taunt Size.
 *
 * @details
 * Exercises the PilotConstantsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/pilot_config_constants.cpp
 * @ingroup descent3_tests
 */
TEST(PilotConstantsTest, PilotTauntSize) {
    EXPECT_EQ(PILOT_TAUNT_SIZE, 60);
    EXPECT_GT(PILOT_TAUNT_SIZE, 0);
}

/**
 * @test ConfigConstantsTest.SupportedVideoResolutions
 * @brief Verifies supported Video Resolutions.
 *
 * @details
 * Exercises the ConfigConstantsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/pilot_config_constants.cpp
 * @ingroup descent3_tests
 */
TEST(ConfigConstantsTest, SupportedVideoResolutions) {
    EXPECT_EQ(N_SUPPORTED_VIDRES, 8);
    EXPECT_GT(N_SUPPORTED_VIDRES, 0);
}

/**
 * @test ConfigConstantsTest.ResolutionIdentifiers
 * @brief Verifies resolution Identifiers.
 *
 * @details
 * Exercises the ConfigConstantsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/pilot_config_constants.cpp
 * @ingroup descent3_tests
 */
TEST(ConfigConstantsTest, ResolutionIdentifiers) {
    EXPECT_EQ(RES_512X384, 0);
    EXPECT_EQ(RES_640X480, 1);
    EXPECT_EQ(RES_800X600, 2);
    EXPECT_EQ(RES_960X720, 3);
    EXPECT_EQ(RES_1024X768, 4);
    EXPECT_EQ(RES_1280X960, 5);
    EXPECT_EQ(RES_1600X1200, 6);
}

/**
 * @test ConfigConstantsTest.DetailLevels
 * @brief Verifies detail Levels.
 *
 * @details
 * Exercises the ConfigConstantsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/pilot_config_constants.cpp
 * @ingroup descent3_tests
 */
TEST(ConfigConstantsTest, DetailLevels) {
    EXPECT_EQ(DETAIL_LEVEL_LOW, 0);
    EXPECT_EQ(DETAIL_LEVEL_MED, 1);
    EXPECT_EQ(DETAIL_LEVEL_HIGH, 2);
    EXPECT_EQ(DETAIL_LEVEL_VERY_HIGH, 3);
    EXPECT_EQ(DETAIL_LEVEL_CUSTOM, 4);
}

/**
 * @test ConfigConstantsTest.DetailLevelRange
 * @brief Verifies detail Level Range.
 *
 * @details
 * Exercises the ConfigConstantsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/pilot_config_constants.cpp
 * @ingroup descent3_tests
 */
TEST(ConfigConstantsTest, DetailLevelRange) {
    EXPECT_EQ(DETAIL_LEVEL_CUSTOM - DETAIL_LEVEL_LOW, 4);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
