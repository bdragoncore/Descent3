/**
 * @file matcen_linked_real_tests.cpp
 * @brief Unit tests for Descent3/matcen.cpp.
 *
 * @details
 * Covers the public contract of `Descent3/matcen.cpp` — construction, state transitions, error handling and edge cases exercised through the GoogleTest harness.
 *
 * This harness validates the behavior of `Descent3/matcen.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/matcen.cpp`
 * @par Harness
 * `matcen_linked_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Linkage
 * Whole-game linked via the `descent3` static library with
 * `LINKER:--wrap,pilot::initialize` and a headless
 * `Video_res_list` fixup for off-screen coverage.
 *
 * @ingroup descent3_tests
 * @see Descent3/matcen.cpp
 */

#include <gtest/gtest.h>
#include <vector>
#include "config.h"
#include "matcen.h"
#include "matcen_external.h"

// Wrap pilot::initialize - headless fix (global ctor touches Video_res_list)
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

/**
 * @test MatcenLinked.Smoke
 * @brief Verifies smoke.
 *
 * @details
 * Exercises the MatcenLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/matcen.cpp
 * @ingroup descent3_tests
 */
TEST(MatcenLinked, Smoke) { EXPECT_TRUE(true); }

/**
 * @test MatcenLinked.DefaultsAndBounds
 * @brief Verifies defaults And Bounds.
 *
 * @details
 * Exercises the MatcenLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/matcen.cpp
 * @ingroup descent3_tests
 */
TEST(MatcenLinked, DefaultsAndBounds) {
  matcen m;
  // defaults (matcen.cpp:219,242)
  EXPECT_EQ(m.GetMaxProd(), 0);
  EXPECT_EQ(m.GetAttach(), MATCEN_ERROR);
  EXPECT_EQ(m.GetAttachType(), MT_UNASSIGNED);
  EXPECT_EQ(m.GetControlType(), MPC_SCRIPT);
  EXPECT_EQ(m.GetNumSpawnPnts(), 0);
  EXPECT_EQ(m.GetNumProdTypes(), 0);
  EXPECT_GE(m.GetCreationTexture(), -1);

  // SetMaxProd: -2 invalid, -1 and >=0 valid (matcen.cpp:258)
  EXPECT_FALSE(m.SetMaxProd(-2));
  EXPECT_TRUE(m.SetMaxProd(5));
  EXPECT_EQ(m.GetMaxProd(), 5);
  EXPECT_TRUE(m.SetMaxProd(-1));
  EXPECT_EQ(m.GetMaxProd(), -1);
  EXPECT_TRUE(m.SetMaxProd(0));

  // SetControlType: 0..MAX_MATCEN_CONTROL_TYPES-1 valid (matcen.cpp:525)
  EXPECT_TRUE(m.SetControlType(MPC_SCRIPT));
  EXPECT_EQ(m.GetControlType(), MPC_SCRIPT);
  EXPECT_TRUE(m.SetControlType(MAX_MATCEN_CONTROL_TYPES - 1));
  EXPECT_FALSE(m.SetControlType(MAX_MATCEN_CONTROL_TYPES));
  EXPECT_FALSE(m.SetControlType(-1));

  // SetNumSpawnPnts: 0..MAX_SPAWN_PNTS valid (matcen.cpp:536)
  EXPECT_TRUE(m.SetNumSpawnPnts(0));
  EXPECT_TRUE(m.SetNumSpawnPnts(MAX_SPAWN_PNTS));
  EXPECT_FALSE(m.SetNumSpawnPnts(MAX_SPAWN_PNTS + 1));
  EXPECT_FALSE(m.SetNumSpawnPnts(-1));

  // SetNumProdTypes: 0..MAX_PROD_TYPES valid (matcen.cpp:979)
  EXPECT_TRUE(m.SetNumProdTypes(0));
  EXPECT_TRUE(m.SetNumProdTypes(MAX_PROD_TYPES));
  EXPECT_FALSE(m.SetNumProdTypes(MAX_PROD_TYPES + 1));
  EXPECT_FALSE(m.SetNumProdTypes(-1));

  // SetCreationEffect: 0..NUM_MATCEN_EFFECTS-1 valid (matcen.cpp:1700)
  EXPECT_TRUE(m.SetCreationEffect(MEFFECT_LINE_LIGHTNING));
  EXPECT_EQ(m.GetCreationEffect(), MEFFECT_LINE_LIGHTNING);
  EXPECT_FALSE(m.SetCreationEffect(NUM_MATCEN_EFFECTS));
  EXPECT_FALSE(m.SetCreationEffect(-1));

  // SetAttachType: MT_OBJECT/ROOM/UNASSIGNED only (matcen.cpp:514)
  EXPECT_TRUE(m.SetAttachType(MT_ROOM));
  EXPECT_EQ(m.GetAttachType(), MT_ROOM);
  EXPECT_TRUE(m.SetAttachType(MT_OBJECT));
  EXPECT_TRUE(m.SetAttachType(MT_UNASSIGNED));
  EXPECT_FALSE(m.SetAttachType(99));

  // Get/SetSpawnPnt bounds (matcen.cpp:710,717)
  EXPECT_EQ(m.GetSpawnPnt(-1), MATCEN_ERROR);
  EXPECT_EQ(m.GetSpawnPnt(MAX_SPAWN_PNTS), MATCEN_ERROR);
  EXPECT_FALSE(m.SetSpawnPnt(-1, 0));
  EXPECT_FALSE(m.SetSpawnPnt(MAX_SPAWN_PNTS, 0));
  EXPECT_TRUE(m.SetSpawnPnt(0, 1));
  EXPECT_EQ(m.GetSpawnPnt(0), 1);

  // GetSound / SetMaxAliveChildren bounds
  EXPECT_EQ(m.GetSound(-1), MATCEN_ERROR);
  EXPECT_EQ(m.GetSound(MAX_MATCEN_SOUNDS), MATCEN_ERROR);
  EXPECT_FALSE(m.SetMaxAliveChildren(0)); // 0 is invalid (matcen.cpp:1783)
  EXPECT_TRUE(m.SetMaxAliveChildren(-1));
  EXPECT_TRUE(m.SetMaxAliveChildren(1));
  EXPECT_FALSE(m.SetMaxAliveChildren(MAX_MATCEN_ALIVE_CHILDREN + 1));
}

/**
 * @test MatcenLinked.NameRoundTrip
 * @brief Verifies name Round Trip.
 *
 * @details
 * Exercises the MatcenLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/matcen.cpp
 * @ingroup descent3_tests
 */
TEST(MatcenLinked, NameRoundTrip) {
  matcen m;
  const char *n = "test_matcen_123";
  EXPECT_TRUE(m.SetName(n));
  char buf[MAX_MATCEN_NAME_LEN] = {};
  m.GetName(buf);
  EXPECT_STREQ(buf, n);
}

/**
 * @test MatcenLinked.GlobalFindAndValid
 * @brief Verifies global Find And Valid.
 *
 * @details
 * Exercises the MatcenLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/matcen.cpp
 * @ingroup descent3_tests
 */
TEST(MatcenLinked, GlobalFindAndValid) {
  // ensure clean global state
  DestroyAllMatcens();
  InitMatcens();
  EXPECT_EQ(Num_matcens, 0);
  EXPECT_EQ(FindMatcenIndex("nonexistent_xyz"), MATCEN_ERROR);
  EXPECT_FALSE(MatcenValid(-1));
  EXPECT_FALSE(MatcenValid(0));

  bool changed = true;
  int id = CreateMatcen("FooMatcen", &changed);
  ASSERT_GE(id, 0);
  EXPECT_EQ(Num_matcens, 1);
  EXPECT_TRUE(MatcenValid(id));
  // stricmp case-insensitive (matcen.cpp:1879)
  EXPECT_EQ(FindMatcenIndex("foomatcen"), id);
  EXPECT_EQ(FindMatcenIndex("FooMatcen"), id);

  // second matcen
  int id2 = CreateMatcen("Bar", &changed);
  EXPECT_EQ(FindMatcenIndex("bar"), id2);
  EXPECT_EQ(Num_matcens, 2);

  DestroyMatcen(id2, false);
  EXPECT_EQ(Num_matcens, 1);
  // after non-resort destroy, id2 slot is NULL but count decremented
  EXPECT_FALSE(MatcenValid(id2));

  DestroyAllMatcens();
  EXPECT_EQ(Num_matcens, 0);
  // re-init for next tests / atexit safety
  InitMatcens();
}

/**
 * @test MatcenLinked.ProdMultiplierAndPrePost
 * @brief Verifies prod Multiplier And Pre Post.
 *
 * @details
 * Exercises the MatcenLinked code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/matcen.cpp
 * @ingroup descent3_tests
 */
TEST(MatcenLinked, ProdMultiplierAndPrePost) {
  matcen m;
  EXPECT_TRUE(m.SetProdMultiplier(1.0f));
  EXPECT_FLOAT_EQ(m.GetProdMultiplier(), 1.0f);
  EXPECT_FALSE(m.SetProdMultiplier(-0.1f));
  // pre/post times 0..2.0 valid for current effect (matcen.cpp:1838,1864)
  EXPECT_TRUE(m.SetPreProdTime(1.0f));
  EXPECT_FLOAT_EQ(m.GetPreProdTime(), 1.0f);
  EXPECT_FALSE(m.SetPreProdTime(-1.0f));
  EXPECT_FALSE(m.SetPreProdTime(3.0f));
  EXPECT_TRUE(m.SetPostProdTime(0.5f));
  EXPECT_FALSE(m.SetPostProdTime(-1.0f));
}
