/**
 * @file ship_real_tests.cpp
 * @brief Unit tests for Descent3/ship.cpp: the global ship table allocator, circular.
 *
 * @details
 * traversal, name lookup, model-handle access, and ship-index remapping.
 *
 * This harness validates the behavior of `Descent3/ship.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/ship.cpp`
 * @par Harness
 * `ship_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/ship.cpp
 */

#include <gtest/gtest.h>
#include <cstring>
#include <filesystem>

#include "ship.h"
#include "object.h"
#include "player.h"

// --- stubs for the few external symbols ship.cpp needs ---
int LoadPolyModel(const std::filesystem::path &, int);
struct otype_wb_info;
void WBClearInfo(otype_wb_info *);

static int s_next_model_handle = 200;
int LoadPolyModel(const std::filesystem::path &, int) { return s_next_model_handle++; }
void WBClearInfo(otype_wb_info *) {}

// player contains an Inventory member; only its ctor/dtor are needed here.
Inventory::Inventory() {}
Inventory::~Inventory() {}

// Globals normally provided by object.cpp / player.cpp.
object Objects[MAX_OBJECTS];
player Players[MAX_PLAYERS];

/**
 * @brief GTest fixture for ShipTest.
 * @details
 * Sets up stubbed engine state and helper factories shared
 * across the suite. Each @test starts from the deterministic
 * baseline established in SetUp().
 * @ingroup descent3_tests
 */
class ShipTest : public ::testing::Test {
protected:
  void SetUp() override {
    memset(Objects, 0, sizeof(Objects));
    memset(Players, 0, sizeof(Players));
    InitShips();
  }
};

/**
 * @test ShipTest.InitResetsTableAndCount
 * @brief Verifies init Resets Table And Count.
 *
 * @details
 * Exercises the ShipTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/ship.cpp
 * @ingroup descent3_tests
 */
TEST_F(ShipTest, InitResetsTableAndCount) {
  Ships[0].used = 1;
  Num_ships = 1;
  InitShips();
  EXPECT_EQ(Num_ships, 0);
  for (int i = 0; i < MAX_SHIPS; i++) {
    EXPECT_EQ(Ships[i].used, 0);
    EXPECT_EQ(Ships[i].model_handle, 0); // memset to 0
  }
}

/**
 * @test ShipTest.AllocInitializesDefaults
 * @brief Verifies alloc Initializes Defaults.
 *
 * @details
 * Exercises the ShipTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/ship.cpp
 * @ingroup descent3_tests
 */
TEST_F(ShipTest, AllocInitializesDefaults) {
  int a = AllocShip();
  EXPECT_EQ(a, 0);
  EXPECT_EQ(Num_ships, 1);
  EXPECT_EQ(Ships[0].used, 1);
  EXPECT_EQ(Ships[0].size, 4.0f); // DEFAULT_SHIP_SIZE
  EXPECT_EQ(Ships[0].model_handle, -1);
  EXPECT_EQ(Ships[0].dying_model_handle, -1);
  EXPECT_EQ(Ships[0].armor_scalar, 1.0f);
  EXPECT_EQ(Ships[0].flags, 0);
}

/**
 * @test ShipTest.FreeMarksUnusedAndDecrements
 * @brief Verifies free Marks Unused And Decrements.
 *
 * @details
 * Exercises the ShipTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/ship.cpp
 * @ingroup descent3_tests
 */
TEST_F(ShipTest, FreeMarksUnusedAndDecrements) {
  int a = AllocShip();
  FreeShip(a);
  EXPECT_EQ(Num_ships, 0);
  EXPECT_EQ(Ships[a].used, 0);
  EXPECT_EQ(Ships[a].name[0], 0);
}

/**
 * @test ShipTest.AllocReusesFreedSlot
 * @brief Verifies alloc Reuses Freed Slot.
 *
 * @details
 * Exercises the ShipTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/ship.cpp
 * @ingroup descent3_tests
 */
TEST_F(ShipTest, AllocReusesFreedSlot) {
  int a = AllocShip();
  int b = AllocShip();
  FreeShip(a);
  int c = AllocShip();
  EXPECT_EQ(c, a);
  EXPECT_EQ(Num_ships, 2);
}

/**
 * @test ShipTest.NextAndPrevWrapAroundUsed
 * @brief Verifies next And Prev Wrap Around Used.
 *
 * @details
 * Exercises the ShipTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/ship.cpp
 * @ingroup descent3_tests
 */
TEST_F(ShipTest, NextAndPrevWrapAroundUsed) {
  AllocShip(); // 0
  AllocShip(); // 1
  AllocShip(); // 2
  FreeShip(1);
  EXPECT_EQ(GetNextShip(0), 2);
  EXPECT_EQ(GetNextShip(2), 0);
  EXPECT_EQ(GetPrevShip(2), 0);
  EXPECT_EQ(GetPrevShip(0), 2);
}

/**
 * @test ShipTest.NextPrevReturnMinusOneWhenEmpty
 * @brief Verifies next Prev Return Minus One When Empty.
 *
 * @details
 * Exercises the ShipTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/ship.cpp
 * @ingroup descent3_tests
 */
TEST_F(ShipTest, NextPrevReturnMinusOneWhenEmpty) {
  EXPECT_EQ(GetNextShip(0), -1);
  EXPECT_EQ(GetPrevShip(0), -1);
}

/**
 * @test ShipTest.FindByNameIsCaseInsensitive
 * @brief Verifies find By Name Is Case Insensitive.
 *
 * @details
 * Exercises the ShipTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/ship.cpp
 * @ingroup descent3_tests
 */
TEST_F(ShipTest, FindByNameIsCaseInsensitive) {
  int a = AllocShip();
  strcpy(Ships[a].name, "Pyro-GL");
  EXPECT_EQ(FindShipName("Pyro-GL"), a);
  EXPECT_EQ(FindShipName("pyro-gl"), a);
  EXPECT_EQ(FindShipName("nope"), -1);

  int b = AllocShip();
  strcpy(Ships[b].name, "Hidden");
  FreeShip(b);
  EXPECT_EQ(FindShipName("Hidden"), -1);
}

/**
 * @test ShipTest.LoadShipImageReturnsModelHandle
 * @brief Verifies load Ship Image Returns Model Handle.
 *
 * @details
 * Exercises the ShipTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/ship.cpp
 * @ingroup descent3_tests
 */
TEST_F(ShipTest, LoadShipImageReturnsModelHandle) {
  s_next_model_handle = 200;
  int h = LoadShipImage("ship.pof");
  EXPECT_EQ(h, 200);
}

/**
 * @test ShipTest.GetShipImageReturnsModelHandle
 * @brief Verifies get Ship Image Returns Model Handle.
 *
 * @details
 * Exercises the ShipTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/ship.cpp
 * @ingroup descent3_tests
 */
TEST_F(ShipTest, GetShipImageReturnsModelHandle) {
  int a = AllocShip();
  Ships[a].model_handle = 99;
  EXPECT_EQ(GetShipImage(a), 99);
}

/**
 * @test ShipTest.RemapAllShipObjectsUpdatesPlayerShipIndex
 * @brief Verifies remap All Ship Objects Updates Player Ship Index.
 *
 * @details
 * Exercises the ShipTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/ship.cpp
 * @ingroup descent3_tests
 */
TEST_F(ShipTest, RemapAllShipObjectsUpdatesPlayerShipIndex) {
  // object 5 is a player whose ship_index points at slot 2
  Objects[5].type = OBJ_PLAYER;
  Objects[5].id = 0;
  Players[0].ship_index = 2;
  RemapAllShipObjects(2, 7);
  EXPECT_EQ(Players[0].ship_index, 7);

  // an object whose ship_index differs is untouched
  Objects[6].type = OBJ_PLAYER;
  Objects[6].id = 1;
  Players[1].ship_index = 3;
  RemapAllShipObjects(2, 7);
  EXPECT_EQ(Players[1].ship_index, 3);

  // non-player objects are ignored
  Objects[5].type = OBJ_NONE; // remove the earlier player so it can't match
  Objects[7].type = OBJ_ROBOT;
  Objects[7].id = 0;
  Players[0].ship_index = 7;
  RemapAllShipObjects(7, 9);
  EXPECT_EQ(Players[0].ship_index, 7); // unchanged (object 7 isn't a player)
}

/**
 * @test ShipTest.RemapShipsToleratesValidModelHandles
 * @brief Verifies remap Ships Tolerates Valid Model Handles.
 *
 * @details
 * Exercises the ShipTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/ship.cpp
 * @ingroup descent3_tests
 */
TEST_F(ShipTest, RemapShipsToleratesValidModelHandles) {
  int a = AllocShip();
  Ships[a].model_handle = 5; // valid so internal ASSERT holds
  RemapShips();              // NUM_STATIC_SHIPS == 0, so only the assert loop runs
  EXPECT_EQ(Ships[a].used, 1);
}

/**
 * @test ShipTest.AllocExhaustionReturnsMinusOne
 * @brief Verifies alloc Exhaustion Returns Minus One.
 *
 * @details
 * Exercises the ShipTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/ship.cpp
 * @ingroup descent3_tests
 */
TEST_F(ShipTest, AllocExhaustionReturnsMinusOne) {
  for (int i = 0; i < MAX_SHIPS; i++)
    ASSERT_NE(AllocShip(), -1);
  EXPECT_EQ(Num_ships, MAX_SHIPS);
  EXPECT_EQ(AllocShip(), -1);
}
