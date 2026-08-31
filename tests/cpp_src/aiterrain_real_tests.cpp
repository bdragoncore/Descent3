/**
 * @file aiterrain_real_tests.cpp
 * @brief Tests for aiterrain.cpp — terrain AI helpers (305 lines).
 *
 * @details
 * Covers ait_Init and ait_GetGroundInfo with stubbed flat terrain.
 *
 * This harness validates the behavior of `Descent3/aiterrain.cpp`. It compiles the real
 * implementation (or a minimal replica when isolation is required) against
 * stubbed engine subsystems and checks the observable contract documented
 * in the source headers and in-game behavior.
 *
 * @par Source
 * `Descent3/aiterrain.cpp`
 * @par Harness
 * `aiterrain_real_tests.cpp`
 * @par Framework
 * GoogleTest (gtest) — `TEST` / `TEST_F` macros
 * @par Isolation
 * Stubs and fakes for engine globals not under test; see the file body
 * for the full stub inventory. `RELEASE` is defined to suppress
 * `Int3()` aborts that would otherwise terminate the test process.
 *
 * @ingroup descent3_tests
 * @see Descent3/aiterrain.cpp
 */

#include <gtest/gtest.h>
#include <cstring>
#include "vecmat.h"
#include "terrain.h"
#include "aiterrain.h"

// Provide flat terrain stubs
terrain_segment Terrain_seg[(TERRAIN_WIDTH + 1) * (TERRAIN_DEPTH + 1)];

int GetTerrainCellFromPos(vector *pos) {
  int x = int(pos->x() / TERRAIN_SIZE);
  int z = int(pos->z() / TERRAIN_SIZE);
  if (x < 0 || x >= TERRAIN_WIDTH || z < 0 || z >= TERRAIN_DEPTH) return -1;
  return z * TERRAIN_WIDTH + x;
}

/**
 * @test AITerrain.InitClears
 * @brief Verifies init Clears.
 *
 * @details
 * Exercises the AITerrain code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/aiterrain.cpp
 * @ingroup descent3_tests
 */
TEST(AITerrain, InitClears) {
  ait_Init();
  // after init, a subsequent GetGroundInfo should succeed on flat terrain
  ground_information gi;
  vector p0 = {128 * TERRAIN_SIZE, 0, 128 * TERRAIN_SIZE};
  vector p1 = {130 * TERRAIN_SIZE, 0, 128 * TERRAIN_SIZE};
  // flat terrain y=0
  for (int i = 0; i < (TERRAIN_WIDTH + 1) * (TERRAIN_DEPTH + 1); i++) Terrain_seg[i].y = 0;
  bool ok = ait_GetGroundInfo(&gi, &p0, &p1, 5.0f, 0);
  EXPECT_TRUE(ok);
  EXPECT_FLOAT_EQ(gi.highest_y, 0);
  EXPECT_FLOAT_EQ(gi.lowest_y, 0);
}

/**
 * @test AITerrain.FlatGroundInfoTracksHeight
 * @brief Verifies flat Ground Info Tracks Height.
 *
 * @details
 * Exercises the AITerrain code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/aiterrain.cpp
 * @ingroup descent3_tests
 */
TEST(AITerrain, FlatGroundInfoTracksHeight) {
  ait_Init();
  for (int i = 0; i < (TERRAIN_WIDTH + 1) * (TERRAIN_DEPTH + 1); i++) Terrain_seg[i].y = 0;
  // raise one cell
  int cx = 128, cz = 128;
  Terrain_seg[cz * TERRAIN_WIDTH + cx].y = 10.0f;
  ground_information gi;
  vector p0 = {float(cx * TERRAIN_SIZE + 8), 0, float(cz * TERRAIN_SIZE + 8)};
  vector p1 = {float((cx + 1) * TERRAIN_SIZE + 8), 0, float(cz * TERRAIN_SIZE + 8)};
  bool ok = ait_GetGroundInfo(&gi, &p0, &p1, 5.0f, 0);
  EXPECT_TRUE(ok);
  EXPECT_GE(gi.highest_y, 10.0f);
  EXPECT_LE(gi.lowest_y, 0.0f);
}

/**
 * @test AITerrain.OffTerrainP1Clamped
 * @brief Verifies off Terrain P1Clamped.
 *
 * @details
 * Exercises the AITerrain code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/aiterrain.cpp
 * @ingroup descent3_tests
 */
TEST(AITerrain, OffTerrainP1Clamped) {
  ait_Init();
  for (int i = 0; i < (TERRAIN_WIDTH + 1) * (TERRAIN_DEPTH + 1); i++) Terrain_seg[i].y = 0;
  ground_information gi;
  vector p0 = {10 * TERRAIN_SIZE, 0, 10 * TERRAIN_SIZE};
  vector p1 = {-100, 0, -100}; // off terrain, clamped to edge (small path to stay <200 segs)
  bool ok = ait_GetGroundInfo(&gi, &p0, &p1, 5.0f, 0);
  // Should still return true after clamping p1 inside terrain (aiterrain handles off-terrain by delta)
  EXPECT_TRUE(ok);
}

/**
 * @test AITerrain.StartOffTerrainReturnsFalse
 * @brief Verifies start Off Terrain Returns False.
 *
 * @details
 * Exercises the AITerrain code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/aiterrain.cpp
 * @ingroup descent3_tests
 */
TEST(AITerrain, StartOffTerrainReturnsFalse) {
  ait_Init();
  ground_information gi;
  vector p0 = {-1000, 0, -1000};
  vector p1 = {128 * TERRAIN_SIZE, 0, 128 * TERRAIN_SIZE};
  bool ok = ait_GetGroundInfo(&gi, &p0, &p1, 5.0f, 0);
  EXPECT_FALSE(ok);
}

/**
 * @test AITerrain.DoubleInitIsIdempotent
 * @brief Verifies double Init Is Idempotent.
 *
 * @details
 * Exercises the AITerrain code path and asserts observable
 * post-conditions. Stubbed subsystems provide deterministic
 * inputs; no external I/O is performed.
 *
 * @see Descent3/aiterrain.cpp
 * @ingroup descent3_tests
 */
TEST(AITerrain, DoubleInitIsIdempotent) {
  ait_Init();
  ait_Init();
  ground_information gi;
  vector p0 = {10 * TERRAIN_SIZE, 0, 10 * TERRAIN_SIZE};
  vector p1 = {11 * TERRAIN_SIZE, 0, 10 * TERRAIN_SIZE};
  for (int i = 0; i < (TERRAIN_WIDTH + 1) * (TERRAIN_DEPTH + 1); i++) Terrain_seg[i].y = 1.0f;
  EXPECT_TRUE(ait_GetGroundInfo(&gi, &p0, &p1, 1.0f, 0));
}
