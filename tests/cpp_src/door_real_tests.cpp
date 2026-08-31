/**
 * @file door_real_tests.cpp
 * @brief Unit tests for Descent3/door.cpp: the global door table allocator, circular.
 *
 * @details
 * traversal, name lookup, and the LoadDoorImage/GetDoorImage model-handle path.
 *
 * This harness validates the behavior of `Descent3/door.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/door.cpp`
 * @par Harness
 * `door_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/door.cpp
 */

#include <gtest/gtest.h>
#include <cstring>
#include <filesystem>

#include "door.h"

// LoadPolyModel is declared in polymodel.h but only used here for LoadDoorImage.
int LoadPolyModel(const std::filesystem::path &, int);

// Stub: hand back a deterministic handle so we can observe it flow through.
static int s_next_model_handle = 100;
int LoadPolyModel(const std::filesystem::path &, int) { return s_next_model_handle++; }

/**
 * @brief GTest fixture for DoorTest.
 * @details
 * Sets up stubbed engine state and helper factories shared
 * across the suite. Each @test starts from the deterministic
 * baseline established in SetUp().
 * @ingroup descent3_tests
 */
class DoorTest : public ::testing::Test {
protected:
  void SetUp() override { InitDoors(); }
};

/**
 * @test DoorTest.InitResetsTableAndCount
 * @brief Verifies init Resets Table And Count.
 *
 * @details
 * Exercises the DoorTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/door.cpp
 * @ingroup descent3_tests
 */
TEST_F(DoorTest, InitResetsTableAndCount) {
  Doors[0].used = 1;
  Num_doors = 1;
  InitDoors();
  EXPECT_EQ(Num_doors, 0);
  for (int i = 0; i < MAX_DOORS; i++) {
    EXPECT_EQ(Doors[i].used, 0);
    EXPECT_EQ(Doors[i].model_handle, -1);
  }
}

/**
 * @test DoorTest.AllocReturnsSequentialIndices
 * @brief Verifies alloc Returns Sequential Indices.
 *
 * @details
 * Exercises the DoorTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/door.cpp
 * @ingroup descent3_tests
 */
TEST_F(DoorTest, AllocReturnsSequentialIndices) {
  int a = AllocDoor();
  int b = AllocDoor();
  EXPECT_EQ(a, 0);
  EXPECT_EQ(b, 1);
  EXPECT_EQ(Num_doors, 2);
  EXPECT_EQ(Doors[0].used, 1);
  EXPECT_EQ(Doors[0].flags, 0);
  EXPECT_EQ(Doors[0].hit_points, 0.0f);
}

/**
 * @test DoorTest.FreeMarksUnusedAndDecrements
 * @brief Verifies free Marks Unused And Decrements.
 *
 * @details
 * Exercises the DoorTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/door.cpp
 * @ingroup descent3_tests
 */
TEST_F(DoorTest, FreeMarksUnusedAndDecrements) {
  int a = AllocDoor();
  FreeDoor(a);
  EXPECT_EQ(Num_doors, 0);
  EXPECT_EQ(Doors[a].used, 0);
  EXPECT_EQ(Doors[a].name[0], 0);
}

/**
 * @test DoorTest.AllocReusesFreedSlot
 * @brief Verifies alloc Reuses Freed Slot.
 *
 * @details
 * Exercises the DoorTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/door.cpp
 * @ingroup descent3_tests
 */
TEST_F(DoorTest, AllocReusesFreedSlot) {
  int a = AllocDoor();
  int b = AllocDoor();
  FreeDoor(a);
  int c = AllocDoor();
  EXPECT_EQ(c, a);
  EXPECT_EQ(Num_doors, 2);
}

/**
 * @test DoorTest.NextAndPrevWrapAroundUsed
 * @brief Verifies next And Prev Wrap Around Used.
 *
 * @details
 * Exercises the DoorTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/door.cpp
 * @ingroup descent3_tests
 */
TEST_F(DoorTest, NextAndPrevWrapAroundUsed) {
  AllocDoor(); // 0
  AllocDoor(); // 1
  AllocDoor(); // 2
  FreeDoor(1); // only 0 and 2 used
  EXPECT_EQ(GetNextDoor(0), 2);
  EXPECT_EQ(GetNextDoor(2), 0);
  EXPECT_EQ(GetPrevDoor(2), 0);
  EXPECT_EQ(GetPrevDoor(0), 2);
}

/**
 * @test DoorTest.NextPrevReturnMinusOneWhenEmpty
 * @brief Verifies next Prev Return Minus One When Empty.
 *
 * @details
 * Exercises the DoorTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/door.cpp
 * @ingroup descent3_tests
 */
TEST_F(DoorTest, NextPrevReturnMinusOneWhenEmpty) {
  EXPECT_EQ(GetNextDoor(0), -1);
  EXPECT_EQ(GetPrevDoor(0), -1);
}

/**
 * @test DoorTest.FindByNameIsCaseInsensitive
 * @brief Verifies find By Name Is Case Insensitive.
 *
 * @details
 * Exercises the DoorTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/door.cpp
 * @ingroup descent3_tests
 */
TEST_F(DoorTest, FindByNameIsCaseInsensitive) {
  int a = AllocDoor();
  strcpy(Doors[a].name, "BlastDoor");
  EXPECT_EQ(FindDoorName("BlastDoor"), a);
  EXPECT_EQ(FindDoorName("blastdoor"), a);
  EXPECT_EQ(FindDoorName("nope"), -1);

  int b = AllocDoor();
  strcpy(Doors[b].name, "Hidden");
  FreeDoor(b);
  EXPECT_EQ(FindDoorName("Hidden"), -1);
}

/**
 * @test DoorTest.LoadDoorImageReturnsModelHandle
 * @brief Verifies load Door Image Returns Model Handle.
 *
 * @details
 * Exercises the DoorTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/door.cpp
 * @ingroup descent3_tests
 */
TEST_F(DoorTest, LoadDoorImageReturnsModelHandle) {
  s_next_model_handle = 100;
  int h = LoadDoorImage("door.pof", 0);
  EXPECT_EQ(h, 100); // first call to stub returns 100
}

/**
 * @test DoorTest.GetDoorImageReturnsModelHandle
 * @brief Verifies get Door Image Returns Model Handle.
 *
 * @details
 * Exercises the DoorTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/door.cpp
 * @ingroup descent3_tests
 */
TEST_F(DoorTest, GetDoorImageReturnsModelHandle) {
  int a = AllocDoor();
  Doors[a].model_handle = 77;
  EXPECT_EQ(GetDoorImage(a), 77);
}

/**
 * @test DoorTest.RemapDoorsToleratesValidModelHandles
 * @brief Verifies remap Doors Tolerates Valid Model Handles.
 *
 * @details
 * Exercises the DoorTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/door.cpp
 * @ingroup descent3_tests
 */
TEST_F(DoorTest, RemapDoorsToleratesValidModelHandles) {
  int a = AllocDoor();
  Doors[a].model_handle = 5; // valid (non -1) so the internal ASSERT holds
  RemapDoors();              // should not crash
  EXPECT_EQ(Doors[a].used, 1);
}

/**
 * @test DoorTest.AllocExhaustionReturnsMinusOne
 * @brief Verifies alloc Exhaustion Returns Minus One.
 *
 * @details
 * Exercises the DoorTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/door.cpp
 * @ingroup descent3_tests
 */
TEST_F(DoorTest, AllocExhaustionReturnsMinusOne) {
  for (int i = 0; i < MAX_DOORS; i++)
    ASSERT_NE(AllocDoor(), -1);
  EXPECT_EQ(Num_doors, MAX_DOORS);
  EXPECT_EQ(AllocDoor(), -1);
}
