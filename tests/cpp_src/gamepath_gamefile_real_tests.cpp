/**
 * @file gamepath_gamefile_real_tests.cpp
 * @brief Tests for gamepath.cpp (125 lines) and gamefile.cpp (119 lines).
 *
 * @details
 * Covers path lifecycle/reinit/free-list semantics, name lookup, and the
 * gamefile alloc/ring-navigation/name-search API.
 *
 * This harness validates the behavior of `Descent3/gamepath_gamefile.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/gamepath_gamefile.cpp`
 * @par Harness
 * `gamepath_gamefile_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/gamepath_gamefile.cpp
 */

#include <gtest/gtest.h>
#include <cstring>

#include "gamepath.h"
#include "gamefile.h"
#include "log.h"
#include "mem.h"

// Defined in gamepath.cpp but missing from gamepath.h
void FreeGamePath(int n);

// ---------------------------------------------------------------------------
// Game paths
// ---------------------------------------------------------------------------

/**
 * @test GamePath.InitZeroesEverythingOnFirstCall
 * @brief Verifies init Zeroes Everything On First Call.
 *
 * @details
 * Exercises the GamePath code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/gamepath_gamefile.cpp
 * @ingroup descent3_tests
 */
TEST(GamePath, InitZeroesEverythingOnFirstCall) {
  GamePaths[5].num_nodes = 99;
  GamePaths[5].used = true;

  InitGamePaths();

  EXPECT_FALSE(GamePaths[5].used);
  EXPECT_EQ(Num_game_paths, 0);
  for (int i = 0; i < MAX_GAME_PATHS; i++)
    EXPECT_FALSE(GamePaths[i].used);
}

/**
 * @test GamePath.FreeReleasesNodesAndDecrementsCounter
 * @brief Verifies free Releases Nodes And Decrements Counter.
 *
 * @details
 * Exercises the GamePath code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/gamepath_gamefile.cpp
 * @ingroup descent3_tests
 */
TEST(GamePath, FreeReleasesNodesAndDecrementsCounter) {
  InitGamePaths();
  GamePaths[3].used = true;
  GamePaths[3].num_nodes = 7;
  GamePaths[3].pathnodes = (node *)mem_malloc(7 * sizeof(node));
  ASSERT_NE(GamePaths[3].pathnodes, nullptr);
  Num_game_paths = 1;
  strcpy(GamePaths[3].name, "patrol_a");

  FreeGamePath(3);

  EXPECT_FALSE(GamePaths[3].used);
  EXPECT_EQ(GamePaths[3].num_nodes, 0);
  EXPECT_EQ(Num_game_paths, 0);
}

/**
 * @test GamePath.FreeOfUnusedPathIsNoop
 * @brief Verifies free Of Unused Path Is Noop.
 *
 * @details
 * Exercises the GamePath code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/gamepath_gamefile.cpp
 * @ingroup descent3_tests
 */
TEST(GamePath, FreeOfUnusedPathIsNoop) {
  InitGamePaths();
  Num_game_paths = 4; // pretend others exist

  FreeGamePath(9); // unused slot

  EXPECT_EQ(Num_game_paths, 4); // untouched
}

/**
 * @test GamePath.ReInitFreesPreviouslyUsedPaths
 * @brief Verifies re Init Frees Previously Used Paths.
 *
 * @details
 * Exercises the GamePath code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/gamepath_gamefile.cpp
 * @ingroup descent3_tests
 */
TEST(GamePath, ReInitFreesPreviouslyUsedPaths) {
  InitGamePaths();
  GamePaths[1].used = true;
  GamePaths[1].pathnodes = (node *)mem_malloc(sizeof(node));
  GamePaths[2].used = true;
  GamePaths[2].pathnodes = (node *)mem_malloc(sizeof(node));
  Num_game_paths = 2;

  // Second call takes the cleanup branch before zeroing again.
  InitGamePaths();

  EXPECT_EQ(Num_game_paths, 0);
  EXPECT_FALSE(GamePaths[1].used);
  EXPECT_FALSE(GamePaths[2].used);
}

/**
 * @test GamePath.FindByNameIsCaseInsensitive
 * @brief Verifies find By Name Is Case Insensitive.
 *
 * @details
 * Exercises the GamePath code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/gamepath_gamefile.cpp
 * @ingroup descent3_tests
 */
TEST(GamePath, FindByNameIsCaseInsensitive) {
  InitGamePaths();
  GamePaths[10].used = true;
  strcpy(GamePaths[10].name, "Patrol_Route");

  EXPECT_EQ(FindGamePathName("patrol_route"), 10);
  EXPECT_EQ(FindGamePathName("PATROL_ROUTE"), 10);
  EXPECT_EQ(FindGamePathName("nope"), -1);
}

/**
 * @test GamePath.FindSkipsUnusedSlots
 * @brief Verifies find Skips Unused Slots.
 *
 * @details
 * Exercises the GamePath code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/gamepath_gamefile.cpp
 * @ingroup descent3_tests
 */
TEST(GamePath, FindSkipsUnusedSlots) {
  InitGamePaths();
  strcpy(GamePaths[4].name, "ghost"); // not marked used
  GamePaths[4].used = false;

  EXPECT_EQ(FindGamePathName("ghost"), -1);
}

// ---------------------------------------------------------------------------
// Game files
// ---------------------------------------------------------------------------

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
 * @test GamefileTest.InitClearsAllEntries
 * @brief Verifies init Clears All Entries.
 *
 * @details
 * Exercises the GamefileTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/gamepath_gamefile.cpp
 * @ingroup descent3_tests
 */
TEST_F(GamefileTest, InitClearsAllEntries) {
  Gamefiles[7].used = 1;
  strcpy(Gamefiles[7].name, "stale");
  InitGamefiles();
  EXPECT_FALSE(Gamefiles[7].used);
  EXPECT_EQ(Num_gamefiles, 0);
}

/**
 * @test GamefileTest.AllocTakesLowestFreeSlotAndCounts
 * @brief Verifies alloc Takes Lowest Free Slot And Counts.
 *
 * @details
 * Exercises the GamefileTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/gamepath_gamefile.cpp
 * @ingroup descent3_tests
 */
TEST_F(GamefileTest, AllocTakesLowestFreeSlotAndCounts) {
  int a = AllocGamefile();
  int b = AllocGamefile();
  EXPECT_EQ(a, 0);
  EXPECT_EQ(b, 1);
  EXPECT_TRUE(Gamefiles[b].used);

  FreeGamefile(a);
  EXPECT_EQ(AllocGamefile(), 0); // reused
  EXPECT_EQ(Num_gamefiles, 2);
}

/**
 * @test GamefileTest.AllocMemsetsNewEntry
 * @brief Verifies alloc Memsets New Entry.
 *
 * @details
 * Exercises the GamefileTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/gamepath_gamefile.cpp
 * @ingroup descent3_tests
 */
TEST_F(GamefileTest, AllocMemsetsNewEntry) {
  strcpy(Gamefiles[0].name, "garbage");
  int n = AllocGamefile();
  EXPECT_EQ(n, 0);
  EXPECT_STREQ(Gamefiles[n].name, ""); // memset wiped old contents
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
 * @see Descent3/gamepath_gamefile.cpp
 * @ingroup descent3_tests
 */
TEST_F(GamefileTest, AllocExhaustionReturnsMinusOne) {
  for (int i = 0; i < MAX_GAMEFILES; i++)
    ASSERT_GE(AllocGamefile(), 0);
  EXPECT_EQ(AllocGamefile(), -1);
  EXPECT_EQ(Num_gamefiles, MAX_GAMEFILES);
}

/**
 * @test GamefileTest.FreeClearsEntry
 * @brief Verifies free Clears Entry.
 *
 * @details
 * Exercises the GamefileTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/gamepath_gamefile.cpp
 * @ingroup descent3_tests
 */
TEST_F(GamefileTest, FreeClearsEntry) {
  int n = AllocGamefile();
  strcpy(Gamefiles[n].name, "mnu_background");
  FreeGamefile(n);
  EXPECT_FALSE(Gamefiles[n].used);
  EXPECT_STREQ(Gamefiles[n].name, "");
  EXPECT_EQ(Num_gamefiles, 0);
}

/**
 * @test GamefileTest.NextWrapsAroundEnd
 * @brief Verifies next Wraps Around End.
 *
 * @details
 * Exercises the GamefileTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/gamepath_gamefile.cpp
 * @ingroup descent3_tests
 */
TEST_F(GamefileTest, NextWrapsAroundEnd) {
  int a = AllocGamefile(); // slot 0
  ASSERT_EQ(a, 0);

  EXPECT_EQ(GetNextGamefile(MAX_GAMEFILES - 1), 0); // wraps to the only file
  EXPECT_EQ(GetNextGamefile(0), 0);                 // sole entry -> itself

  int b = AllocGamefile(); // slot 1
  ASSERT_EQ(b, 1);
  EXPECT_EQ(GetNextGamefile(0), 1);   // forward scan
  EXPECT_EQ(GetNextGamefile(1), 0);   // wraps past end back to 0
}

/**
 * @test GamefileTest.PrevWrapsAroundStart
 * @brief Verifies prev Wraps Around Start.
 *
 * @details
 * Exercises the GamefileTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/gamepath_gamefile.cpp
 * @ingroup descent3_tests
 */
TEST_F(GamefileTest, PrevWrapsAroundStart) {
  int a = AllocGamefile(); // 0
  int b = AllocGamefile(); // 1
  (void)a;

  EXPECT_EQ(GetPrevGamefile(0), 1);   // wraps below start to highest
  EXPECT_EQ(GetPrevGamefile(1), 0);   // backward scan
}

/**
 * @test GamefileTest.NavigationReturnsMinusOneWhenNoFiles
 * @brief Verifies navigation Returns Minus One When No Files.
 *
 * @details
 * Exercises the GamefileTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/gamepath_gamefile.cpp
 * @ingroup descent3_tests
 */
TEST_F(GamefileTest, NavigationReturnsMinusOneWhenNoFiles) {
  EXPECT_EQ(GetNextGamefile(0), -1);
  EXPECT_EQ(GetPrevGamefile(0), -1);
}

/**
 * @test GamefileTest.FindByNameCaseInsensitive
 * @brief Verifies find By Name Case Insensitive.
 *
 * @details
 * Exercises the GamefileTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/gamepath_gamefile.cpp
 * @ingroup descent3_tests
 */
TEST_F(GamefileTest, FindByNameCaseInsensitive) {
  int n = AllocGamefile();
  strcpy(Gamefiles[n].name, "Intro.Bkg");
  EXPECT_EQ(FindGamefileName("intro.bkg"), n);
  EXPECT_EQ(FindGamefileName("INTRO.BKG"), n);
  EXPECT_EQ(FindGamefileName("missing"), -1);
}
