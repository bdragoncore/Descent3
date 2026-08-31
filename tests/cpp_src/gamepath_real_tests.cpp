/**
 * @file gamepath_real_tests.cpp
 * @brief Unit tests for Descent3/gamepath.cpp: the global game-path table init,.
 *
 * @details
 * free, and case-insensitive name lookup.
 *
 * This harness validates the behavior of `Descent3/gamepath.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/gamepath.cpp`
 * @par Harness
 * `gamepath_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/gamepath.cpp
 */

#include <gtest/gtest.h>
#include <cstring>

#include "gamepath.h"
#include "mem.h"

// FreeGamePath is defined in gamepath.cpp but not prototyped in the header.
void FreeGamePath(int n);

/**
 * @brief GTest fixture for GamepathTest.
 * @details
 * Sets up stubbed engine state and helper factories shared
 * across the suite. Each @test starts from the deterministic
 * baseline established in SetUp().
 * @ingroup descent3_tests
 */
class GamepathTest : public ::testing::Test {
protected:
  void SetUp() override { InitGamePaths(); }
};

/**
 * @test GamepathTest.InitClearsTableAndCount
 * @brief Verifies init Clears Table And Count.
 *
 * @details
 * Exercises the GamepathTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/gamepath.cpp
 * @ingroup descent3_tests
 */
TEST_F(GamepathTest, InitClearsTableAndCount) {
  // dirty it
  GamePaths[0].used = 1;
  GamePaths[0].num_nodes = 3;
  Num_game_paths = 1;
  InitGamePaths();
  EXPECT_EQ(Num_game_paths, 0);
  for (int i = 0; i < MAX_GAME_PATHS; i++) {
    EXPECT_EQ(GamePaths[i].used, 0);
    EXPECT_EQ(GamePaths[i].num_nodes, 0);
  }
}

/**
 * @test GamepathTest.InitIsIdempotentAndFreesExisting
 * @brief Verifies init Is Idempotent And Frees Existing.
 *
 * @details
 * Exercises the GamepathTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/gamepath.cpp
 * @ingroup descent3_tests
 */
TEST_F(GamepathTest, InitIsIdempotentAndFreesExisting) {
  GamePaths[2].used = 1;
  GamePaths[2].num_nodes = 1;
  GamePaths[2].pathnodes = nullptr; // FreeGamePath calls mem_free (safe on null)
  Num_game_paths = 1;
  InitGamePaths(); // second call frees index 2 first
  EXPECT_EQ(Num_game_paths, 0);
  EXPECT_EQ(GamePaths[2].used, 0);
}

/**
 * @test GamepathTest.FreeIgnoresUnusedEntry
 * @brief Verifies free Ignores Unused Entry.
 *
 * @details
 * Exercises the GamepathTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/gamepath.cpp
 * @ingroup descent3_tests
 */
TEST_F(GamepathTest, FreeIgnoresUnusedEntry) {
  Num_game_paths = 0;
  GamePaths[5].used = 0;
  FreeGamePath(5); // should early-return, no decrement
  EXPECT_EQ(Num_game_paths, 0);
  EXPECT_EQ(GamePaths[5].used, 0);
}

/**
 * @test GamepathTest.FreeClearsUsedEntry
 * @brief Verifies free Clears Used Entry.
 *
 * @details
 * Exercises the GamepathTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/gamepath.cpp
 * @ingroup descent3_tests
 */
TEST_F(GamepathTest, FreeClearsUsedEntry) {
  GamePaths[1].used = 1;
  GamePaths[1].num_nodes = 4;
  GamePaths[1].pathnodes = nullptr;
  Num_game_paths = 1;
  FreeGamePath(1);
  EXPECT_EQ(Num_game_paths, 0);
  EXPECT_EQ(GamePaths[1].used, 0);
  EXPECT_EQ(GamePaths[1].num_nodes, 0);
}

/**
 * @test GamepathTest.FindByNameIsCaseInsensitive
 * @brief Verifies find By Name Is Case Insensitive.
 *
 * @details
 * Exercises the GamepathTest code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/gamepath.cpp
 * @ingroup descent3_tests
 */
TEST_F(GamepathTest, FindByNameIsCaseInsensitive) {
  int a = 3;
  GamePaths[a].used = 1;
  strcpy(GamePaths[a].name, "PatrolRoute");
  EXPECT_EQ(FindGamePathName("PatrolRoute"), a);
  EXPECT_EQ(FindGamePathName("patrolroute"), a);
  EXPECT_EQ(FindGamePathName("missing"), -1);

  // unused entries are skipped
  int b = 7;
  strcpy(GamePaths[b].name, "Hidden");
  GamePaths[b].used = 0;
  EXPECT_EQ(FindGamePathName("Hidden"), -1);
}
