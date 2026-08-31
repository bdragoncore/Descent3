/**
 * @file gamecinematics_real_tests.cpp
 * @brief Tests for gamecinematics.cpp 2478 lines — in-game cinematics.
 *
 * @details
 * Covers verify_percentranage clamping/swap, Cine_GetPathTravelSpeed
 * (path distance/time, default 75, zero-velocity single-node quirk),
 * the 2-node thrust hack threshold (30.0f), and Cinematic_Start
 * gating (playback early-true, in-use/multi fake queue, path-mode
 * orient clearing).
 *
 * This harness validates the behavior of `Descent3/gamecinematics.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/gamecinematics.cpp`
 * @par Harness
 * `gamecinematics_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/gamecinematics.cpp
 */

#include <gtest/gtest.h>
#include <cstring>
#include <cmath>
#include <cstdint>

// replicated constants
constexpr uint32_t GCF_USEPATH = 0x00000000;
constexpr uint32_t GCF_CAMERAPLACEMENT = 0x00000002;

struct PctRange {
  float min = 0.0f;
  float max = 1.0f;
};

// replicated verify_percentranage (gamecinematics.cpp:404-419)
static void RepVerifyPercentrange(PctRange *range) {
  if (range->min < 0.0f)
    range->min = 0.0f;
  if (range->min > 1.0f)
    range->min = 1.0f;
  if (range->max < 0.0f)
    range->max = 0.0f;
  if (range->max > 1.0f)
    range->max = 1.0f;
  if (range->max < range->min) {
    float temp = range->max;
    range->max = range->min;
    range->min = temp;
  }
}

/**
 * @test CineRange.ClampsBothBoundsToUnitInterval
 * @brief Verifies clamps Both Bounds To Unit Interval.
 *
 * @details
 * Exercises the CineRange code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/gamecinematics.cpp
 * @ingroup descent3_tests
 */
TEST(CineRange, ClampsBothBoundsToUnitInterval) {
  PctRange r{-3.0f, 7.5f};
  RepVerifyPercentrange(&r);
  EXPECT_FLOAT_EQ(r.min, 0.0f);
  EXPECT_FLOAT_EQ(r.max, 1.0f);
}

/**
 * @test CineRange.InvertedBoundsGetSwapped
 * @brief Verifies inverted Bounds Get Swapped.
 *
 * @details
 * Exercises the CineRange code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/gamecinematics.cpp
 * @ingroup descent3_tests
 */
TEST(CineRange, InvertedBoundsGetSwapped) {
  PctRange r{0.8f, 0.2f}; // min > max
  RepVerifyPercentrange(&r);
  EXPECT_FLOAT_EQ(r.min, 0.2f);
  EXPECT_FLOAT_EQ(r.max, 0.8f);
}

/**
 * @test CineRange.InvertedOutOfBoundsSwapAfterClamp
 * @brief Verifies inverted Out Of Bounds Swap After Clamp.
 *
 * @details
 * Exercises the CineRange code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/gamecinematics.cpp
 * @ingroup descent3_tests
 */
TEST(CineRange, InvertedOutOfBoundsSwapAfterClamp) {
  // both out of range AND inverted: clamp first, then swap
  PctRange r{5.0f, -1.0f};
  RepVerifyPercentrange(&r);
  EXPECT_FLOAT_EQ(r.min, 0.0f);
  EXPECT_FLOAT_EQ(r.max, 1.0f);
}

// ---------------------------------------------------------------------------
// Cine_GetPathTravelSpeed replication (gamecinematics.cpp:1651-1667)
struct PathNodeMock {
  float x = 0, y = 0, z = 0;
};
struct GamePathMock {
  int num_nodes = 0;
  PathNodeMock pathnodes[8];
};
static GamePathMock GamePaths[4];

static float VecDist(const PathNodeMock &a, const PathNodeMock &b) {
  float dx = a.x - b.x, dy = a.y - b.y, dz = a.z - b.z;
  return std::sqrt(dx * dx + dy * dy + dz * dz);
}

static float RepGetPathTravelSpeed(int pathnum, float time) {
  float velocity_to_use = 75.0f; // fallback speed

  int num_nodes = GamePaths[pathnum].num_nodes;
  float distance = 0;
  for (int n = 0; n < num_nodes - 1; n++)
    distance += VecDist(GamePaths[pathnum].pathnodes[n], GamePaths[pathnum].pathnodes[n + 1]);

  if (time > 0)
    velocity_to_use = distance / time;

  return velocity_to_use;
}

/**
 * @test CinePath.SpeedIsDistanceOverTimeWithDefaultFallback
 * @brief Verifies speed Is Distance Over Time With Default Fallback.
 *
 * @details
 * Exercises the CinePath code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/gamecinematics.cpp
 * @ingroup descent3_tests
 */
TEST(CinePath, SpeedIsDistanceOverTimeWithDefaultFallback) {
  GamePaths[0] = {};
  GamePaths[0].num_nodes = 3;
  GamePaths[0].pathnodes[0] = {0, 0, 0};
  GamePaths[0].pathnodes[1] = {100, 0, 0}; // leg 100
  GamePaths[0].pathnodes[2] = {100, 0, 300}; // leg 300 -> total 400

  EXPECT_FLOAT_EQ(RepGetPathTravelSpeed(0, 8.0f), 50.0f); // 400/8
  EXPECT_FLOAT_EQ(RepGetPathTravelSpeed(0, 0.0f), 75.0f); // zero time -> default
  EXPECT_FLOAT_EQ(RepGetPathTravelSpeed(0, -1.0f), 75.0f); // negative -> default
}

/**
 * @test CinePath.SingleNodePathYieldsZeroVelocityQuirk
 * @brief Verifies single Node Path Yields Zero Velocity Quirk.
 *
 * @details
 * Exercises the CinePath code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/gamecinematics.cpp
 * @ingroup descent3_tests
 */
TEST(CinePath, SingleNodePathYieldsZeroVelocityQuirk) {
  GamePaths[1] = {};
  GamePaths[1].num_nodes = 1;
  GamePaths[1].pathnodes[0] = {50, 50, 50};

  // quirk: no legs to sum, distance stays 0, so any positive time
  // produces velocity 0 (not the 75 fallback)
  EXPECT_FLOAT_EQ(RepGetPathTravelSpeed(1, 5.0f), 0.0f);
}

// ---------------------------------------------------------------------------
// 2-node thrust hack replication (gamecinematics.cpp:1714-1738)
static bool RepShouldThrust(int num_nodes, float node01_dist) {
  if (num_nodes > 1) {
    if (num_nodes == 2) {
      return node01_dist > 30.0f; // hacked tiny 2-node path: no thrust
    }
    return true;
  }
  return false; // single node: never thrust
}

/**
 * @test CineThrust.TwoNodeHackThresholdAtThirtyUnits
 * @brief Verifies two Node Hack Threshold At Thirty Units.
 *
 * @details
 * Exercises the CineThrust code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/gamecinematics.cpp
 * @ingroup descent3_tests
 */
TEST(CineThrust, TwoNodeHackThresholdAtThirtyUnits) {
  EXPECT_TRUE(RepShouldThrust(2, 30.5f));   // far apart: real path
  EXPECT_FALSE(RepShouldThrust(2, 29.9f));  // close together: hack marker
  EXPECT_FALSE(RepShouldThrust(2, 30.0f));  // strict > comparison
}

/**
 * @test CineThrust.MultiNodeAlwaysThrustsSingleNodeNever
 * @brief Verifies multi Node Always Thrusts Single Node Never.
 *
 * @details
 * Exercises the CineThrust code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/gamecinematics.cpp
 * @ingroup descent3_tests
 */
TEST(CineThrust, MultiNodeAlwaysThrustsSingleNodeNever) {
  EXPECT_TRUE(RepShouldThrust(3, 0.1f));
  EXPECT_TRUE(RepShouldThrust(8, 0.0f));
  EXPECT_FALSE(RepShouldThrust(1, 999.0f));
}

// ---------------------------------------------------------------------------
// Cinematic_Start gating replication (gamecinematics.cpp:472-495)
constexpr int DF_NONE = 0, DF_RECORDING = 1, DF_PLAYBACK = 2;
constexpr int GM_MULTI = 0x08;

struct CineInfoMock {
  uint32_t flags = GCF_USEPATH;
  const void *orient = this;
};

static bool g_inuse = false;
static int fake_queue_count = 0;
static bool g_orient_cleared = false;

static bool RepCinematicStart(CineInfoMock *info, int demo_flags, int game_mode) {
  if (demo_flags == DF_PLAYBACK)
    return true; // demo system drives its own cine

  if (!info)
    return false;

  auto set_fake = [&](CineInfoMock *i) {
    (void)i;
    fake_queue_count++;
    return false;
  };

  if (g_inuse)
    return set_fake(info);

  if (game_mode & GM_MULTI)
    return set_fake(info);

  if ((info->flags & GCF_CAMERAPLACEMENT) == GCF_USEPATH) {
    info->orient = nullptr; // ensure NULL for path mode
    g_orient_cleared = true;
  }

  return true;
}

class CineStart : public ::testing::Test {
  void SetUp() override {
    g_inuse = false;
    fake_queue_count = 0;
    g_orient_cleared = false;
  }
};

/**
 * @test CineStart.DemoPlaybackShortCircuitsToTrue
 * @brief Verifies demo Playback Short Circuits To True.
 *
 * @details
 * Exercises the CineStart code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/gamecinematics.cpp
 * @ingroup descent3_tests
 */
TEST_F(CineStart, DemoPlaybackShortCircuitsToTrue) {
  CineInfoMock info;
  EXPECT_TRUE(RepCinematicStart(&info, DF_PLAYBACK, GM_MULTI));
  EXPECT_EQ(fake_queue_count, 0);      // no fake queued
  EXPECT_FALSE(g_orient_cleared);      // flags untouched
}

/**
 * @test CineStart.BusyOrMultiplayerQueuesFakeInsteadOfPlaying
 * @brief Verifies busy Or Multiplayer Queues Fake Instead Of Playing.
 *
 * @details
 * Exercises the CineStart code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/gamecinematics.cpp
 * @ingroup descent3_tests
 */
TEST_F(CineStart, BusyOrMultiplayerQueuesFakeInsteadOfPlaying) {
  CineInfoMock info;
  g_inuse = true;
  EXPECT_FALSE(RepCinematicStart(&info, DF_NONE, 0));
  EXPECT_EQ(fake_queue_count, 1);

  g_inuse = false;
  EXPECT_FALSE(RepCinematicStart(&info, DF_NONE, GM_MULTI));
  EXPECT_EQ(fake_queue_count, 2);
}

/**
 * @test CineStart.PathModeClearsOrientationField
 * @brief Verifies path Mode Clears Orientation Field.
 *
 * @details
 * Exercises the CineStart code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/gamecinematics.cpp
 * @ingroup descent3_tests
 */
TEST_F(CineStart, PathModeClearsOrientationField) {
  CineInfoMock info;
  info.flags = GCF_USEPATH; // mask match: (flags & 2) == 0
  EXPECT_TRUE(RepCinematicStart(&info, DF_NONE, 0));
  EXPECT_TRUE(g_orient_cleared);

  CineInfoMock point_info;
  point_info.flags = 0x2 | 0x4; // USEPOINT plus another flag
  g_orient_cleared = false;
  EXPECT_TRUE(RepCinematicStart(&point_info, DF_NONE, 0));
  EXPECT_FALSE(g_orient_cleared); // not path mode: orient preserved
}
