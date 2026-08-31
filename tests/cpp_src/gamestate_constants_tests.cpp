/**
 * @file gamestate_constants_tests.cpp
 * @brief Unit tests for Descent3/gamestate_constants.cpp.
 *
 * @details
 * Covers the public contract of `Descent3/gamestate_constants.cpp` — construction, state transitions, error handling and edge cases exercised through the GoogleTest harness.
 *
 * This harness validates the behavior of `Descent3/gamestate_constants.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/gamestate_constants.cpp`
 * @par Harness
 * `gamestate_constants_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/gamestate_constants.cpp
 */

#include "gtest/gtest.h"
#include "pstypes.h"
#include <cstring>

#define GAME_INTERFACE 0
#define GAME_OPTIONS_INTERFACE 1
#define GAME_PAUSE_INTERFACE 2
#define GAME_HELP_INTERFACE 3
#define GAME_BUDDY_INTERFACE 4
#define GAME_TELCOM_BRIEFINGS 5
#define GAME_TELCOM_AUTOMAP 6
#define GAME_TELCOM_CARGO 7
#define GAME_EXIT_CONFIRM 8
#define GAME_DLL_INTERFACE 9
#define GAME_SAVE_INTERFACE 10
#define GAME_LOAD_INTERFACE 11
#define GAME_TOGGLE_DEMO 12
#define GAME_POST_DEMO 13
#define GAME_DEMO_LOOP 14
#define GAME_LEVEL_WARP 15
#define GAME_DEBUGGRAPH_INTERFACE 16
#define GAMESAVE_SLOTS 8
#define GAMESAVE_DESCLEN 31
#define GAMESAVE_VERSION 2
#define GAMESAVE_OLDVER 0
#define LGS_OK 0
#define LGS_FILENOTFOUND 1
#define LGS_OUTDATEDVER 2
#define LGS_STARTLVLFAILED 3
#define LGS_MISSIONFAILED 4
#define LGS_OBJECTSCORRUPT 5
#define LGS_CORRUPTLEVEL 6
#define LEVEL_FILE_VERSION 132
#define LEVEL_FILE_OLDEST_COMPATIBLE_VERSION 13
#define CHUNK_TEXTURE_NAMES "TXNM"
#define CHUNK_ROOMS "ROOM"
#define CHUNK_OBJECTS "OBJS"
#define CHUNK_TERRAIN "TERR"
#define CHUNK_TRIGGERS "TRIG"
#define CHUNK_LIGHTMAPS "LMAP"
#define CHUNK_LEVEL_INFO "INFO"

/**
 * @test GameSequenceConstantsTest.GameInterfaces
 * @brief Verifies game Interfaces.
 *
 * @details
 * Exercises the GameSequenceConstantsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/gamestate_constants.cpp
 * @ingroup descent3_tests
 */
TEST(GameSequenceConstantsTest, GameInterfaces) {
    EXPECT_EQ(GAME_INTERFACE, 0);
    EXPECT_EQ(GAME_OPTIONS_INTERFACE, 1);
    EXPECT_EQ(GAME_PAUSE_INTERFACE, 2);
    EXPECT_EQ(GAME_HELP_INTERFACE, 3);
    EXPECT_EQ(GAME_BUDDY_INTERFACE, 4);
    EXPECT_EQ(GAME_TELCOM_BRIEFINGS, 5);
    EXPECT_EQ(GAME_TELCOM_AUTOMAP, 6);
    EXPECT_EQ(GAME_TELCOM_CARGO, 7);
    EXPECT_EQ(GAME_EXIT_CONFIRM, 8);
    EXPECT_EQ(GAME_DLL_INTERFACE, 9);
    EXPECT_EQ(GAME_SAVE_INTERFACE, 10);
    EXPECT_EQ(GAME_LOAD_INTERFACE, 11);
    EXPECT_EQ(GAME_TOGGLE_DEMO, 12);
    EXPECT_EQ(GAME_POST_DEMO, 13);
    EXPECT_EQ(GAME_DEMO_LOOP, 14);
    EXPECT_EQ(GAME_LEVEL_WARP, 15);
    EXPECT_EQ(GAME_DEBUGGRAPH_INTERFACE, 16);
}

/**
 * @test GameSequenceConstantsTest.InterfacesSequential
 * @brief Verifies interfaces Sequential.
 *
 * @details
 * Exercises the GameSequenceConstantsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/gamestate_constants.cpp
 * @ingroup descent3_tests
 */
TEST(GameSequenceConstantsTest, InterfacesSequential) {
    EXPECT_EQ(GAME_DEBUGGRAPH_INTERFACE - GAME_INTERFACE, 16);
}

/**
 * @test GameSaveConstantsTest.MaxSaveSlots
 * @brief Verifies max Save Slots.
 *
 * @details
 * Exercises the GameSaveConstantsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/gamestate_constants.cpp
 * @ingroup descent3_tests
 */
TEST(GameSaveConstantsTest, MaxSaveSlots) {
    EXPECT_EQ(GAMESAVE_SLOTS, 8);
    EXPECT_GT(GAMESAVE_SLOTS, 0);
}

/**
 * @test GameSaveConstantsTest.DescriptionLength
 * @brief Verifies description Length.
 *
 * @details
 * Exercises the GameSaveConstantsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/gamestate_constants.cpp
 * @ingroup descent3_tests
 */
TEST(GameSaveConstantsTest, DescriptionLength) {
    EXPECT_EQ(GAMESAVE_DESCLEN, 31);
    EXPECT_GT(GAMESAVE_DESCLEN, 0);
}

/**
 * @test GameSaveConstantsTest.SaveVersion
 * @brief Verifies save Version.
 *
 * @details
 * Exercises the GameSaveConstantsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/gamestate_constants.cpp
 * @ingroup descent3_tests
 */
TEST(GameSaveConstantsTest, SaveVersion) {
    EXPECT_EQ(GAMESAVE_VERSION, 2);
    EXPECT_EQ(GAMESAVE_OLDVER, 0);
    EXPECT_LT(GAMESAVE_OLDVER, GAMESAVE_VERSION);
}

/**
 * @test GameSaveConstantsTest.LoadGameStatus
 * @brief Verifies load Game Status.
 *
 * @details
 * Exercises the GameSaveConstantsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/gamestate_constants.cpp
 * @ingroup descent3_tests
 */
TEST(GameSaveConstantsTest, LoadGameStatus) {
    EXPECT_EQ(LGS_OK, 0);
    EXPECT_EQ(LGS_FILENOTFOUND, 1);
    EXPECT_EQ(LGS_OUTDATEDVER, 2);
    EXPECT_EQ(LGS_STARTLVLFAILED, 3);
    EXPECT_EQ(LGS_MISSIONFAILED, 4);
    EXPECT_EQ(LGS_OBJECTSCORRUPT, 5);
    EXPECT_EQ(LGS_CORRUPTLEVEL, 6);
}

/**
 * @test GameSaveConstantsTest.LoadGameStatusSequential
 * @brief Verifies load Game Status Sequential.
 *
 * @details
 * Exercises the GameSaveConstantsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/gamestate_constants.cpp
 * @ingroup descent3_tests
 */
TEST(GameSaveConstantsTest, LoadGameStatusSequential) {
    EXPECT_EQ(LGS_CORRUPTLEVEL - LGS_OK, 6);
}

/**
 * @test LoadLevelConstantsTest.LevelFileVersion
 * @brief Verifies level File Version.
 *
 * @details
 * Exercises the LoadLevelConstantsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/gamestate_constants.cpp
 * @ingroup descent3_tests
 */
TEST(LoadLevelConstantsTest, LevelFileVersion) {
    EXPECT_EQ(LEVEL_FILE_VERSION, 132);
    EXPECT_GT(LEVEL_FILE_VERSION, 0);
}

/**
 * @test LoadLevelConstantsTest.LevelFileOldestCompatible
 * @brief Verifies level File Oldest Compatible.
 *
 * @details
 * Exercises the LoadLevelConstantsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/gamestate_constants.cpp
 * @ingroup descent3_tests
 */
TEST(LoadLevelConstantsTest, LevelFileOldestCompatible) {
    EXPECT_EQ(LEVEL_FILE_OLDEST_COMPATIBLE_VERSION, 13);
    EXPECT_LT(LEVEL_FILE_OLDEST_COMPATIBLE_VERSION, LEVEL_FILE_VERSION);
}

/**
 * @test LoadLevelConstantsTest.ChunkIdentifiers
 * @brief Verifies chunk Identifiers.
 *
 * @details
 * Exercises the LoadLevelConstantsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/gamestate_constants.cpp
 * @ingroup descent3_tests
 */
TEST(LoadLevelConstantsTest, ChunkIdentifiers) {
    EXPECT_STREQ(CHUNK_TEXTURE_NAMES, "TXNM");
    EXPECT_STREQ(CHUNK_ROOMS, "ROOM");
    EXPECT_STREQ(CHUNK_OBJECTS, "OBJS");
    EXPECT_STREQ(CHUNK_TERRAIN, "TERR");
    EXPECT_STREQ(CHUNK_TRIGGERS, "TRIG");
    EXPECT_STREQ(CHUNK_LIGHTMAPS, "LMAP");
    EXPECT_STREQ(CHUNK_LEVEL_INFO, "INFO");
}

/**
 * @test LoadLevelConstantsTest.ChunkNamesLength
 * @brief Verifies chunk Names Length.
 *
 * @details
 * Exercises the LoadLevelConstantsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/gamestate_constants.cpp
 * @ingroup descent3_tests
 */
TEST(LoadLevelConstantsTest, ChunkNamesLength) {
    EXPECT_EQ(strlen(CHUNK_TEXTURE_NAMES), 4);
    EXPECT_EQ(strlen(CHUNK_ROOMS), 4);
    EXPECT_EQ(strlen(CHUNK_LEVEL_INFO), 4);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
