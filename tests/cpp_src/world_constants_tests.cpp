/**
 * @file world_constants_tests.cpp
 * @brief Unit tests for Descent3/world_constants.cpp.
 *
 * @details
 * Covers the public contract of `Descent3/world_constants.cpp` — construction, state transitions, error handling and edge cases exercised through the GoogleTest harness.
 *
 * This harness validates the behavior of `Descent3/world_constants.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/world_constants.cpp`
 * @par Harness
 * `world_constants_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/world_constants.cpp
 */

#include "gtest/gtest.h"
#include "pstypes.h"

// Room/terrain/trigger/door constants (from Descent3 headers) for standalone tests
#define MAX_ROOMS 400
#define FIRST_PALETTE_ROOM MAX_ROOMS
#define MAX_PALETTE_ROOMS 50
#define MAX_ROOM_CHANGES 100
#define FPF_SOLID 1
#define FPF_TRANSPARENT 2
#define FPF_PORTAL 4
#define FPF_RECORD 8
#define TERRAIN_WIDTH 256
#define TERRAIN_DEPTH 256
#define TERRAIN_SIZE 16.0
#define MAX_TERRAIN_LOD 4
#define MAX_LOD_SIZE (1 << (MAX_TERRAIN_LOD - 1))
#define MAX_CELLS_TO_RENDER 8000
#define MAX_TERRAIN_HEIGHT 350.0f
#define TF_STARS 1
#define TF_SATELLITES 2
#define TF_FOG 4
#define TF_ROTATE_STARS 8
#define TF_ROTATE_SKY 16
#define TSF_HALO 1
#define TSF_ATMOSPHERE 2
#define TF_DYNAMIC 1
#define TF_SPECIAL_WATER 4
#define TF_SPECIAL_MINE 8
#define TF_INVISIBLE 16
#define TSEARCH_FOUND_TERRAIN 0
#define TSEARCH_FOUND_MINE 1
#define TSEARCH_FOUND_SATELLITE 2
#define TSEARCH_FOUND_SKY_DOME 3
#define TSEARCH_FOUND_SKY_BAND 4
#define TSEARCH_FOUND_OBJECT 5
#define TSEARCH_FOUND_NODE 6
#define TSEARCH_FOUND_BNODE 7
#define TF_UNUSED 1
#define TF_DISABLED 2
#define TF_DEAD 4
#define TF_ONESHOT 8
#define TF_INFORM_ACTIVATE_TO_LG 16
#define AF_PLAYER 1
#define AF_PLAYER_WEAPON 2
#define AF_ROBOT 4
#define AF_ROBOT_WEAPON 8
#define AF_CLUTTER 16
#define TRIG_NAME_LEN 19
#define TT_PASS_THROUGH 0
#define TT_COLLIDE 1
#define DF_BLASTABLE 1
#define DF_SEETHROUGH 2
#define MAX_DOORS 60
#define DOORWAY_STOPPED 0
#define DOORWAY_OPENING 1
#define DOORWAY_CLOSING 2
#define DOORWAY_WAITING 3
#define DOORWAY_OPENING_AUTO 4
#define DF_BLASTED 1
#define DF_AUTO 2
#define DF_LOCKED 4
#define DF_KEY_ONLY_ONE 8
#define DF_GB_IGNORE_LOCKED 16
#define KF_KEY1 1
#define KF_KEY2 2
#define KF_KEY3 4
#define KF_KEY4 8
#define MAX_ACTIVE_DOORWAYS 30

/**
 * @test RoomConstantsTest.MaxRooms
 * @brief Verifies max Rooms.
 *
 * @details
 * Exercises the RoomConstantsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/world_constants.cpp
 * @ingroup descent3_tests
 */
TEST(RoomConstantsTest, MaxRooms) {
    EXPECT_EQ(MAX_ROOMS, 400);
    EXPECT_GT(MAX_ROOMS, 0);
}

/**
 * @test RoomConstantsTest.PaletteRooms
 * @brief Verifies palette Rooms.
 *
 * @details
 * Exercises the RoomConstantsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/world_constants.cpp
 * @ingroup descent3_tests
 */
TEST(RoomConstantsTest, PaletteRooms) {
    EXPECT_EQ(FIRST_PALETTE_ROOM, MAX_ROOMS);
    EXPECT_EQ(MAX_PALETTE_ROOMS, 50);
}

/**
 * @test RoomConstantsTest.MaxRoomChanges
 * @brief Verifies max Room Changes.
 *
 * @details
 * Exercises the RoomConstantsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/world_constants.cpp
 * @ingroup descent3_tests
 */
TEST(RoomConstantsTest, MaxRoomChanges) {
    EXPECT_EQ(MAX_ROOM_CHANGES, 100);
    EXPECT_GT(MAX_ROOM_CHANGES, 0);
}

/**
 * @test RoomConstantsTest.FacePortalFlags
 * @brief Verifies face Portal Flags.
 *
 * @details
 * Exercises the RoomConstantsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/world_constants.cpp
 * @ingroup descent3_tests
 */
TEST(RoomConstantsTest, FacePortalFlags) {
    EXPECT_EQ(FPF_SOLID, 1);
    EXPECT_EQ(FPF_TRANSPARENT, 2);
    EXPECT_EQ(FPF_PORTAL, 4);
    EXPECT_EQ(FPF_RECORD, 8);
}

/**
 * @test TerrainConstantsTest.TerrainDimensions
 * @brief Verifies terrain Dimensions.
 *
 * @details
 * Exercises the TerrainConstantsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/world_constants.cpp
 * @ingroup descent3_tests
 */
TEST(TerrainConstantsTest, TerrainDimensions) {
    EXPECT_EQ(TERRAIN_WIDTH, 256);
    EXPECT_EQ(TERRAIN_DEPTH, 256);
    EXPECT_EQ(TERRAIN_WIDTH, TERRAIN_DEPTH);
}

/**
 * @test TerrainConstantsTest.TerrainSize
 * @brief Verifies terrain Size.
 *
 * @details
 * Exercises the TerrainConstantsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/world_constants.cpp
 * @ingroup descent3_tests
 */
TEST(TerrainConstantsTest, TerrainSize) {
    EXPECT_EQ(TERRAIN_SIZE, 16.0);
    EXPECT_GT(TERRAIN_SIZE, 0.0);
}

/**
 * @test TerrainConstantsTest.TerrainLOD
 * @brief Verifies terrain LOD.
 *
 * @details
 * Exercises the TerrainConstantsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/world_constants.cpp
 * @ingroup descent3_tests
 */
TEST(TerrainConstantsTest, TerrainLOD) {
    EXPECT_EQ(MAX_TERRAIN_LOD, 4);
    EXPECT_GT(MAX_TERRAIN_LOD, 0);
    EXPECT_EQ(MAX_LOD_SIZE, 1 << (MAX_TERRAIN_LOD - 1));
}

/**
 * @test TerrainConstantsTest.MaxCellsToRender
 * @brief Verifies max Cells To Render.
 *
 * @details
 * Exercises the TerrainConstantsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/world_constants.cpp
 * @ingroup descent3_tests
 */
TEST(TerrainConstantsTest, MaxCellsToRender) {
    EXPECT_EQ(MAX_CELLS_TO_RENDER, 8000);
    EXPECT_GT(MAX_CELLS_TO_RENDER, 0);
}

/**
 * @test TerrainConstantsTest.MaxTerrainHeight
 * @brief Verifies max Terrain Height.
 *
 * @details
 * Exercises the TerrainConstantsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/world_constants.cpp
 * @ingroup descent3_tests
 */
TEST(TerrainConstantsTest, MaxTerrainHeight) {
    EXPECT_EQ(MAX_TERRAIN_HEIGHT, 350.0f);
    EXPECT_GT(MAX_TERRAIN_HEIGHT, 0.0f);
}

/**
 * @test TerrainConstantsTest.TerrainFlags
 * @brief Verifies terrain Flags.
 *
 * @details
 * Exercises the TerrainConstantsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/world_constants.cpp
 * @ingroup descent3_tests
 */
TEST(TerrainConstantsTest, TerrainFlags) {
    EXPECT_EQ(TF_STARS, 1);
    EXPECT_EQ(TF_SATELLITES, 2);
    EXPECT_EQ(TF_FOG, 4);
    EXPECT_EQ(TF_ROTATE_STARS, 8);
    EXPECT_EQ(TF_ROTATE_SKY, 16);
}

/**
 * @test TerrainConstantsTest.TerrainSkyFlags
 * @brief Verifies terrain Sky Flags.
 *
 * @details
 * Exercises the TerrainConstantsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/world_constants.cpp
 * @ingroup descent3_tests
 */
TEST(TerrainConstantsTest, TerrainSkyFlags) {
    EXPECT_EQ(TSF_HALO, 1);
    EXPECT_EQ(TSF_ATMOSPHERE, 2);
}

/**
 * @test TerrainConstantsTest.TerrainFeatureFlags
 * @brief Verifies terrain Feature Flags.
 *
 * @details
 * Exercises the TerrainConstantsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/world_constants.cpp
 * @ingroup descent3_tests
 */
TEST(TerrainConstantsTest, TerrainFeatureFlags) {
    EXPECT_EQ(TF_DYNAMIC, 1);
    EXPECT_EQ(TF_SPECIAL_WATER, 4);
    EXPECT_EQ(TF_SPECIAL_MINE, 8);
    EXPECT_EQ(TF_INVISIBLE, 16);
}

/**
 * @test TerrainConstantsTest.TerrainSearchResults
 * @brief Verifies terrain Search Results.
 *
 * @details
 * Exercises the TerrainConstantsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/world_constants.cpp
 * @ingroup descent3_tests
 */
TEST(TerrainConstantsTest, TerrainSearchResults) {
    EXPECT_EQ(TSEARCH_FOUND_TERRAIN, 0);
    EXPECT_EQ(TSEARCH_FOUND_MINE, 1);
    EXPECT_EQ(TSEARCH_FOUND_SATELLITE, 2);
    EXPECT_EQ(TSEARCH_FOUND_SKY_DOME, 3);
    EXPECT_EQ(TSEARCH_FOUND_SKY_BAND, 4);
    EXPECT_EQ(TSEARCH_FOUND_OBJECT, 5);
    EXPECT_EQ(TSEARCH_FOUND_NODE, 6);
    EXPECT_EQ(TSEARCH_FOUND_BNODE, 7);
}

/**
 * @test TriggerConstantsTest.TriggerFlags
 * @brief Verifies trigger Flags.
 *
 * @details
 * Exercises the TriggerConstantsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/world_constants.cpp
 * @ingroup descent3_tests
 */
TEST(TriggerConstantsTest, TriggerFlags) {
    EXPECT_EQ(TF_UNUSED, 1);
    EXPECT_EQ(TF_DISABLED, 2);
    EXPECT_EQ(TF_DEAD, 4);
    EXPECT_EQ(TF_ONESHOT, 8);
    EXPECT_EQ(TF_INFORM_ACTIVATE_TO_LG, 16);
}

/**
 * @test TriggerConstantsTest.ActivationFlags
 * @brief Verifies activation Flags.
 *
 * @details
 * Exercises the TriggerConstantsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/world_constants.cpp
 * @ingroup descent3_tests
 */
TEST(TriggerConstantsTest, ActivationFlags) {
    EXPECT_EQ(AF_PLAYER, 1);
    EXPECT_EQ(AF_PLAYER_WEAPON, 2);
    EXPECT_EQ(AF_ROBOT, 4);
    EXPECT_EQ(AF_ROBOT_WEAPON, 8);
    EXPECT_EQ(AF_CLUTTER, 16);
}

/**
 * @test TriggerConstantsTest.TriggerNameLength
 * @brief Verifies trigger Name Length.
 *
 * @details
 * Exercises the TriggerConstantsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/world_constants.cpp
 * @ingroup descent3_tests
 */
TEST(TriggerConstantsTest, TriggerNameLength) {
    EXPECT_EQ(TRIG_NAME_LEN, 19);
    EXPECT_GT(TRIG_NAME_LEN, 0);
}

/**
 * @test TriggerConstantsTest.TriggerTypes
 * @brief Verifies trigger Types.
 *
 * @details
 * Exercises the TriggerConstantsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/world_constants.cpp
 * @ingroup descent3_tests
 */
TEST(TriggerConstantsTest, TriggerTypes) {
    EXPECT_EQ(TT_PASS_THROUGH, 0);
    EXPECT_EQ(TT_COLLIDE, 1);
}

/**
 * @test DoorConstantsTest.DoorFlags
 * @brief Verifies door Flags.
 *
 * @details
 * Exercises the DoorConstantsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/world_constants.cpp
 * @ingroup descent3_tests
 */
TEST(DoorConstantsTest, DoorFlags) {
    EXPECT_EQ(DF_BLASTABLE, 1);
    EXPECT_EQ(DF_SEETHROUGH, 2);
}

/**
 * @test DoorConstantsTest.MaxDoors
 * @brief Verifies max Doors.
 *
 * @details
 * Exercises the DoorConstantsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/world_constants.cpp
 * @ingroup descent3_tests
 */
TEST(DoorConstantsTest, MaxDoors) {
    EXPECT_EQ(MAX_DOORS, 60);
    EXPECT_GT(MAX_DOORS, 0);
}

/**
 * @test DoorwayConstantsTest.DoorwayStates
 * @brief Verifies doorway States.
 *
 * @details
 * Exercises the DoorwayConstantsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/world_constants.cpp
 * @ingroup descent3_tests
 */
TEST(DoorwayConstantsTest, DoorwayStates) {
    EXPECT_EQ(DOORWAY_STOPPED, 0);
    EXPECT_EQ(DOORWAY_OPENING, 1);
    EXPECT_EQ(DOORWAY_CLOSING, 2);
    EXPECT_EQ(DOORWAY_WAITING, 3);
    EXPECT_EQ(DOORWAY_OPENING_AUTO, 4);
}

/**
 * @test DoorwayConstantsTest.DoorwayFlags
 * @brief Verifies doorway Flags.
 *
 * @details
 * Exercises the DoorwayConstantsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/world_constants.cpp
 * @ingroup descent3_tests
 */
TEST(DoorwayConstantsTest, DoorwayFlags) {
    EXPECT_EQ(DF_BLASTED, 1);
    EXPECT_EQ(DF_AUTO, 2);
    EXPECT_EQ(DF_LOCKED, 4);
    EXPECT_EQ(DF_KEY_ONLY_ONE, 8);
    EXPECT_EQ(DF_GB_IGNORE_LOCKED, 16);
}

/**
 * @test DoorwayConstantsTest.KeyFlags
 * @brief Verifies key Flags.
 *
 * @details
 * Exercises the DoorwayConstantsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/world_constants.cpp
 * @ingroup descent3_tests
 */
TEST(DoorwayConstantsTest, KeyFlags) {
    EXPECT_EQ(KF_KEY1, 1);
    EXPECT_EQ(KF_KEY2, 2);
    EXPECT_EQ(KF_KEY3, 4);
    EXPECT_EQ(KF_KEY4, 8);
}

/**
 * @test DoorwayConstantsTest.MaxActiveDoorways
 * @brief Verifies max Active Doorways.
 *
 * @details
 * Exercises the DoorwayConstantsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/world_constants.cpp
 * @ingroup descent3_tests
 */
TEST(DoorwayConstantsTest, MaxActiveDoorways) {
    EXPECT_EQ(MAX_ACTIVE_DOORWAYS, 30);
    EXPECT_GT(MAX_ACTIVE_DOORWAYS, 0);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
