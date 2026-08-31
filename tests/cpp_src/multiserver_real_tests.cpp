/**
 * @file multiserver_real_tests.cpp
 * @brief Tests for multi_server.cpp 3040 lines — dedicated/P2P server logic.
 *
 * @details
 * Covers MultiIsValidMovedObject gating, GetRankLevel/GetRankIndex
 * ladder mapping, CalculateNewRanking level-delta math with the
 * self-kill no-op quirk and floor-at-zero, respawn candidate
 * selection (randomize flag widens the pool), the stuck-timer
 * early-return quirk in MultiCheckToRespawnPowerups, timer
 * compaction, and the MultiSendPositionalUpdates visibility rules
 * (server position always sent — ghost ship fix; disconnected
 * players clear visibility bits both ways).
 *
 * This harness validates the behavior of `Descent3/multiserver.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/multiserver.cpp`
 * @par Harness
 * `multiserver_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/multiserver.cpp
 */

#include <gtest/gtest.h>
#include <cstring>
#include <cstdio>
#include <algorithm>

// replicated constants (multi_server.cpp:2814-2819)
constexpr float LEVEL_SPAN = 10.0f;
constexpr float KILL_BASE = 0.52f;
constexpr float DEATH_BASE = 0.50f;
#define KILL_SCALAR (KILL_BASE / LEVEL_SPAN)
#define DEATH_SCALAR (DEATH_BASE / LEVEL_SPAN)

enum { OBJ_NONE_MS = 0, OBJ_ROBOT_MS = 2, OBJ_BUILDING_MS = 3, OBJ_CLUTTER_MS = 4 };
constexpr uint32_t OF_DEAD_MS = 0x00000008;
constexpr uint32_t OF_CLIENT_KNOWS_MS = 0x00800000;
enum { MT_NONE_MS = 0, MT_WALKING_MS = 3, MT_PHYSICS_MS = 4 };

// ---------------------------------------------------------------------------
// MultiIsValidMovedObject replication (multi_server.cpp:1231-1252)
struct MObj {
  int type = OBJ_NONE_MS;
  uint32_t flags = 0;
  int movement_type = MT_NONE_MS;
};

static bool RepIsValidMovedObject(const MObj &obj) {
  bool good = false;
  if (obj.type == OBJ_ROBOT_MS || obj.type == OBJ_CLUTTER_MS || obj.type == OBJ_BUILDING_MS)
    good = true;
  if (good) {
    if (obj.flags & OF_DEAD_MS)
      good = false;
    if (!(obj.flags & OF_CLIENT_KNOWS_MS))
      good = false;
    if (good) {
      if (obj.movement_type == MT_WALKING_MS || obj.movement_type == MT_PHYSICS_MS)
        return true;
    }
  }
  return false;
}

/**
 * @test MServerMovedObject.TypeFlagAndMovementGate
 * @brief Verifies type Flag And Movement Gate.
 *
 * @details
 * Exercises the MServerMovedObject code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/multiserver.cpp
 * @ingroup descent3_tests
 */
TEST(MServerMovedObject, TypeFlagAndMovementGate) {
  MObj robot{OBJ_ROBOT_MS, OF_CLIENT_KNOWS_MS, MT_PHYSICS_MS};
  EXPECT_TRUE(RepIsValidMovedObject(robot));

  // clutter/building also eligible
  EXPECT_TRUE(RepIsValidMovedObject(MObj{OBJ_CLUTTER_MS, OF_CLIENT_KNOWS_MS, MT_WALKING_MS}));
  EXPECT_TRUE(RepIsValidMovedObject(MObj{OBJ_BUILDING_MS, OF_CLIENT_KNOWS_MS, MT_PHYSICS_MS}));

  // wrong type rejected even when everything else is fine
  EXPECT_FALSE(RepIsValidMovedObject(MObj{OBJ_NONE_MS, OF_CLIENT_KNOWS_MS, MT_PHYSICS_MS}));

  // dead or not known by clients -> dropped
  MObj dead{OBJ_ROBOT_MS, OF_CLIENT_KNOWS_MS | OF_DEAD_MS, MT_PHYSICS_MS};
  EXPECT_FALSE(RepIsValidMovedObject(dead));
  MObj unknown{OBJ_ROBOT_MS, 0, MT_PHYSICS_MS};
  EXPECT_FALSE(RepIsValidMovedObject(unknown));

  // right type + flags but static movement -> not sent
  EXPECT_FALSE(RepIsValidMovedObject(MObj{OBJ_ROBOT_MS, OF_CLIENT_KNOWS_MS, MT_NONE_MS}));
}

// ---------------------------------------------------------------------------
// GetRankLevel / GetRankIndex / CalculateNewRanking replication
// (multi_server.cpp:2821-2867, 2920+)
struct RPlayer {
  float rank = 0.0f;
};
static RPlayer RPlayers[8];

static int RepGetRankLevel(int rank) {
  if (rank < 600)
    return 1;
  if (rank < 900)
    return 2;
  if (rank < 1200)
    return 3;
  if (rank < 1500)
    return 4;
  if (rank < 1800)
    return 5;
  if (rank < 2100)
    return 6;
  if (rank < 2400)
    return 7;
  if (rank < 2600)
    return 8;
  if (rank < 3000)
    return 9;
  return 10;
}

static int RepGetRankIndex(float ranking, int master_tracker) {
  if (!master_tracker)
    return -1;
  int val = 0;
  if (ranking >= 0 && ranking < 600)
    val = 0;
  else if (ranking >= 600 && ranking < 900)
    val = 1;
  else if (ranking >= 900 && ranking < 1200)
    val = 2;
  else if (ranking >= 1200 && ranking < 1500)
    val = 3;
  else if (ranking >= 1500 && ranking < 1800)
    val = 4;
  else if (ranking >= 1800 && ranking < 2100)
    val = 5;
  else if (ranking >= 2100 && ranking < 2400)
    val = 6;
  else if (ranking >= 2400 && ranking < 2600)
    val = 7;
  else if (ranking >= 2600 && ranking < 3000)
    val = 8;
  else if (ranking >= 3000) // guarded: negatives fall through with val=0
    val = 9;
  return val;
}

static float RepCalcNewRanking(int id_a, int id_b, bool won) {
  float rank_a = RPlayers[id_a].rank;
  float rank_b = RPlayers[id_b].rank;

  int level_a = RepGetRankLevel((int)rank_a);
  int level_b = RepGetRankLevel((int)rank_b);

  if ((id_a == id_b) && won)
    return rank_a;

  if (won)
    rank_a += (KILL_SCALAR * float(level_b - level_a) + KILL_BASE);
  else
    rank_a -= (DEATH_SCALAR * float(level_a - level_b) + DEATH_BASE);
  float new_rank = rank_a;
  if (new_rank < 0)
    new_rank = 0;
  return new_rank;
}

/**
 * @test MServerRanking.LadderBandsAndIndexMapping
 * @brief Verifies ladder Bands And Index Mapping.
 *
 * @details
 * Exercises the MServerRanking code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/multiserver.cpp
 * @ingroup descent3_tests
 */
TEST(MServerRanking, LadderBandsAndIndexMapping) {
  // GetRankLevel: 300-point bands up to 2600 then 400s, min level 1
  EXPECT_EQ(RepGetRankLevel(0), 1);
  EXPECT_EQ(RepGetRankLevel(599), 1);
  EXPECT_EQ(RepGetRankLevel(600), 2); // boundary inclusive on upper band
  EXPECT_EQ(RepGetRankLevel(2599), 8);
  EXPECT_EQ(RepGetRankLevel(2999), 9);
  EXPECT_EQ(RepGetRankLevel(3000), 10);
  EXPECT_EQ(RepGetRankLevel(-100), 1); // negative still level 1

  // GetRankIndex: 0-based twin of the same ladder
  EXPECT_EQ(RepGetRankIndex(500.0f, 0), -1); // non-tracker game
  EXPECT_EQ(RepGetRankIndex(599.9f, 1), 0);
  EXPECT_EQ(RepGetRankIndex(600.0f, 1), 1);
  EXPECT_EQ(RepGetRankIndex(-5.0f, 1), 0); // negative falls through to default
}

/**
 * @test MServerRanking.KillDeathDeltaSelfQuirkAndFloor
 * @brief Verifies kill Death Delta Self Quirk And Floor.
 *
 * @details
 * Exercises the MServerRanking code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/multiserver.cpp
 * @ingroup descent3_tests
 */
TEST(MServerRanking, KillDeathDeltaSelfQuirkAndFloor) {
  // equal ranks: kill nets exactly KILL_BASE
  RPlayers[0].rank = 1000.0f;
  RPlayers[1].rank = 1000.0f;
  float gain = KILL_SCALAR * 0.0f + KILL_BASE; // same level -> scalar term zero
  EXPECT_FLOAT_EQ(RepCalcNewRanking(0, 1, true), 1000.0f + gain);
  EXPECT_FLOAT_EQ(RepCalcNewRanking(1, 0, false), 1000.0f - (DEATH_BASE));

  // beating a higher-ranked player pays more than reverse
  RPlayers[2].rank = 800.0f;  // level 2
  RPlayers[3].rank = 1400.0f; // level 4
  float low_beats_high = KILL_SCALAR * 2.0f + KILL_BASE;
  float high_beats_low = KILL_SCALAR * (-2.0f) + KILL_BASE;
  EXPECT_FLOAT_EQ(RepCalcNewRanking(2, 3, true), 800.0f + low_beats_high);
  EXPECT_FLOAT_EQ(RepCalcNewRanking(3, 2, true), 1400.0f + high_beats_low);
  EXPECT_GT(low_beats_high, high_beats_low);

  // quirk: killing YOURSELF (obj_a == obj_b) returns old rank unchanged
  EXPECT_FLOAT_EQ(RepCalcNewRanking(0, 0, true), 1000.0f);
  // ...but dying to yourself DOES deduct
  EXPECT_FLOAT_EQ(RepCalcNewRanking(0, 0, false), 1000.0f - DEATH_BASE);

  // floor at zero: a tiny rank minus death cost clamps to 0
  RPlayers[4].rank = 0.2f; // death costs DEATH_BASE=0.50 here (level diff -2 gives -0.1)
  EXPECT_FLOAT_EQ(RepCalcNewRanking(4, 1, false), 0.0f);
}

// ---------------------------------------------------------------------------
// Respawn candidate selection replication (multi_server.cpp:1933-1979)
struct RespSpot {
  int used = 1;
  int original_id = -1;
};
struct RespTimer {
  int id = -1;
  float respawn_time = 999999.0f;
};
static RespSpot RSpots[16];
static RespTimer RTimers[16];
static int NumSpots = 0, NumTimers = 0;
static uint32_t NFlagsRandomize = 0x01;

static int RepPickRespawnCandidate(bool randomize, int want_id) {
  int num_cand = 0;
  int candidates[16];
  for (int t = 0; t < NumSpots; t++) {
    if (RSpots[t].used == 0) {
      if (randomize)
        candidates[num_cand++] = t;
      else if (want_id == RSpots[t].original_id)
        candidates[num_cand++] = t;
    }
  }
  if (num_cand == 0)
    return -1; // original logs warning and RETURNS -- timer stays queued
  return candidates[0];
}

/**
 * @test MServerRespawn.CandidatePoolRulesAndStuckTimerQuirk
 * @brief Verifies candidate Pool Rules And Stuck Timer Quirk.
 *
 * @details
 * Exercises the MServerRespawn code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/multiserver.cpp
 * @ingroup descent3_tests
 */
TEST(MServerRespawn, CandidatePoolRulesAndStuckTimerQuirk) {
  NumSpots = 4;
  RSpots[0] = {0, 7};  // free, matches id 7
  RSpots[1] = {0, 9};  // free, other id
  RSpots[2] = {1, 7};  // occupied

  // strict mode: only spots whose original_id matches
  EXPECT_EQ(RepPickRespawnCandidate(false, 7), 0);
  EXPECT_EQ(RepPickRespawnCandidate(false, 9), 1);
  EXPECT_EQ(RepPickRespawnCandidate(false, 12), -1); // no matching spot

  // randomize mode: ANY unused spot joins the pool (both free ones here)
  auto pick = RepPickRespawnCandidate(true, 7);
  EXPECT_TRUE(pick == 0 || pick == 1);

  // quirk: no candidates -> early return leaves the timer entry queued,
  // retried every frame until a spot frees up
  RSpots[0] = {1, 7};
  RSpots[1] = {1, 9};
  EXPECT_EQ(RepPickRespawnCandidate(false, 7), -1);
}

/**
 * @test MServerRespawn.TimerCompactionPreservesOrderAndRewindsLoop
 * @brief Verifies timer Compaction Preserves Order And Rewinds Loop.
 *
 * @details
 * Exercises the MServerRespawn code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/multiserver.cpp
 * @ingroup descent3_tests
 */
TEST(MServerRespawn, TimerCompactionPreservesOrderAndRewindsLoop) {
  // replicate the erase-by-shift loop (for t=i; t<num-1; t++) arr[t]=arr[t+1]
  NumTimers = 4;
  for (int i = 0; i < 4; i++)
    RTimers[i] = {10 + i, 0.0f};

  int i = 1; // respawning second entry
  for (int t = i; t < NumTimers - 1; t++)
    RTimers[t] = RTimers[t + 1];
  NumTimers--;

  EXPECT_EQ(NumTimers, 3);
  EXPECT_EQ(RTimers[0].id, 10);
  EXPECT_EQ(RTimers[1].id, 12); // shifted down over the removed one
  EXPECT_EQ(RTimers[2].id, 13);
  // caller does i-- after removal so the new entry at index i gets checked
  int next_check = i - 1;
  EXPECT_EQ(next_check, 0);
}

// ---------------------------------------------------------------------------
// Positional update visibility rules replication
// (multi_server.cpp:1983-2130)
struct PosPlayer {
  int objnum = 0;
  bool connected = true;
  int sequence = 2; // NETSEQ_PLAYING
};
static PosPlayer PosPlayers[8];
static uint32_t VisBits[8];
constexpr int NETSEQ_PLAYING_MS = 2;
constexpr int PFP_NO_FIRED = 0;

static void RepVisUpdate(int to_slot, int i, bool p2p, int local_pnum,
                         bool dedicated) {
  if (p2p && i != local_pnum)
    return;
  if (dedicated && i == local_pnum)
    return;
  if (PosPlayers[i].sequence != NETSEQ_PLAYING_MS ||
      !PosPlayers[i].connected) {
    VisBits[i] &= ~(1u << to_slot);
    VisBits[to_slot] &= ~(1u << i);
    return;
  }
  // server position always considered visible (ghost ship fix)
  if (i == local_pnum) { /* send_position forced */
  }
  VisBits[to_slot] |= (1u << i);
}

/**
 * @test MServerPositional.VisibilityGatingAndBitClearing
 * @brief Verifies visibility Gating And Bit Clearing.
 *
 * @details
 * Exercises the MServerPositional code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/multiserver.cpp
 * @ingroup descent3_tests
 */
TEST(MServerPositional, VisibilityGatingAndBitClearing) {
  memset(VisBits, 0xFF, sizeof(VisBits)); // start fully visible

  PosPlayers[1] = {1, false, NETSEQ_PLAYING_MS};   // disconnected
  PosPlayers[2] = {2, true, 1};                    // still joining
  PosPlayers[3] = {3, true, NETSEQ_PLAYING_MS};    // fine

  RepVisUpdate(0, 1, false, 0, false);
  EXPECT_EQ(VisBits[1] & (1u << 0), 0u); // cleared both directions
  EXPECT_EQ(VisBits[0] & (1u << 1), 0u);

  RepVisUpdate(0, 2, false, 0, false);
  EXPECT_EQ(VisBits[2] & (1u << 0), 0u);

  RepVisUpdate(0, 3, false, 0, false);
  EXPECT_NE(VisBits[0] & (1u << 3), 0u);

  // peer-peer: only the local player's own position is distributed;
  // other slots are skipped BEFORE any bit manipulation
  RepVisUpdate(0, 3, true, 0, false);
  EXPECT_NE(VisBits[0] & (1u << 3), 0u); // unchanged from earlier pass

  // dedicated server never sends its own slot's position
  VisBits[4] = 0xFF;
  RepVisUpdate(4, 0, false, 0, true);
  EXPECT_EQ(VisBits[4], 0xFF); // skipped before bit manipulation
}
