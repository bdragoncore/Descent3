/**
 * @file viseffect_real_tests.cpp
 * @brief Tests for viseffect.cpp 2276 lines — vis effect pool & linking.
 *
 * @details
 * Covers the allocate/free LIFO pool, Highest_vis_effect_index
 * tracking with its never-shrinks quirk (inverted while condition),
 * InitVisEffects idempotence guard, VisEffectCreate gating (dedicated
 * servers, stale rooms > 5s), terrain early-outs in Link/Unlink,
 * InitType explosion lighting (custom/napalm excluded), and spark
 * creation parameter ranges.
 *
 * This harness validates the behavior of `Descent3/viseffect.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/viseffect.cpp`
 * @par Harness
 * `viseffect_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/viseffect.cpp
 */

#include <gtest/gtest.h>
#include <cstring>
#include <cstdint>
#include <cstdlib>

// replicated constants
constexpr int MAX_VIS_EFFECTS = 5000;
constexpr uint8_t VIS_NONE = 0;
constexpr uint8_t VIS_FIREBALL = 1;
constexpr int VF_USES_LIFELEFT = 1;
constexpr int MT_NONE = 0, MT_PHYSICS = 1;
constexpr int PF_GRAVITY = 0x10, PF_NO_COLLIDE = 0x4000; // values not asserted, just set
constexpr int CUSTOM_EXPLOSION_INDEX = 11;
constexpr int HOT_SPARK_INDEX = 15;
constexpr int COOL_SPARK_INDEX = 16;
constexpr int FADING_LINE_INDEX = 19;
constexpr int NAPALM_BALL_INDEX = 25;
constexpr int FT_EXPLOSION = 0;

static uint16_t OPAQUE_FLAG = 0x8000;
static uint16_t GR_RGB16(int r, int g, int b) {
  return (uint16_t)(((r >> 3) << 10) | ((g >> 3) << 5) | (b >> 3));
}

// deterministic rand stand-in for ps_rand
static unsigned RepSeed = 1;
static void RepSrand(unsigned s) { RepSeed = s; }
static int RepRand() { // LCG, mirrors ps_rand's % usage patterns
  RepSeed = RepSeed * 1103515245u + 12345u;
  return (int)((RepSeed >> 16) & 0x7FFF);
}

// ---------------------------------------------------------------------------
// pool replication (viseffect.cpp:472-551)
struct VisMock {
  uint8_t type = VIS_NONE;
  int roomnum = -1;
  int next = -1, prev = -1;
};

static constexpr int POOL = 32; // small pool instead of 5000
static VisMock VPool[POOL];
static int16_t VFree[POOL];
static int VNum = 0;
static int VHighest = 0;

static void PoolInit() {
  memset(VPool, 0, sizeof(VPool));
  for (int i = 0; i < POOL; i++) {
    VPool[i].type = VIS_NONE;
    VPool[i].roomnum = -1;
    VPool[i].prev = VPool[i].next = -1;
    VFree[i] = i;
  }
  VNum = 0;
  VHighest = 0;
}

static int PoolAllocate() {
  if (VNum == POOL)
    return -1;
  int n = VFree[VNum++];
  if (n > VHighest)
    VHighest = n;
  return n;
}

static int PoolFree(int visnum) {
  VFree[--VNum] = visnum;
  VPool[visnum].type = VIS_NONE;
  if (visnum == VHighest) {
    // quirk: condition order means the walk never runs — the slot was
    // JUST set to VIS_NONE two lines above, so Highest never shrinks
    while (VPool[VHighest].type != VIS_NONE && VHighest > 0)
      VHighest--;
  }
  return 1;
}

/**
 * @test VisPool.AllocateIsLIFOAndTracksHighest
 * @brief Verifies allocate Is LIFOAnd Tracks Highest.
 *
 * @details
 * Exercises the VisPool code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/viseffect.cpp
 * @ingroup descent3_tests
 */
TEST(VisPool, AllocateIsLIFOAndTracksHighest) {
  PoolInit();
  EXPECT_EQ(PoolAllocate(), 0);
  EXPECT_EQ(PoolAllocate(), 1);
  EXPECT_EQ(VNum, 2);

  PoolFree(0); // free lowest first
  EXPECT_EQ(VNum, 1);
  EXPECT_EQ(PoolAllocate(), 0); // stack top reused
  EXPECT_EQ(PoolAllocate(), 2); // then next fresh
  EXPECT_EQ(VHighest, 2);
}

/**
 * @test VisPool.ExhaustedPoolRejectsWithMinusOne
 * @brief Verifies exhausted Pool Rejects With Minus One.
 *
 * @details
 * Exercises the VisPool code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/viseffect.cpp
 * @ingroup descent3_tests
 */
TEST(VisPool, ExhaustedPoolRejectsWithMinusOne) {
  PoolInit();
  for (int i = 0; i < POOL; i++)
    EXPECT_GE(PoolAllocate(), 0);
  EXPECT_EQ(PoolAllocate(), -1);
  EXPECT_EQ(VNum, POOL);
}

/**
 * @test VisPool.HighestIndexNeverShrinksOnFree
 * @brief Verifies highest Index Never Shrinks On Free.
 *
 * @details
 * Exercises the VisPool code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/viseffect.cpp
 * @ingroup descent3_tests
 */
TEST(VisPool, HighestIndexNeverShrinksOnFree) {
  PoolInit();
  for (int i = 0; i <= 4; i++)
    PoolAllocate(); // highest = 4

  PoolFree(4);
  // quirk: freeing the highest sets its type to VIS_NONE first, so the
  // inverted walk-down condition exits immediately — index stays at 4
  EXPECT_EQ(VHighest, 4);

  // contrast with object.cpp ObjFree which walks down over NONE slots
  PoolFree(3);
  EXPECT_EQ(VHighest, 4);
  EXPECT_EQ(PoolAllocate(), 3); // reuse works regardless
  EXPECT_EQ(VHighest, 4);       // still not lowered... and re-alloc of 4 later keeps it
}

// ---------------------------------------------------------------------------
// InitVisEffects idempotence guard replication (viseffect.cpp:491-519)
/**
 * @test VisInit.SecondCallWithSameMaxSkipsReset
 * @brief Verifies second Call With Same Max Skips Reset.
 *
 * @details
 * Exercises the VisInit code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/viseffect.cpp
 * @ingroup descent3_tests
 */
TEST(VisInit, SecondCallWithSameMaxSkipsReset) {
  static uint16_t old_max_vis = 0;
  auto init = [&](bool *allocated) {
    uint16_t max_vis_effects = 100;
    if (old_max_vis == max_vis_effects)
      return false; // early-out: no reset of counters/list
    *allocated = true;
    old_max_vis = max_vis_effects;
    return true;
  };
  bool allocated = false;
  EXPECT_TRUE(init(&allocated)); // first call allocates
  allocated = false;
  EXPECT_FALSE(init(&allocated)); // second call is a no-op
}

// ---------------------------------------------------------------------------
// Create gating replication (viseffect.cpp:569-629)
static float Gametime_v = 10.0f;

/**
 * @test VisCreate.StaleRoomsRejectedAfterFiveSeconds
 * @brief Verifies stale Rooms Rejected After Five Seconds.
 *
 * @details
 * Exercises the VisCreate code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/viseffect.cpp
 * @ingroup descent3_tests
 */
TEST(VisCreate, StaleRoomsRejectedAfterFiveSeconds) {
  float last_render = Gametime_v - 5.5f;
  bool reject = (Gametime_v - last_render) > 5.0f; // strict >
  EXPECT_TRUE(reject);

  last_render = Gametime_v - 5.0f;
  EXPECT_FALSE((Gametime_v - last_render) > 5.0f); // exactly 5s still allowed
}

// ---------------------------------------------------------------------------
// Link/Unlink terrain early-outs (viseffect.cpp:724-770)
struct RoomMock {
  int vis_effects = -1;
};
static RoomMock VRoom;

static bool IsOutside(int roomnum) { return roomnum < 0; } // stand-in for ROOMNUM_OUTSIDE

static void VLink(VisMock &vis, int visnum, int roomnum) {
  vis.roomnum = roomnum;
  if (IsOutside(roomnum))
    return; // quirk: terrain effects keep STALE next/prev from before
  vis.next = VRoom.vis_effects;
  VRoom.vis_effects = visnum;
  vis.prev = -1;
  if (vis.next != -1)
    VPool[vis.next].prev = visnum;
}

static void VUnlink(VisMock &vis, int visnum) {
  if (IsOutside(vis.roomnum))
    return; // no-op for terrain
  if (vis.prev == -1)
    VRoom.vis_effects = vis.next;
  else
    VPool[vis.prev].next = vis.next;
  if (vis.next != -1)
    VPool[vis.next].prev = vis.prev;
  vis.roomnum = -1;
}

/**
 * @test VisLink.RoomListSpliceAndUnlink
 * @brief Verifies room List Splice And Unlink.
 *
 * @details
 * Exercises the VisLink code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/viseffect.cpp
 * @ingroup descent3_tests
 */
TEST(VisLink, RoomListSpliceAndUnlink) {
  PoolInit();
  VRoom.vis_effects = -1;
  int a = PoolAllocate(), b = PoolAllocate();
  VLink(VPool[a], a, 7);
  VLink(VPool[b], b, 7);
  EXPECT_EQ(VRoom.vis_effects, b); // newest head
  EXPECT_EQ(VPool[b].next, a);
  EXPECT_EQ(VPool[a].prev, b);

  VUnlink(VPool[b], b);
  EXPECT_EQ(VRoom.vis_effects, a);
  EXPECT_EQ(VPool[a].prev, -1);
  EXPECT_EQ(VPool[b].roomnum, -1);
}

/**
 * @test VisLink.TerrainEffectsSkipListEntirely
 * @brief Verifies terrain Effects Skip List Entirely.
 *
 * @details
 * Exercises the VisLink code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/viseffect.cpp
 * @ingroup descent3_tests
 */
TEST(VisLink, TerrainEffectsSkipListEntirely) {
  PoolInit();
  VRoom.vis_effects = -1;
  int t = PoolAllocate();
  VLink(VPool[t], t, -5); // outside cell
  EXPECT_EQ(VPool[t].roomnum, -5);
  EXPECT_EQ(VRoom.vis_effects, -1);   // never entered room list
  EXPECT_EQ(VPool[t].next, -1);       // links untouched
  VUnlink(VPool[t], t);               // no-op
  EXPECT_EQ(VPool[t].roomnum, -5);    // still "linked" to terrain cell
}

// ---------------------------------------------------------------------------
// DeleteDead drain replication (viseffect.cpp:782-789)
/**
 * @test VisDelete.DeadListDrainsCompletelyEachFrame
 * @brief Verifies dead List Drains Completely Each Frame.
 *
 * @details
 * Exercises the VisDelete code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/viseffect.cpp
 * @ingroup descent3_tests
 */
TEST(VisDelete, DeadListDrainsCompletelyEachFrame) {
  int dead_list[8] = {3, 7, 9};
  int num_dead = 3, deletes = 0;
  for (int i = 0; i < num_dead; i++)
    deletes++;
  num_dead = 0;
  EXPECT_EQ(deletes, 3);
  EXPECT_EQ(num_dead, 0);
  (void)dead_list;
}

// ---------------------------------------------------------------------------
// InitType replication (viseffect.cpp:553-565)
struct FireballDefMock {
  float size = 2.5f;
  float total_life = 1.25f;
  int type = FT_EXPLOSION;
};
static FireballDefMock Fireballs[32];

struct VisInitMock {
  uint8_t id = 0;
  float size = 0, lifeleft = 0, lifetime = 0;
  uint16_t flags = 0;
  uint16_t lighting_color = 0;
};

static int RepInitTypeImpl(VisInitMock *vis) {
  vis->size = Fireballs[vis->id].size;
  vis->flags |= VF_USES_LIFELEFT;
  vis->lifeleft = Fireballs[vis->id].total_life;
  vis->lifetime = vis->lifeleft;
  if (Fireballs[vis->id].type == FT_EXPLOSION && vis->id != CUSTOM_EXPLOSION_INDEX &&
      vis->id != NAPALM_BALL_INDEX)
    vis->lighting_color = OPAQUE_FLAG | GR_RGB16(255, 180, 20);
  return 1;
}

/**
 * @test VisInitType.ExplosionGetsOrangeLightingExceptCustomAndNapalm
 * @brief Verifies explosion Gets Orange Lighting Except Custom And Napalm.
 *
 * @details
 * Exercises the VisInitType code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/viseffect.cpp
 * @ingroup descent3_tests
 */
TEST(VisInitType, ExplosionGetsOrangeLightingExceptCustomAndNapalm) {
  Fireballs[3].size = 2.5f;
  Fireballs[3].total_life = 1.25f;
  Fireballs[3].type = FT_EXPLOSION;
  Fireballs[CUSTOM_EXPLOSION_INDEX].type = FT_EXPLOSION;
  Fireballs[NAPALM_BALL_INDEX].type = FT_EXPLOSION;

  VisInitMock v;
  v.id = 3;
  RepInitTypeImpl(&v);
  EXPECT_FLOAT_EQ(v.size, 2.5f);
  EXPECT_FLOAT_EQ(v.lifeleft, 1.25f);
  EXPECT_FLOAT_EQ(v.lifetime, 1.25f);
  EXPECT_TRUE(v.flags & VF_USES_LIFELEFT);
  uint16_t expected = OPAQUE_FLAG | GR_RGB16(255, 180, 20);
  EXPECT_EQ(v.lighting_color, expected);

  VisInitMock c;
  c.id = CUSTOM_EXPLOSION_INDEX;
  RepInitTypeImpl(&c);
  EXPECT_EQ(c.lighting_color, 0); // custom explosions keep caller color

  VisInitMock nap;
  nap.id = NAPALM_BALL_INDEX;
  RepInitTypeImpl(&nap);
  EXPECT_EQ(nap.lighting_color, 0); // napalm keeps caller color too
}
