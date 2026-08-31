/**
 * @file gamefile_real_tests.cpp
 * @brief Unit tests for Descent3/gamefile.cpp: the global gamefile table allocator.
 *
 * @details
 * and its circular next/prev traversal and name lookup.
 *
 * This harness validates the behavior of `Descent3/gamefile.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/gamefile.cpp`
 * @par Harness
 * `gamefile_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/gamefile.cpp
 */

#include <gtest/gtest.h>
#include <cstring>

#include "gamefile.h"

/**
 * @brief GTest fixture for GamefileTest.
 * @details
 * Sets up stubbed engine state and helper factories shared
 * across the suite. Each @test starts from the deterministic
 * baseline established in SetUp().
 * @ingroup descent3_tests
 */
class GamefileTest : public ::testing::Test {
protected:
  void SetUp() override { InitGamefiles(); }
};

/**
 * @test GamefileTest.InitClearsTableAndCount
 * @brief Verifies init Clears Table And Count.
 *
 * @details
 * Exercises the GamefileTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/gamefile.cpp
 * @ingroup descent3_tests
 */
TEST_F(GamefileTest, InitClearsTableAndCount) {
  // dirty it first
  AllocGamefile();
  AllocGamefile();
  InitGamefiles();
  EXPECT_EQ(Num_gamefiles, 0);
  for (int i = 0; i < MAX_GAMEFILES; i++) {
    EXPECT_EQ(Gamefiles[i].used, 0);
    EXPECT_EQ(Gamefiles[i].name[0], 0);
  }
}

/**
 * @test GamefileTest.AllocReturnsSequentialIndices
 * @brief Verifies alloc Returns Sequential Indices.
 *
 * @details
 * Exercises the GamefileTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/gamefile.cpp
 * @ingroup descent3_tests
 */
TEST_F(GamefileTest, AllocReturnsSequentialIndices) {
  int a = AllocGamefile();
  int b = AllocGamefile();
  int c = AllocGamefile();
  EXPECT_EQ(a, 0);
  EXPECT_EQ(b, 1);
  EXPECT_EQ(c, 2);
  EXPECT_EQ(Num_gamefiles, 3);
  EXPECT_EQ(Gamefiles[0].used, 1);
  EXPECT_EQ(Gamefiles[1].used, 1);
  EXPECT_EQ(Gamefiles[2].used, 1);
}

/**
 * @test GamefileTest.FreeMarksUnusedAndDecrements
 * @brief Verifies free Marks Unused And Decrements.
 *
 * @details
 * Exercises the GamefileTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/gamefile.cpp
 * @ingroup descent3_tests
 */
TEST_F(GamefileTest, FreeMarksUnusedAndDecrements) {
  int a = AllocGamefile();
  FreeGamefile(a);
  EXPECT_EQ(Num_gamefiles, 0);
  EXPECT_EQ(Gamefiles[a].used, 0);
}

/**
 * @test GamefileTest.AllocReusesFreedSlot
 * @brief Verifies alloc Reuses Freed Slot.
 *
 * @details
 * Exercises the GamefileTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/gamefile.cpp
 * @ingroup descent3_tests
 */
TEST_F(GamefileTest, AllocReusesFreedSlot) {
  int a = AllocGamefile();
  int b = AllocGamefile();
  FreeGamefile(a);
  int c = AllocGamefile();
  EXPECT_EQ(c, a); // the freed low slot is reused first
  EXPECT_EQ(Num_gamefiles, 2);
}

/**
 * @test GamefileTest.GetNextWrapsAroundUsedEntries
 * @brief Verifies get Next Wraps Around Used Entries.
 *
 * @details
 * Exercises the GamefileTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/gamefile.cpp
 * @ingroup descent3_tests
 */
TEST_F(GamefileTest, GetNextWrapsAroundUsedEntries) {
  AllocGamefile(); // 0
  AllocGamefile(); // 1
  AllocGamefile(); // 2
  FreeGamefile(1); // only 0 and 2 used
  EXPECT_EQ(GetNextGamefile(0), 2);
  EXPECT_EQ(GetNextGamefile(2), 0); // wrap
}

/**
 * @test GamefileTest.GetPrevWrapsAroundUsedEntries
 * @brief Verifies get Prev Wraps Around Used Entries.
 *
 * @details
 * Exercises the GamefileTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/gamefile.cpp
 * @ingroup descent3_tests
 */
TEST_F(GamefileTest, GetPrevWrapsAroundUsedEntries) {
  AllocGamefile(); // 0
  AllocGamefile(); // 1
  AllocGamefile(); // 2
  FreeGamefile(1);
  EXPECT_EQ(GetPrevGamefile(2), 0);
  EXPECT_EQ(GetPrevGamefile(0), 2); // wrap
}

/**
 * @test GamefileTest.GetNextReturnsMinusOneWhenEmpty
 * @brief Verifies get Next Returns Minus One When Empty.
 *
 * @details
 * Exercises the GamefileTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/gamefile.cpp
 * @ingroup descent3_tests
 */
TEST_F(GamefileTest, GetNextReturnsMinusOneWhenEmpty) {
  EXPECT_EQ(GetNextGamefile(0), -1);
  EXPECT_EQ(GetPrevGamefile(0), -1);
}

/**
 * @test GamefileTest.FindByNameIsCaseInsensitive
 * @brief Verifies find By Name Is Case Insensitive.
 *
 * @details
 * Exercises the GamefileTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/gamefile.cpp
 * @ingroup descent3_tests
 */
TEST_F(GamefileTest, FindByNameIsCaseInsensitive) {
  int a = AllocGamefile();
  strcpy(Gamefiles[a].name, "MyMission");
  EXPECT_EQ(FindGamefileName("MyMission"), a);
  EXPECT_EQ(FindGamefileName("mymission"), a);
  EXPECT_EQ(FindGamefileName("NotThere"), -1);
  // unused entries are not matched
  int b = AllocGamefile();
  strcpy(Gamefiles[b].name, "Hidden");
  FreeGamefile(b);
  EXPECT_EQ(FindGamefileName("Hidden"), -1);
}

/**
 * @test GamefileTest.AllocExhaustionReturnsMinusOne
 * @brief Verifies alloc Exhaustion Returns Minus One.
 *
 * @details
 * Exercises the GamefileTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/gamefile.cpp
 * @ingroup descent3_tests
 */
TEST_F(GamefileTest, AllocExhaustionReturnsMinusOne) {
  int last = -1;
  for (int i = 0; i < MAX_GAMEFILES; i++) {
    last = AllocGamefile();
    ASSERT_NE(last, -1) << "ran out early at " << i;
  }
  EXPECT_EQ(Num_gamefiles, MAX_GAMEFILES);
  EXPECT_EQ(AllocGamefile(), -1); // table full -> Int3 + -1
}
