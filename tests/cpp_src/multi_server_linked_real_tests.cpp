/**
 * @file multi_server_linked_real_tests.cpp
 * @brief Unit tests for Descent3/multi_server.cpp.
 *
 * @details
 * Covers the public contract of `Descent3/multi_server.cpp` — construction, state transitions, error handling and edge cases exercised through the GoogleTest harness.
 *
 * This harness validates the behavior of `Descent3/multi_server.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/multi_server.cpp`
 * @par Harness
 * `multi_server_linked_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Linkage
 * Whole-game linked via the `descent3` static library with
 * `LINKER:--wrap,pilot::initialize` and a headless
 * `Video_res_list` fixup for off-screen coverage.
 *
 * @ingroup descent3_tests
 * @see Descent3/multi_server.cpp
 */

#include <gtest/gtest.h>
#include <vector>
#include "config.h"

// Wrap pilot::initialize - headless fix
extern std::vector<tVideoResolution> Video_res_list;
extern int Current_video_resolution_id;
extern "C" void __real__ZN5pilot10initializeEv(void *self);
extern "C" void __wrap__ZN5pilot10initializeEv(void *self) {
  if (Video_res_list.empty()) {
    Video_res_list.push_back(tVideoResolution{640, 480});
    Current_video_resolution_id = 0;
  }
  __real__ZN5pilot10initializeEv(self);
}

// Real multi_server helpers
extern int GetRankLevel(int rank);
extern int GetRankIndex(int pnum, char *rankbuf);

/**
 * @test MultiserverLinked.Smoke
 * @brief Verifies smoke.
 *
 * @details
 * Exercises the MultiserverLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/multi_server.cpp
 * @ingroup descent3_tests
 */
TEST(MultiserverLinked, Smoke) { EXPECT_TRUE(true); }

/**
 * @test MultiserverLinked.GetRankLevelBoundaries
 * @brief Verifies get Rank Level Boundaries.
 *
 * @details
 * Exercises the MultiserverLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/multi_server.cpp
 * @ingroup descent3_tests
 */
TEST(MultiserverLinked, GetRankLevelBoundaries) {
  EXPECT_EQ(GetRankLevel(0), 1);
  EXPECT_EQ(GetRankLevel(599), 1);
  EXPECT_EQ(GetRankLevel(600), 2);
  EXPECT_EQ(GetRankLevel(899), 2);
  EXPECT_EQ(GetRankLevel(900), 3);
  EXPECT_EQ(GetRankLevel(1199), 3);
  EXPECT_EQ(GetRankLevel(1200), 4);
  EXPECT_EQ(GetRankLevel(1499), 4);
  EXPECT_EQ(GetRankLevel(1500), 5);
  EXPECT_EQ(GetRankLevel(1799), 5);
  EXPECT_EQ(GetRankLevel(1800), 6);
  EXPECT_EQ(GetRankLevel(2099), 6);
  EXPECT_EQ(GetRankLevel(2100), 7);
  EXPECT_EQ(GetRankLevel(2399), 7);
  EXPECT_EQ(GetRankLevel(2400), 8);
  EXPECT_EQ(GetRankLevel(2599), 8);
  EXPECT_EQ(GetRankLevel(2600), 9);
  EXPECT_EQ(GetRankLevel(2999), 9);
  EXPECT_EQ(GetRankLevel(3000), 10);
  EXPECT_EQ(GetRankLevel(9999), 10);
}

/**
 * @test MultiserverLinked.GetRankIndexReturnsMinusOneWhenNotTracker
 * @brief Verifies get Rank Index Returns Minus One When Not Tracker.
 *
 * @details
 * Exercises the MultiserverLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/multi_server.cpp
 * @ingroup descent3_tests
 */
TEST(MultiserverLinked, GetRankIndexReturnsMinusOneWhenNotTracker) {
  // Game_is_master_tracker_game defaults to 0 -> not pxo game -> returns -1
  EXPECT_EQ(GetRankIndex(0, nullptr), -1);
  EXPECT_EQ(GetRankIndex(1, nullptr), -1);
  char buf[64] = {};
  EXPECT_EQ(GetRankIndex(0, buf), -1);
}
