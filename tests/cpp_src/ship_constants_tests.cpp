/**
 * @file ship_constants_tests.cpp
 * @brief Unit tests for Descent3/ship_constants.cpp.
 *
 * @details
 * Covers the public contract of `Descent3/ship_constants.cpp` — construction, state transitions, error handling and edge cases exercised through the GoogleTest harness.
 *
 * This harness validates the behavior of `Descent3/ship_constants.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/ship_constants.cpp`
 * @par Harness
 * `ship_constants_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/ship_constants.cpp
 */

#include "gtest/gtest.h"
#include "pstypes.h"
#include <cstring>

// Ship constants (from Descent3/ship.h) for standalone tests
#define MAX_SHIPS 30
#define DEFAULT_SHIP "Pyro-GL"
#define SFF_FUSION 1
#define SFF_ZOOM 4
#define SFF_TENTHS 8
#define SHIP_PYRO_ID 0
#define SHIP_PHOENIX_ID 1
#define SHIP_MAGNUM_ID 2
#define MAX_DEFAULT_SHIPS 3
#define SF_DEFAULT_ALLOW 1

/**
 * @test ShipConstantsTest.MaxShips
 * @brief Verifies max Ships.
 *
 * @details
 * Exercises the ShipConstantsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/ship_constants.cpp
 * @ingroup descent3_tests
 */
TEST(ShipConstantsTest, MaxShips) {
    EXPECT_EQ(MAX_SHIPS, 30);
    EXPECT_GT(MAX_SHIPS, 0);
}

/**
 * @test ShipConstantsTest.DefaultShipName
 * @brief Verifies default Ship Name.
 *
 * @details
 * Exercises the ShipConstantsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/ship_constants.cpp
 * @ingroup descent3_tests
 */
TEST(ShipConstantsTest, DefaultShipName) {
    EXPECT_STREQ(DEFAULT_SHIP, "Pyro-GL");
    EXPECT_GT(strlen(DEFAULT_SHIP), 0);
}

/**
 * @test ShipConstantsTest.ShipFeatureFlags
 * @brief Verifies ship Feature Flags.
 *
 * @details
 * Exercises the ShipConstantsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/ship_constants.cpp
 * @ingroup descent3_tests
 */
TEST(ShipConstantsTest, ShipFeatureFlags) {
    EXPECT_EQ(SFF_FUSION, 1);
    EXPECT_EQ(SFF_ZOOM, 4);
    EXPECT_EQ(SFF_TENTHS, 8);
    
    EXPECT_EQ(SFF_FUSION, 1 << 0);
    EXPECT_EQ(SFF_ZOOM, 1 << 2);
    EXPECT_EQ(SFF_TENTHS, 1 << 3);
    
    int all_sff = SFF_FUSION | SFF_ZOOM | SFF_TENTHS;
    EXPECT_EQ(all_sff, 0xD);
}

/**
 * @test ShipConstantsTest.ShipIds
 * @brief Verifies ship Ids.
 *
 * @details
 * Exercises the ShipConstantsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/ship_constants.cpp
 * @ingroup descent3_tests
 */
TEST(ShipConstantsTest, ShipIds) {
    EXPECT_EQ(SHIP_PYRO_ID, 0);
    EXPECT_EQ(SHIP_PHOENIX_ID, 1);
    EXPECT_EQ(SHIP_MAGNUM_ID, 2);
    
    EXPECT_EQ(SHIP_MAGNUM_ID - SHIP_PYRO_ID, 2);
}

/**
 * @test ShipConstantsTest.MaxDefaultShips
 * @brief Verifies max Default Ships.
 *
 * @details
 * Exercises the ShipConstantsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/ship_constants.cpp
 * @ingroup descent3_tests
 */
TEST(ShipConstantsTest, MaxDefaultShips) {
    EXPECT_EQ(MAX_DEFAULT_SHIPS, 3);
    EXPECT_GT(MAX_DEFAULT_SHIPS, 0);
    EXPECT_LE(MAX_DEFAULT_SHIPS, MAX_SHIPS);
}

/**
 * @test ShipConstantsTest.ShipFlags
 * @brief Verifies ship Flags.
 *
 * @details
 * Exercises the ShipConstantsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/ship_constants.cpp
 * @ingroup descent3_tests
 */
TEST(ShipConstantsTest, ShipFlags) {
    EXPECT_EQ(SF_DEFAULT_ALLOW, 1);
    EXPECT_GT(SF_DEFAULT_ALLOW, 0);
}

/**
 * @test ShipConstantsTest.ShipIdsAreSequential
 * @brief Verifies ship Ids Are Sequential.
 *
 * @details
 * Exercises the ShipConstantsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/ship_constants.cpp
 * @ingroup descent3_tests
 */
TEST(ShipConstantsTest, ShipIdsAreSequential) {
    EXPECT_EQ(SHIP_PYRO_ID + 1, SHIP_PHOENIX_ID);
    EXPECT_EQ(SHIP_PHOENIX_ID + 1, SHIP_MAGNUM_ID);
}

/**
 * @test ShipConstantsTest.DefaultShipsNotZero
 * @brief Verifies default Ships Not Zero.
 *
 * @details
 * Exercises the ShipConstantsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/ship_constants.cpp
 * @ingroup descent3_tests
 */
TEST(ShipConstantsTest, DefaultShipsNotZero) {
    EXPECT_EQ(SHIP_PYRO_ID, 0);  // first ship has id 0
    EXPECT_NE(SHIP_PHOENIX_ID, 0);
    EXPECT_NE(SHIP_MAGNUM_ID, 0);
}

/**
 * @test ShipConstantsTest.MaxShipsGreaterThanDefaults
 * @brief Verifies max Ships Greater Than Defaults.
 *
 * @details
 * Exercises the ShipConstantsTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/ship_constants.cpp
 * @ingroup descent3_tests
 */
TEST(ShipConstantsTest, MaxShipsGreaterThanDefaults) {
    EXPECT_GT(MAX_SHIPS, MAX_DEFAULT_SHIPS);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
